alias i:= init
alias bi:= build-image
alias b:= build
alias c:= clean
alias e:= enter

container_name:='current_monitoring_esp32'
docker-compose-service:='current_monitoring_esp32-app'

default:
  just --list

init:
  uv sync
  uv run pio run -t compiledb
  @echo "Done. Use 'uv run pio run' to build."

pre-commit-init:
  uv run pre-commit install --hook-type pre-commit --hook-type pre-push

lsp-update:
  uv run pio run -t compiledb

build:
  export $(cat .env | xargs) && uv run pio run -e bare_metal -e rtos

clean:
  uv run pio run -t clean

hard-clean: && clean
  rm -rf .pio_docker .venv_docker
  rm -rf .pio/build
  rm -f sdkconfig

reset:
  rm -rf .pio .venv .uv compile_commands.json sdkconfig
  @echo "Project reset to factory settings."

test:
    uv run pio test -e native

test-all:
    uv run pio test -e native -e rtos -vvv

lint:
    uv run pio check --environment rtos

format:
    find app components -name "*.c" -o -name "*.h" | xargs clang-format -i

qa:
  just format
  just lint
  just test
  just build

sim env="rtos":
    sed 's|\${PIO_ENV}|{{env}}|g' wokwi.template.toml > wokwi.toml
    # mkdir -p .pio/build/{{env}}/bootloader/
    # ln -sf ../bootloader.bin .pio/build/{{env}}/bootloader/bootloader.bin
    # mkdir -p .pio/build/{{env}}/partition_table/
    # ln -sf ../partitions.bin .pio/build/{{env}}/partition_table/partition-table.bin
    # ln -sf firmware.bin .pio/build/rtos/current_monitoring_esp32.bin
    uv run python -m esptool --chip esp32 merge_bin \
        -o .pio/build/{{env}}/merged-flash.bin \
        --flash_mode dio --flash_freq 40m --flash_size 4MB \
        @.pio/build/{{env}}/flasher_args.json
    wokwi-cli --serial-log-file serial.log

# Headless check for CI
test-sim-env env="rtos" text="RTOS Task Started":
    sed 's|\${PIO_ENV}|{{env}}|g' wokwi.template.toml > wokwi.toml
    # mkdir -p .pio/build/{{env}}/bootloader/
    # ln -sf ../bootloader.bin .pio/build/{{env}}/bootloader/bootloader.bin
    # mkdir -p .pio/build/{{env}}/partition_table/
    # ln -sf ../partitions.bin .pio/build/{{env}}/partition_table/partition-table.bin
    # ln -sf firmware.bin .pio/build/rtos/current_monitoring_esp32.bin
    uv run esptool --chip esp32 merge-bin \
        --output .pio/build/{{env}}/merged-flash.bin \
        --flash-mode dio \
        --flash-freq 40m \
        --flash-size 4MB \
        @.pio/build/{{env}}/flasher_args.json
    @.pio/build/{{env}}/flasher_args.json

    wokwi-cli --timeout 15000 --expect-text "{{text}}" --serial-log-file serial.log

test-sim-all:
  just test-sim-env bare_metal "Bare Metal Boot"
  just test-sim-env rtos "RTOS Task Started"

down:
  docker stop {{container_name}} || true
  docker rm {{container_name}} || true
  docker compose -f docker-compose.yml down || true

build-image args="--progress='auto'": down
  @echo "Use 'just build-image --progress=\"plain\"' for more information. Options: auto (default), tty, plain, json, quiet"
  docker compose {{args}} -f docker-compose.yml build;

enter: down && down
  docker compose -f docker-compose.yml run -it --rm --name {{container_name}} {{docker-compose-service}} bash

dev target: down && down
    docker-compose run --rm {{docker-compose-service}} just {{target}}
