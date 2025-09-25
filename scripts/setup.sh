#!/bin/bash
set -e

IDF_VERSION="v5.1.2"
IDF_PATH="$(pwd)/esp-idf"

echo "Setup ESP-IDF for Linux/macOS"

if ! command -v git &> /dev/null; then
    echo "Git is not installed or not in PATH"
    echo "Please install Git and try again"
    exit 1
fi

if [ -d "$IDF_PATH" ]; then
    echo "ESP-IDF already downloaded at $IDF_PATH"
else
    echo "Cloning ESP-IDF version ${IDF_VERSION} into ${IDF_PATH}..."
    git clone --recursive --shallow-submodules --branch "${IDF_VERSION}" https://github.com/espressif/esp-idf.git "${IDF_PATH}"
fi

echo "Installing ESP-IDF tools..."
$IDF_PATH/install.sh esp32

echo "Setup completed"
echo "Now you can call scripts/build.bat"
