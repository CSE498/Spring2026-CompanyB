FROM emscripten/emsdk:5.0.0

# Install Qt6 and GCC 12 for native builds.
# GCC 12 is required for some C++23 features
# We are using Ubuntu 22.04 LTS which only can download gcc-12 and not the latest.
# https://documentation.ubuntu.com/ubuntu-for-developers/reference/availability/gcc/
RUN apt-get update && apt-get install -y --no-install-recommends \
    qt6-base-dev \
    libgl-dev \
    gcc-12 \
    g++-12 \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 12 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 12 \
    && git config --system --add safe.directory '*'

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
