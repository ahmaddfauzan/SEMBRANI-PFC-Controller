/*
 * SEMBRANI PFC Dashboard
 * MQTT data topic : sembrani/powermeter
 * MQTT control topic: sembrani/control
 *
 * PENTING:
 * Format perintah target PF di bawah adalah rancangan awal.
 * Sesuaikan CONTROL_TOPIC dan payload dengan parser MQTT pada firmware ESP32.
 */

const MQTT_BROKER = "wss://broker.emqx.io:8084/mqtt";
const MQTT_TOPIC = "sembrani/powermeter";
const CONTROL_TOPIC = "sembrani/control";
const CONTROL_ACK_TOPIC = "sembrani/control/ack";
let pendingTargetPF = null;
let pendingTargetPFTimer = null;

const mqttStatus = document.getElementById("mqttStatus");
const lastUpdate = document.getElementById("lastUpdate");
const controlMessage = document.getElementById("controlMessage");
const sendTargetBtn = document.getElementById("sendTargetBtn");

const clientId = "sembrani-dashboard-" + Math.random().toString(16).slice(2, 10);

function setConnectionStatus(text, state) {
  mqttStatus.innerHTML = '<span class="status-dot"></span><span>' + text + '</span>';
  mqttStatus.className = "status-pill" + (state === "connected" ? " connected" : "");
}

function setValue(id, value, decimals = 2) {
  const element = document.getElementById(id);
  if (!element) return;

  if (value === undefined || value === null || value === "" || !Number.isFinite(Number(value))) {
    element.textContent = "--";
    return;
  }
  element.textContent = Number(value).toFixed(decimals);
}

function updatePowerFactor(value) {
  setValue("powerFactor", value, 3);
  const bar = document.getElementById("pfProgress");
  if (bar && Number.isFinite(Number(value))) {
    const pf = Math.max(0, Math.min(1, Number(value)));
    bar.style.width = (pf * 100) + "%";
  }
}

function setDisplayMode(mode) {
  const isThreePhase = mode === "3";
  document.getElementById("mode1Btn").classList.toggle("active", !isThreePhase);
  document.getElementById("mode3Btn").classList.toggle("active", isThreePhase);
  document.getElementById("mode1Btn").setAttribute("aria-pressed", String(!isThreePhase));
  document.getElementById("mode3Btn").setAttribute("aria-pressed", String(isThreePhase));

  document.querySelectorAll(".phase-3-only").forEach((element) => {
    element.hidden = !isThreePhase;
  });
  document.getElementById("phase2Section").hidden = !isThreePhase;
  document.getElementById("phase3Section").hidden = !isThreePhase;
}

document.getElementById("mode1Btn").addEventListener("click", () => setDisplayMode("1"));
document.getElementById("mode3Btn").addEventListener("click", () => setDisplayMode("3"));
setDisplayMode("1"); // Mode awal hanya mengubah tampilan browser, bukan mode kerja ESP32.

let client = null;

