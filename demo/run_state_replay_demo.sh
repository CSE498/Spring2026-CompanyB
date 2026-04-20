#!/usr/bin/env bash
#Used AI to generate this script to run the demo easier
g++ -std=c++2b -Wall -Wextra -Wpedantic \
  demo/state_replay_demo_main.cpp source/tools/OutputManager.cpp \
  -I source -I third-party/include -o build/native/state_replay_demo &&
./build/native/state_replay_demo