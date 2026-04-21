#!/bin/bash
set -e

# Default values
export SERVE_PORT="${SERVE_PORT:-8080}"
export SOURCE_DIR="${SOURCE_DIR:-/app/source}"
export BUILD_DIR="${BUILD_DIR:-/app/build}"
export GNUTLS_CPUID_OVERRIDE=0x1
export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

# Qt refuses XDG_RUNTIME_DIR not owned by the running user, which causes
# QApplication to hang during init. Give each user their own dir.
export XDG_RUNTIME_DIR="/tmp/xdg-runtime-$(id -u)"
mkdir -p "${XDG_RUNTIME_DIR}"
chmod 700 "${XDG_RUNTIME_DIR}"

# Qt (via libQt6DBus) tries to auto-launch dbus-daemon for a session bus and
# hangs indefinitely when none is available in the container. Point at a
# non-existent socket so the connect fails fast.
export DBUS_SESSION_BUS_ADDRESS="unix:path=/dev/null"

# Restore build directory ownership to the host user
fix_permissions() {
    if [ -n "${HOST_UID}" ] && [ "${HOST_UID}" != "0" ]; then
        chown -R "${HOST_UID}:${HOST_GID:-${HOST_UID}}" "${BUILD_DIR}"
    fi
}

# Always restore permissions on exit even if a failure occurs
trap fix_permissions EXIT

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
    echo "  NO_QT              Set to 1 to exclude Qt and GUI sources (native and dev only)"
    echo "  TARGET_MAIN        Entry point file to build (searches source/ then demo/)"
    echo "                     Default: simple_main.cpp (native), web_main.cpp (emscripten)"
    echo "  CMAKE_BUILD_TYPE   Passed directly to cmake (e.g. Debug, Release)"
    echo "  CMAKE_EXTRA_FLAGS  Extra C++ flags passed to cmake"
    echo
}

do_build_emscripten() {
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
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${BUILD_DIR}/emscripten" \
        ${CMAKE_BUILD_TYPE:+-DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"} \
        ${CMAKE_EXTRA_FLAGS:+-DCMAKE_CXX_FLAGS="${CMAKE_EXTRA_FLAGS}"} \
        ${TARGET_MAIN:+-DTARGET_MAIN="${TARGET_MAIN}"}

    # Build with emmake
    # The j flag is for parallel compilation which should help speed up compile jobs later
    emmake make -C "${BUILD_DIR}/emscripten" -j$(nproc)

    echo "==> Build complete! <==="
    ls -lh "${BUILD_DIR}/emscripten"
    fix_permissions
}

find_qt6_dir() {
    find /usr -name Qt6Config.cmake -exec dirname {} \; 2>/dev/null | head -1
}

do_build_native() {
    if [ "${NO_QT:-0}" = "1" ]; then
        echo "==> Building natively (Qt excluded) <==="
    else
        echo "==> Building natively with Qt <==="
    fi

    if [ -n "${TARGET_MAIN}" ] && [[ "${TARGET_MAIN}" != *_main.cpp ]]; then
        echo "Error: TARGET_MAIN='${TARGET_MAIN}' must end in _main.cpp"
        exit 1
    fi

    if [ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]; then
        echo "Error: No CMakeLists.txt found in ${SOURCE_DIR}"
        exit 1
    fi

    local NATIVE_BUILD_DIR="${BUILD_DIR}/docker-native"
    local QT6_DIR
    QT6_DIR="$(find_qt6_dir)"
    mkdir -p "${NATIVE_BUILD_DIR}"

    cmake \
        -S "${SOURCE_DIR}" \
        -B "${NATIVE_BUILD_DIR}" \
        ${CMAKE_BUILD_TYPE:+-DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"} \
        ${CMAKE_EXTRA_FLAGS:+-DCMAKE_CXX_FLAGS="${CMAKE_EXTRA_FLAGS}"} \
        $( [ "${NO_QT:-0}" = "1" ] && echo "-DNO_QT=ON" ) \
        ${TARGET_MAIN:+-DTARGET_MAIN="${TARGET_MAIN}"} \
        ${QT6_DIR:+-DQt6_DIR="${QT6_DIR}"}

    cmake --build "${NATIVE_BUILD_DIR}" --parallel

    echo "==> Native build complete! <==="
    fix_permissions
}

do_serve() {
    do_build_emscripten

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
    fix_permissions
}

do_test_native() {
    echo "==> Building and running tests (Catch2, native) <==="

    if [ "${NO_QT:-0}" = "1" ]; then
        echo "==> Building and running tests (Catch2, native, Qt excluded) <==="
    else
        echo "==> Building and running tests (Catch2, native, Qt) <==="
    fi

    if [ ! -f "${SOURCE_DIR}/CMakeLists.txt" ]; then
        echo "Error: No CMakeLists.txt found in ${SOURCE_DIR}"
        exit 1
    fi

    local TEST_BUILD_DIR="${BUILD_DIR}/tests/docker-native"
    local QT6_DIR
    QT6_DIR="$(find_qt6_dir)"
    mkdir -p "${TEST_BUILD_DIR}"

    cmake \
        -S "${SOURCE_DIR}" \
        -B "${TEST_BUILD_DIR}" \
        -DBUILD_TESTS=ON \
        $( [ "${NO_QT:-0}" = "1" ] && echo "-DNO_QT=ON" ) \
        ${QT6_DIR:+-DQt6_DIR="${QT6_DIR}"}

    cmake --build "${TEST_BUILD_DIR}" --parallel

    echo "==> Running tests <==="
    QT_QPA_PLATFORM=offscreen "${TEST_BUILD_DIR}/tests"
    fix_permissions
}

do_clean() {
    echo "==> Cleaning build artifacts <==="
    rm -rf "${BUILD_DIR}"/*
}

# Main command handler
case "${1:-build}" in
    build-emscripten)
        do_build_emscripten
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
