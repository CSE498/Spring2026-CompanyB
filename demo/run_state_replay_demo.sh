#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build/native"
BIN="$BUILD_DIR/state_replay_demo"

mkdir -p "$BUILD_DIR"

echo "[demo] Building state replay demo..."
g++ -std=c++2b -Wall -Wextra -Wpedantic \
  "$REPO_ROOT/demo/state_replay_demo_main.cpp" \
  "$REPO_ROOT/source/tools/OutputManager.cpp" \
  -I "$REPO_ROOT/source" \
  -I "$REPO_ROOT/third-party/include" \
  -o "$BIN"

echo "[demo] Running..."
"$BIN"

