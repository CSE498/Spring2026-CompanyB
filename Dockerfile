FROM ubuntu:24.04

# Prevent apt installs from asking for user input
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies.
# Install Qt6.
# GCC 14 is required for some C++23 features.
RUN apt-get update && apt-get install -y --no-install-recommends \
    qt6-base-dev \
    qt6-charts-dev \
    libgl-dev \
    gcc-14 \
    g++-14 \
    git \
    cmake \
    make \
    python3 \
    ca-certificates \
    xz-utils \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 14 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 14 \
    && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-14 14 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-14 14 \
    && git config --system --add safe.directory '*'

# Install Emscripten SDK 5.0.0
RUN git clone https://github.com/emscripten-core/emsdk.git /emsdk && \
    /emsdk/emsdk install 5.0.0 && \
    /emsdk/emsdk activate 5.0.0

ENV EMSDK=/emsdk \
    EM_CONFIG=/emsdk/.emscripten \
    PATH="/emsdk/upstream/emscripten:/emsdk:${PATH}"

# Start emsdk.sh for all bash scripts
ENV BASH_ENV=/emsdk/emsdk_env.sh

# Copy docker-entrypoint script and setup directories
COPY scripts/docker-entrypoint.sh /app/docker-entrypoint.sh
RUN mkdir -p /app/source /app/build /app/output && chmod +x /app/docker-entrypoint.sh

# Copy source code
COPY source/ /app/source/

WORKDIR /app

# Expose port for emrun web server
EXPOSE 8080

ENTRYPOINT ["/app/docker-entrypoint.sh"]

# Default command: build the project
CMD ["build"]
