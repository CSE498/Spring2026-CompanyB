#!/usr/bin/env bash
# Run tests with GCC 15 and Xcode SDK (avoids _bounds.h error on macOS)
SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk CXX=g++-15 make test
