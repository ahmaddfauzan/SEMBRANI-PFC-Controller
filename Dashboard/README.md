# SEMBRANI PFC Dashboard

Dashboard statis berbasis HTML, CSS, dan JavaScript. MQTT menggunakan WebSocket untuk menerima telemetri dan mengirim target PF.

## Hosting di Cloudflare Pages

Hubungkan repositori GitHub ini ke Cloudflare Pages, lalu gunakan pengaturan berikut:

- **Production branch:** `main`
- **Root directory:** `Dashboard`
- **Framework preset:** `None`
- **Build command:** kosongkan
- **Build output directory:** `.`

Simpan pengaturan dan deploy. Cloudflare Pages akan menerbitkan isi folder ini dan membuat deployment baru ketika ada perubahan pada branch yang terhubung. Setelah URL `*.pages.dev` tersedia, buka URL tersebut dan pastikan status MQTT di dashboard menunjukkan terhubung.

## Catatan

- Dashboard saat ini menggunakan broker `broker.emqx.io` dan topik `sembrani/*` yang sama dengan firmware.
- Ini adalah broker publik. Jangan gunakan dashboard ini untuk kendali jarak jauh peralatan berbahaya atau sensitif sebelum akses broker dan topik diamankan.
- Jangan menaruh kredensial Wi-Fi atau rahasia perangkat di folder `Dashboard`, karena seluruh file dashboard dapat dibaca oleh pengunjung situs.
