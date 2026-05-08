alias i:= init
# alias bi:= build-image
alias b:= build
alias c:= clean
# alias e:= enter

default:
  just --list

init:
  @echo "Initializing the project"
  uv run platformio project init --board esp32dev --project-option "framework=espidf"
  @echo "Generating compilation database for Neovim/clangd..."
  uv run pio run -t compiledb
  @echo "Done. Use 'uv run pio run' to build."

lsp-update:
  uv run pio run -t compiledb

build:
  uv run pio run

clean:
  uv run pio run -t clean

hard-clean: && clean
  rm -rf .pio/build
  rm -f sdkconfig

reset:
  rm -rf .pio .venv .uv compile_commands.json sdkconfig
  @echo "Project reset to factory settings."
