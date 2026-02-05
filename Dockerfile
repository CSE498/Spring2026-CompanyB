FROM emscripten/emsdk:5.0.0

# Build arguments
ARG SDL_VERSION=2

# Environment variables for runtime configuration
ENV SDL_VERSION=${SDL_VERSION}
ENV BUILD_OUTPUT=html
ENV SERVE_PORT=8080

# Pre-fetch SDL port to speed up first builds
RUN emcc --use-port=sdl${SDL_VERSION} --check

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
