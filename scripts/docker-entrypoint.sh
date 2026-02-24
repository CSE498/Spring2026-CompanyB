#!/bin/bash
set -e

# Default values
export SERVE_PORT="${SERVE_PORT:-8080}"
export SOURCE_DIR="${SOURCE_DIR:-/app/source}"
export BUILD_DIR="${BUILD_DIR:-/app/build}"

show_help() {
    echo
    echo "! THIS SCRIPT IS ONLY RECOMMENDED FOR USE IN THE INTERACTIVE SHELL !"
    echo ""
    echo "Commands:"
    echo "  build-emscripten    Build the project with Emscripten"
    echo "  build-native        Build the project natively with Qt"
    echo "  test-emscripten     Build and run Emscripten tests"
    echo "  test-native         Build and run native tests"
    echo "  serve               Build and serve with emrun"
    echo "  clean               Clean build artifacts"
    echo "  shell               Start an interactive shell"
    echo "  help                Show this help message"
    echo ""
    echo "Environment Variables:"
    echo "  SERVE_PORT         Port for emrun server (default: 8080)"
    echo "  SOURCE_DIR         Source directory (default: /app/source)"
    echo "  BUILD_DIR          Build directory (default: /app/build)"
    echo
}

do_build-emscripten() {
    echo "==> Building with Emscripten <==="

    if [ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]; then
        echo "Error: No CMakeLists.txt found in ${SOURCE_DIR}"
        exit 1
    fi

    mkdir -p "${BUILD_DIR}"

    # Configure with emcmake
    emcmake cmake \
        -S "${SOURCE_DIR}" \
        -B "${BUILD_DIR}/emscripten" \
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${BUILD_DIR}/emscripten"

    # Build with emmake
    # The j flag is for parallel compilation which should help speed up compile jobs later
    emmake make -C "${BUILD_DIR}/emscripten" -j$(nproc)

    echo "==> Build complete! <==="
    ls -lh "${BUILD_DIR}/emscripten"
}

find_qt6_dir() {
    find /usr -name Qt6Config.cmake -exec dirname {} \; 2>/dev/null | head -1
}

do_build_native() {
    echo "==> Building natively with Qt <==="

    if [ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]; then
        echo "Error: No CMakeLists.txt found in ${SOURCE_DIR}"
        exit 1
    fi

    local NATIVE_BUILD_DIR="${BUILD_DIR}/native"
    local QT6_DIR
    QT6_DIR="$(find_qt6_dir)"
    mkdir -p "${NATIVE_BUILD_DIR}"

    cmake \
        -S "${SOURCE_DIR}" \
        -B "${NATIVE_BUILD_DIR}" \
        ${QT6_DIR:+-DQt6_DIR="${QT6_DIR}"}

    cmake --build "${NATIVE_BUILD_DIR}" --parallel

    echo "==> Native build complete! <==="
}

do_serve() {
    do_build-emscripten

    echo "==> Starting emrun server on port ${SERVE_PORT}... <==="
    echo "==> Access at http://localhost:${SERVE_PORT} <==="

    emrun \
        --port "$SERVE_PORT" \
        --hostname 0.0.0.0 \
        --no_browser \
        --serve_after_close \
        "${BUILD_DIR}/emscripten/index.html"
}

do_test_emscripten() {
    echo "==> Building and running tests (Catch2, Emscripten) <==="

    if [ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]; then
        echo "Error: No CMakeLists.txt found in ${SOURCE_DIR}"
        exit 1
    fi

    local TEST_BUILD_DIR="${BUILD_DIR}/tests/emscripten"
    mkdir -p "${TEST_BUILD_DIR}"

    # Configure for Emscripten specific wasm tests
    emcmake cmake \
        -S "${SOURCE_DIR}" \
        -B "${TEST_BUILD_DIR}" \
        -DBUILD_WASM_TESTS=ON

    # Build tests with emmake
    emmake make -C "${TEST_BUILD_DIR}" -j$(nproc)

    # Run tests with Node.js
    echo "==> Running tests <==="
    node "${TEST_BUILD_DIR}/tests.js"
}

do_test_native() {
    echo "==> Building and running tests (Catch2, native) <==="

    if [ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]; then
        echo "Error: No CMakeLists.txt found in ${SOURCE_DIR}"
        exit 1
    fi

    local TEST_BUILD_DIR="${BUILD_DIR}/tests/native"
    local QT6_DIR
    QT6_DIR="$(find_qt6_dir)"
    mkdir -p "${TEST_BUILD_DIR}"

    cmake \
        -S "${SOURCE_DIR}" \
        -B "${TEST_BUILD_DIR}" \
        -DBUILD_TESTS=ON \
        ${QT6_DIR:+-DQt6_DIR="${QT6_DIR}"}

    cmake --build "${TEST_BUILD_DIR}" --parallel

    echo "==> Running tests <==="
    QT_QPA_PLATFORM=offscreen "${TEST_BUILD_DIR}/tests"
}

do_clean() {
    echo "==> Cleaning build artifacts <==="
    rm -rf "${BUILD_DIR}"/* "${OUTPUT_DIR}"/*
}

# Main command handler
case "${1:-build}" in
    build-emscripten)
        do_build-emscripten
        ;;
    build-native)
        do_build_native
        ;;
    test-emscripten)
        do_test_emscripten
        ;;
    test-native)
        do_test_native
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
