@echo off
setlocal

set IDF_VERSION v5.1.2
set IDF_PATH=%cd%\esp-idf

echo Setup ESP-IDF for Windows

git --version >nul 2>nul
if %errorlevel% neq 0 (
    echo Git is not installed or not in PATH
    echo Please install Git and try again
    exit /b 1
)

if exist "%IDF_PATH%" (
    echo ESP-IDF already downloaded at %IDF_PATH%
) else (
    echo Cloning ESP-IDF version %IDF_VERSION% into %IDF_PATH%...
    git clone --recursive --shallow-submodules --branch %IDF_VERSION% https://github.com/espressif/esp-idf.git "%IDF_PATH%"
    if %errorlevel% neq 0 (
        echo Failed to clone ESP-IDF
        exit /b 1
    )
)

echo Installing ESP-IDF tools...
call %IDF_PATH%\install.bat esp32

if %errorlevel% neq 0 (
    echo Failed to install ESP-IDF tools
    exit /b 1
)

echo Setup completed
echo Now you can call scripts\build.bat
