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
  uv run pre-commit install --hook-type pre-commit --hook-type pre-push

lsp-update:
  uv run pio run -t compiledb

build:
  uv run pio run -e bare_metal -e rtos

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

lint:
    uv run pio check --environment rtos

format:
    find app components -name "*.c" -o -name "*.h" | xargs clang-format -i

qa:
  just format
  just lint
  just test
  just build

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
