# FROM python:3.11-slim
FROM espressif/idf:v5.2.1

RUN apt-get update && apt-get install -y \
    curl \
    git \
    cmake \
    ninja-build \
    clang-format \
    cppcheck \
    python3-dev \
    python3-pip \
    python3-venv \
    libffi-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN curl -LsSf https://astral.sh/uv/install.sh | BINDIR=/usr/local/bin sh
RUN curl --proto '=https' --tlsv1.2 -sSf https://just.systems/install.sh | bash -s -- --to /usr/local/bin

ENV PATH="/usr/local/bin:/root/.local/bin:$PATH"

WORKDIR /project

RUN uv pip install --system platformio

RUN pio platform install espressif32
