FROM espressif/idf:v5.2.1

RUN apt-get update && apt-get install -y \
    curl \
    git \
    cmake \
    ninja-build \
    clang-format \
    cppcheck \
    && rm -rf /var/lib/apt/lists/*

RUN curl -LsSf https://astral.sh/uv/install.sh | BINDIR=/usr/local/bin sh
RUN curl --proto '=https' --tlsv1.2 -sSf https://just.systems/install.sh | bash -s -- --to /usr/local/bin

ENV PATH="/usr/local/bin:/root/.local/bin:$PATH"

WORKDIR /project

RUN uv pip install --system platformio

RUN pio platform install espressif32

RUN curl -L https://wokwi.com/ci/install.sh | sh && \
    mv /root/.wokwi/bin/wokwi-cli /usr/local/bin/
