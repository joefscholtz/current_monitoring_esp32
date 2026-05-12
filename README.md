# current_monitoring_esp32

## Project structure

```
.
├── .github/workflows/    # CI/CD (Lints, Build, Wokwi Tests)
├── components/           # Library ESP-IDF style components
│   └── current_monitor/  # RMS, Differential math, Calibration
│       ├── include/
│       └── src/
├── app/                  # Two alternatives to consume the library
│   ├── bare_metal/       # Simple Super-Loop implementation
│   └── rtos/             # RTOS Task-based, using MQTT and non-blocking I/O
├── test/                 # Unit tests for the monitor library
├── platformio.ini        # Multi-environment config
├── wokwi.toml            # Simulation config for ESP32 + Sensors
└── diagram.json          # Wokwi hardware layout
```

## Installation

### With Docker

Requirements:

- Docker
- Docker Compose
- just (Optional)

using just:

```bash
just dev init
```

without just

```bash
DOCKER_COMPOSE_SERVICE=current_monitoring_esp32-app

docker-compose run --rm $DOCKER_COMPOSE_SERVICE just init
```

### Without Docker:

Requirements:

- uv (Python manager)
- PlatformIO CLI
- just (Optional)

using just:

```bash
just init
```

without just

```bash
uv sync
uv run pio run -t compiledb
```
