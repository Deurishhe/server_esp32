#!/bin/bash
set -e

IDF_PATH="$(pwd)/esp-idf"

if [ ! -f "$IDF_PATH/export.sh" ]; then
    echo "ESP-IDF not found at $IDF_PATH"
    echo "Please run 'scripts/setup.sh' first"
    exit 1
fi

echo "Setting up ESP-IDF build environment..."
source $IDF_PATH/export.sh

echo "Environment is ready. Building..."

if [ ! -d "build" ]; then
    mkdir build
fi

cd build
cmake ..
cmake --build .
