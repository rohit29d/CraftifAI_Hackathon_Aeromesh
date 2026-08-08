# aeromesh

Firmware project for target `esp32c3`, organized for portability: application logic
is kept separate from vendor-specific code so it can be retargeted with less rework.
(This build implements the vendor layer with ESP-IDF; that code lives under `platforms/`.)

## Layout

- `app/` — application entry/orchestration (`app_start()` in `app.c`); includes service headers only.
- `interfaces/` — vendor-neutral contracts (e.g. `*_if.h`) that platforms implement.
- `platforms/` — vendor/platform-specific implementations; ESP-IDF (`esp_*`, `driver/*`) code lives here.
- `services/`, `modules/` — vendor-agnostic application logic.
- `configs/` — compile-time config (`app_config.h` for pins & constants).
- `utils/` — logging and helpers.
- `tests/`, `scripts/`, `docs/` — supporting material (expand these as the project grows).

The ESP-IDF entry point `app_main()` lives once in `../main/entry.c` and only
calls `app_start()`. The `main/` component is generated build glue — edit the
`firmware/` tree, not `main/`.


## Data collection and RGB status LED

The sensor task emits one CSV row every 20 ms using:

`timestamp_ms,roll,pitch,yaw`

The onboard WS2812B RGB LED on GPIO8 indicates turbulence state: green for Stable, yellow for Light Chop, and red for Severe Turbulence. The `espressif/led_strip` managed component is declared in `main/idf_component.yml` and required by the main component.
