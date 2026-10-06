const MQTT_BROKER = "ws://broker.emqx.io:8083/mqtt";
const MQTT_TOPIC = "sembrani/powermeter";

const mqttStatus = document.getElementById("mqttStatus");

const clientId =
    "sembrani-dashboard-" +
    Math.random().toString(16).substring(2, 10);

const client = mqtt.connect(MQTT_BROKER, {
    clientId: clientId,
    clean: true,
    connectTimeout: 5000,
    reconnectPeriod: 2000
});

function setValue(id, value, decimals = 2)
{
    const element = document.getElementById(id);

    if (!element) {
        return;
    }

    if (value === undefined || value === null || Number.isNaN(Number(value))) {
        element.textContent = "--";
        return;
    }

    element.textContent = Number(value).toFixed(decimals);
}

client.on("connect", function ()
{
    console.log("MQTT connected");

    mqttStatus.textContent = "MQTT Connected";
    mqttStatus.className = "badge text-bg-success";

    client.subscribe(MQTT_TOPIC, function (error)
    {
        if (error) {
            console.error("Subscribe failed:", error);
            mqttStatus.textContent = "Subscribe Error";
            mqttStatus.className = "badge text-bg-danger";
        }
        else {
            console.log("Subscribed to:", MQTT_TOPIC);
        }
    });
});

client.on("reconnect", function ()
{
    console.log("MQTT reconnecting...");

    mqttStatus.textContent = "MQTT Reconnecting";
    mqttStatus.className = "badge text-bg-warning";
});

client.on("offline", function ()
{
    console.log("MQTT offline");

    mqttStatus.textContent = "MQTT Offline";
    mqttStatus.className = "badge text-bg-secondary";
});

client.on("error", function (error)
{
    console.error("MQTT error:", error);

    mqttStatus.textContent = "MQTT Error";
    mqttStatus.className = "badge text-bg-danger";
});

client.on("message", function (topic, message)
{
    if (topic !== MQTT_TOPIC) {
        return;
    }

    try {
        const data = JSON.parse(message.toString());

        console.log("Power data:", data);

        // Total / system
        setValue("activePower", data.activePower);
        setValue("reactivePower", data.reactivePower);
        setValue("apparentPower", data.apparentPower);
        setValue("powerFactor", data.powerFactor, 3);

        setValue("voltageLL", data.voltageLL);
        setValue("voltageLN", data.voltageLN);
        setValue("current", data.current);
        setValue("frequency", data.frequency);

        // Voltage
        setValue("voltageL1N", data.voltageL1N);
        setValue("voltageL2N", data.voltageL2N);
        setValue("voltageL3N", data.voltageL3N);

        setValue("voltageL1L2", data.voltageL1L2);
        setValue("voltageL2L3", data.voltageL2L3);
        setValue("voltageL1L3", data.voltageL1L3);

        // Phase 1
        setValue("apparentPowerL1", data.apparentPowerL1);
        setValue("activePowerL1", data.activePowerL1);
        setValue("reactivePowerL1", data.reactivePowerL1);
        setValue("powerFactorL1", data.powerFactorL1, 3);
        setValue("currentL1", data.currentL1);

        // Phase 2
        setValue("apparentPowerL2", data.apparentPowerL2);
        setValue("activePowerL2", data.activePowerL2);
        setValue("reactivePowerL2", data.reactivePowerL2);
        setValue("powerFactorL2", data.powerFactorL2, 3);
        setValue("currentL2", data.currentL2);

        // Phase 3
        setValue("apparentPowerL3", data.apparentPowerL3);
        setValue("activePowerL3", data.activePowerL3);
        setValue("reactivePowerL3", data.reactivePowerL3);
        setValue("powerFactorL3", data.powerFactorL3, 3);
        setValue("currentL3", data.currentL3);

    }
    catch (error) {
        console.error("Invalid MQTT JSON:", error);
    }
});
