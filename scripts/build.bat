@echo off
setlocal

set IDF_PATH=%cd%\esp-idf

if not exist "%IDF_PATH%\export.bat" (
    echo ESP-IDF not found at %IDF_PATH%.
    echo Please run 'scripts\setup.bat' first.
    exit /b 1
)

echo Setting up ESP-IDF build environment...
call %IDF_PATH%\export.bat

echo Environment is ready. Building...

if not exist build (
    mkdir build
)

cd build
cmake -G "Ninja" ..
cmake --build .
