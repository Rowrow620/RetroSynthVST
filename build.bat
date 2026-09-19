@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Building RetroBitSynth (VST3 ^& Standalone)
echo ===================================================

set CMAKE_EXE="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

if not exist %CMAKE_EXE% (
    echo CMake not found at default VS2022 location. Trying system PATH...
    set CMAKE_EXE=cmake
)

echo.
echo [1/2] Configuring project with CMake...
%CMAKE_EXE% -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo Error during CMake configuration.
    pause
    exit /b 1
)

echo.
echo [2/2] Compiling Release target...
%CMAKE_EXE% --build build --config Release --parallel
if errorlevel 1 (
    echo Compilation failed.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo   Build Successful!
echo ===================================================
echo VST3 plugin location:
echo   build\SimpleSynthVST_artefacts\Release\VST3\RetroBitSynth.vst3
echo.
echo Standalone executable:
echo   build\SimpleSynthVST_artefacts\Release\Standalone\RetroBitSynth.exe
echo ===================================================

pause
