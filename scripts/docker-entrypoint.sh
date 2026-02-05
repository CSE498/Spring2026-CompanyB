#!/bin/bash
set -e

# Default values
export SDL_VERSION="${SDL_VERSION:-3}"
export BUILD_OUTPUT="${BUILD_OUTPUT:-html}"
export SERVE_PORT="${SERVE_PORT:-8080}"
export SOURCE_DIR="${SOURCE_DIR:-/app/source}"
export OUTPUT_DIR="${OUTPUT_DIR:-/app/output}"
export BUILD_DIR="${BUILD_DIR:-/app/build}"

show_help() {
    echo
    echo "Commands:"
    echo "  build       Build the project (default)"
    echo "  test        Build and run tests"
    echo "  serve       Build and serve with emrun"
    echo "  clean       Clean build artifacts"
    echo "  shell       Start an interactive shell"
    echo "  help        Show this help message"
    echo ""
    echo "Environment Variables:"
    echo "  SDL_VERSION   SDL version to use (2 or 3, default: 3)"
    echo "  BUILD_OUTPUT  Output format: html, js (default: html)"
    echo "  SERVE_PORT    Port for emrun server (default: 8080)"
    echo "  SOURCE_DIR    Source directory (default: /app/source)"
    echo "  OUTPUT_DIR    Output directory (default: /app/output)"
    echo
    echo "Examples:"
    echo "  docker run --rm -v \$(pwd)/source:/app/source -v \$(pwd)/output:/app/output cse498-companyb-project build"
    echo "  docker run --rm -v \$(pwd)/source:/app/source -v \$(pwd)/tests:/app/tests cse498-companyb-project test"
    echo "  docker run --rm -p 8080:8080 cse498-companyb-project serve"
    echo "  docker run -it --rm cse498-companyb-project shell"
    echo
}

do_build() {
    echo "==> Building with Emscripten (SDL${SDL_VERSION}, output: ${BUILD_OUTPUT}) <=="

    if [ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]; then
        echo "Error: No CMakeLists.txt found in ${SOURCE_DIR}"
        exit 1
    fi

    mkdir -p "${BUILD_DIR}" "${OUTPUT_DIR}"

    # Configure with emcmake
    emcmake cmake \
        -S "${SOURCE_DIR}" \
        -B "${BUILD_DIR}" \
        -DSDL_VERSION="${SDL_VERSION}" \
        -DBUILD_OUTPUT="${BUILD_OUTPUT}" \
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${OUTPUT_DIR}"

    # Build with emmake
    # The j flag is for parallel compilation which should help speed up compile jobs later
    emmake make -C "${BUILD_DIR}" -j$(nproc)

    echo "==> Build complete! <=="
    ls -lh "${OUTPUT_DIR}"
}

do_serve() {
    do_build

    echo "==> Starting emrun server on port ${SERVE_PORT}... <=="
    echo "==> Access at http://localhost:${SERVE_PORT} <=="

    emrun \
        --port "$SERVE_PORT" \
        --hostname 0.0.0.0 \
        --no_browser \
        --serve_after_close \
        "${OUTPUT_DIR}/index.html"
}

do_test() {
    echo "==> Building and running tests (Catch2 + Emscripten) <=="

    if [ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]; then
        echo "Error: No CMakeLists.txt found in ${SOURCE_DIR}"
        exit 1
    fi

    local TEST_BUILD_DIR="${BUILD_DIR}/tests"
    mkdir -p "${TEST_BUILD_DIR}"

    # Configure with emcmake for Emscripten tests
    emcmake cmake \
        -S "${SOURCE_DIR}" \
        -B "${TEST_BUILD_DIR}" \
        -DBUILD_TESTS=ON

    # Build tests with emmake
    emmake make -C "${TEST_BUILD_DIR}" -j$(nproc)

    # Run tests with Node.js
    echo "==> Running tests <=="
    node "${TEST_BUILD_DIR}/tests.js"
}

do_clean() {
    echo "==> Cleaning build artifacts <=="
    rm -rf "${BUILD_DIR}"/* "${OUTPUT_DIR}"/*
}

# Main command handler
case "${1:-build}" in
    build)
        do_build
        ;;
    test)
        do_test
        ;;
    serve)
        do_serve
        ;;
    clean)
        do_clean
        ;;
    shell)
        exec /bin/bash
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        # Pass through to exec for custom commands
        exec "$@"
        ;;
esac
