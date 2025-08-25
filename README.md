# Frame of Enlightenment

## Runtime Configuration

The device exposes a small HTTP server that allows several runtime constants
to be adjusted without recompiling. These values are persisted in the ESP32's
non‑volatile storage so they survive power cycles.

### Web interface

Visit `http://<device-ip>/` in a browser to access a lightweight configuration
page. It fetches the current settings and lets you update them using a simple
form instead of manual `curl` commands.

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

You can find reference HSV hue values in the [FastLED HSV Colors chart](https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors).

### Resetting to defaults

To restore factory defaults for all parameters (including WiFi credentials),
issue a `POST` to `/config/reset`:

```sh
curl -X POST "http://<device-ip>/config/reset"
```

All values are reset and persisted immediately.

## Presence Detection (optional)

The project can automatically enable the LEDs only when someone is nearby and
turn them off when the room is empty. Presence detection also takes ambient
light into account:

- **Daytime:** LEDs remain on for a few minutes after motion stops.
- **Nighttime:** LEDs turn off immediately once no motion is detected.

### Wiring

1. **PIR motion sensor (HC‑SR501 or similar)**
   - VCC → 5V
   - GND → GND
   - OUT → GPIO 25
2. **Light sensor (LDR)**
   - Create a voltage divider: 3.3V → LDR → GPIO 34 → 10 kΩ resistor → GND

### Enabling

Presence detection is compiled in by defining the `ENABLE_PRESENCE` build flag.
It is enabled in `platformio.ini` and can be disabled by removing the flag or
passing `-U ENABLE_PRESENCE` when compiling.

