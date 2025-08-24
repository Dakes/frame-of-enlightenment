# Frame of Enlightenment

## Runtime Configuration

The device exposes a small HTTP server that allows several runtime constants
to be adjusted without recompiling. These values are persisted in the ESP32's
non‑volatile storage so they survive power cycles.

### Updating parameters

Send a `GET` request to `/config` with one or more URL parameters. Supported
fields include:

- `frame_full` – number of combined reviews and lessons that fill the LED frame
- `framerate` – display refresh rate in frames per second
- `min_s` – minimum HSV saturation for lit pixels
- `min_v` – minimum HSV value (brightness)
- `hue_lesson` – hue used for lesson items
- `hue_review` – hue used for current review items
- `hue_review_future` – hue used for upcoming review items
- `wifi_ssid` – primary WiFi network name
- `wifi_pass` – primary WiFi password (redacted in responses)
- `wifi_backup_ssid` – optional backup network name
- `wifi_backup_pass` – password for the backup network (redacted in responses)

Example: set the frame rate to 30 FPS and minimum brightness to 10:

```sh
curl "http://<device-ip>/config?framerate=30&min_v=10"
```

Example: update WiFi credentials and reconnect:

```sh
curl "http://<device-ip>/config?wifi_ssid=Home&wifi_pass=secret&wifi_backup_ssid=Phone&wifi_backup_pass=12345678"
```

The endpoint responds with a JSON object containing the current settings. If
called without parameters, it simply returns the existing configuration with any
password fields redacted.

### Resetting to defaults

To restore factory defaults for all parameters (including WiFi credentials),
issue a `POST` to `/config/reset`:

```sh
curl -X POST "http://<device-ip>/config/reset"
```

All values are reset and persisted immediately.