if (typeof mqtt === "undefined") {
  setConnectionStatus("Library MQTT gagal dimuat", "error");
} else {
  client = mqtt.connect(MQTT_BROKER, {
    clientId,
    clean: true,
    connectTimeout: 10000,
    reconnectPeriod: 3000,
    keepalive: 30
  });

  client.on("connect", () => {
    setConnectionStatus("MQTT terhubung", "connected");
    client.subscribe(MQTT_TOPIC, (error) => {
      if (error) {
        console.error("Subscribe telemetry gagal:", error);
        setConnectionStatus("Gagal subscribe", "error");
      }
    });
    client.subscribe(CONTROL_ACK_TOPIC, (error) => {
      if (error) console.error("Subscribe ACK gagal:", error);
      else console.log("Menunggu konfirmasi ESP32 pada", CONTROL_ACK_TOPIC);
    });
  });

  client.on("reconnect", () => setConnectionStatus("Menghubungkan ulang...", "pending"));
  client.on("offline", () => setConnectionStatus("MQTT offline", "offline"));
  client.on("close", () => setConnectionStatus("MQTT terputus", "offline"));
  client.on("error", (error) => {
    console.error("MQTT error:", error);
    setConnectionStatus("MQTT error", "error");
  });

  client.on("message", (topic, message) => {
    let data;
    try {
      data = JSON.parse(message.toString());
    } catch (error) {
      console.error("Pesan MQTT bukan JSON yang valid:", error);
      return;
    }

    if (topic === CONTROL_ACK_TOPIC) {
      if (data.command === "set_target_pf" && data.status === "applied") {
        if (pendingTargetPFTimer) clearTimeout(pendingTargetPFTimer);
        pendingTargetPF = null;
        controlMessage.textContent = "ESP32 mengonfirmasi: Target PF " + Number(data.targetPF).toFixed(2) + " berhasil diterapkan.";
        controlMessage.style.color = "#16845d";
      }
      return;
    }

    if (topic !== MQTT_TOPIC) return;

    try {

      setValue("activePower", data.activePower);
      setValue("reactivePower", data.reactivePower);
      setValue("apparentPower", data.apparentPower);
      updatePowerFactor(data.powerFactor);

      setValue("voltageLL", data.voltageLL);
      setValue("voltageLN", data.voltageLN);
      setValue("current", data.current);
      setValue("frequency", data.frequency);

      setValue("voltageL1N", data.voltageL1N);
      setValue("voltageL2N", data.voltageL2N);
      setValue("voltageL3N", data.voltageL3N);
      setValue("voltageL1L2", data.voltageL1L2);
      setValue("voltageL2L3", data.voltageL2L3);
      setValue("voltageL1L3", data.voltageL1L3);

      setValue("apparentPowerL1", data.apparentPowerL1);
      setValue("activePowerL1", data.activePowerL1);
      setValue("reactivePowerL1", data.reactivePowerL1);
      setValue("powerFactorL1", data.powerFactorL1, 3);
      setValue("currentL1", data.currentL1);

      setValue("apparentPowerL2", data.apparentPowerL2);
      setValue("activePowerL2", data.activePowerL2);
      setValue("reactivePowerL2", data.reactivePowerL2);
      setValue("powerFactorL2", data.powerFactorL2, 3);
      setValue("currentL2", data.currentL2);

      setValue("apparentPowerL3", data.apparentPowerL3);
      setValue("activePowerL3", data.activePowerL3);
      setValue("reactivePowerL3", data.reactivePowerL3);
      setValue("powerFactorL3", data.powerFactorL3, 3);
      setValue("currentL3", data.currentL3);

      lastUpdate.textContent = new Date().toLocaleString("id-ID");
    } catch (error) {
      console.error("Pesan MQTT bukan JSON yang valid:", error);
    }
  });
}

sendTargetBtn.addEventListener("click", () => {
  const input = document.getElementById("targetPF");
  const targetPF = Number(input.value);

  if (!Number.isFinite(targetPF) || targetPF < 0.85 || targetPF > 0.99) {
    controlMessage.textContent = "Nilai tidak valid. Masukkan target PF antara 0.85 sampai 0.99.";
    controlMessage.style.color = "#c0392b";
    input.focus();
    return;
  }

  if (!client || !client.connected) {
    controlMessage.textContent = "Belum terkirim: koneksi MQTT belum terhubung.";
    controlMessage.style.color = "#c0392b";
    return;
  }

  if (pendingTargetPF !== null) {
    controlMessage.textContent = "Masih menunggu konfirmasi target PF sebelumnya.";
    controlMessage.style.color = "#c0392b";
    return;
  }

  const command = {
    command: "set_target_pf",
    targetPF: Number(targetPF.toFixed(2))
  };

  client.publish(CONTROL_TOPIC, JSON.stringify(command), { qos: 0, retain: false }, (error) => {
    if (error) {
      controlMessage.textContent = "Gagal mengirim target PF: " + error.message;
      controlMessage.style.color = "#c0392b";
    } else {
      pendingTargetPF = command.targetPF;
      controlMessage.textContent = "Target PF " + targetPF.toFixed(2) + " terkirim. Menunggu konfirmasi ESP32...";
      controlMessage.style.color = "#9a6700";
      if (pendingTargetPFTimer) clearTimeout(pendingTargetPFTimer);
      pendingTargetPFTimer = setTimeout(() => {
        if (pendingTargetPF !== null) {
          pendingTargetPF = null;
          controlMessage.textContent = "Belum ada konfirmasi dari ESP32. Periksa koneksi dan Serial Monitor.";
          controlMessage.style.color = "#c0392b";
        }
      }, 10000);
      console.log("Perintah kontrol terkirim:", command);
    }
  });
});
