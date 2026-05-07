# Initialize project, environment, and LSP for Neovim
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
