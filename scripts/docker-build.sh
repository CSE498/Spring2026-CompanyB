#!/bin/bash
set -e

show_help() {
    echo "Usage: docker-build.sh <service> [variant] [TARGET_MAIN=] [NO_QT=]"
    echo
    echo "Services:"
    echo "  build-native       Build natively with Qt"
    echo "  build-emscripten   Build with Emscripten"
    echo "  serve              Build with Emscripten and serve on port 8080"
    echo "  dev                Interactive development shell"
    echo
    echo "Variants (base warnings always included):"
    echo "  (none)   Base warnings only"
    echo "  debug    Base warnings + CMAKE_BUILD_TYPE=Debug"
    echo "  opt      Base warnings + CMAKE_BUILD_TYPE=Release (-O3 -DNDEBUG)"
    echo "  quick    Base warnings + -DNDEBUG (fast compile, no optimization)"
    echo "  grumpy   Base warnings + -Wconversion -Weffc++"
    echo ""
    echo "Environment variables:"
    echo "  NO_QT=1                    Exclude Qt and GUI sources (native and dev only)"
    echo "  TARGET_MAIN=<file>         Entry point file (searches source/ then demo/)"
    echo "                             Default: simple_main.cpp (native), web_main.cpp (emscripten)"
    echo "  Note: NO_QT is ignored for build-emscripten and serve."
}

SERVICE="$1"
VARIANT="${2:-}"

if [ -n "${NO_QT}" ] && [ "${NO_QT}" != "1" ] && [ "${NO_QT}" != "0" ]; then
    echo "==========================================================="
    echo "NO_QT must be either 1 or 0. (True and False respectively)!";
    echo "==========================================================="
    exit 1;
fi

case "$SERVICE" in
  help|--help|-h|"")
    show_help
    exit 0
    ;;
  build-native|build-emscripten|serve|dev) ;;
  *)
    echo "Unknown service '$SERVICE'."
    echo "Valid services: build-native | build-emscripten | serve | dev"
    exit 1
    ;;
esac

FLAGS_WARN="-Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic"

case "$VARIANT" in
  debug)  export CMAKE_BUILD_TYPE=Debug
          export CMAKE_EXTRA_FLAGS="$FLAGS_WARN" ;;
  opt)    export CMAKE_BUILD_TYPE=Release
          export CMAKE_EXTRA_FLAGS="$FLAGS_WARN -O3 -DNDEBUG" ;;
  quick)  export CMAKE_EXTRA_FLAGS="$FLAGS_WARN -DNDEBUG" ;;
  grumpy) export CMAKE_EXTRA_FLAGS="$FLAGS_WARN -Wconversion -Weffc++" ;;
  "")     export CMAKE_EXTRA_FLAGS="$FLAGS_WARN";;
  *)
    echo "Unknown variant '$VARIANT'."
    echo "Valid variants: default | debug | opt | quick | grumpy"
    exit 1
    ;;
esac

if [ "$SERVICE" = "serve" ]; then
    docker compose up "$SERVICE"
else
    docker compose run --rm "$SERVICE"
fi
