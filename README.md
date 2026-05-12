# current_monitoring_esp32

## Project structure

```
.
├── .github/workflows/          # CI/CD
├── app/                        # Two alternatives to consume the library
│   ├── bare_metal/             # Simple Super-Loop implementation
│   └── rtos/                   # RTOS Task-based, using MQTT and non-blocking I/O
├── components/                 # Library ESP-IDF style components
│   └── current_monitor/        # RMS, Differential math, Calibration
├── docs/                       # Documentation
├── kicad/                      # KiCad files
│   └── current_monitor_board/  # KiCad project
│   └── mods/                   # KiCad libraries
├── test/                       # Unit tests for the monitor library
├── platformio.ini              # Multi-environment config
├── wokwi.toml                  # Simulation config for ESP32 + Sensors
└── diagram.json                # Wokwi hardware layout
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

## Usage

## TODO

### current_monitor:

[ ] Acquisition interval missed
[ ] Convert raw into voltage
[ ] Calibration

### mqtt:

### App:

### KiCad

[ ] MQTT thru Ethernet

### Docs:

[ ] Hardware mapping
[ ] C4 model

### Other:

[ ] Test with PlatformIO IDE
[ ] Run Tests without `static inline` functions
