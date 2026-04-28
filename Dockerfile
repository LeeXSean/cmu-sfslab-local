FROM ubuntu:22.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        clang-format \
        gdb \
        liblua5.4-dev \
        lua5.4 \
        make \
        pkg-config \
        shellcheck \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work/sfslab
CMD ["/bin/bash"]
