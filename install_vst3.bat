@echo off
:: Self-elevation script
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"

if '%errorlevel%' NEQ '0' (
    echo Requesting Administrator privileges to install VST3...
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    echo UAC.ShellExecute "cmd.exe", "/k """"%~f0"" elevated""", "", "runas", 1 >> "%temp%\getadmin.vbs"
    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /b
)

cd /d "%~dp0"

echo.
echo =======================================================
echo   Installing RetroBitSynth to Windows VST3 Folder
echo =======================================================

set "SOURCE_DIR=%~dp0build\SimpleSynthVST_artefacts\Release\VST3\RetroBitSynth.vst3"
set "DEST_DIR=C:\Program Files\Common Files\VST3\RetroBitSynth.vst3"

if not exist "%SOURCE_DIR%" (
    echo [ERROR] Could not find source VST3 at:
    echo   %SOURCE_DIR%
    pause
    exit /b 1
)

echo Creating "C:\Program Files\Common Files\VST3"...
if not exist "C:\Program Files\Common Files\VST3" (
    mkdir "C:\Program Files\Common Files\VST3"
)

echo Copying plugin files...
xcopy /E /I /Y /Q "%SOURCE_DIR%" "%DEST_DIR%"

:: Grant Full Control to Users so future builds can update automatically without elevation
icacls "%DEST_DIR%" /grant *S-1-5-32-545:(OI)(CI)F /T /Q >nul 2>&1

echo.
if exist "%DEST_DIR%" (
    echo =======================================================
    echo   SUCCESS!
    echo   RetroBitSynth.vst3 installed successfully to:
    echo   C:\Program Files\Common Files\VST3\RetroBitSynth.vst3
    echo =======================================================
    echo.
    echo Now switch to FL Studio Plugin Manager:
    echo 1. Click "Find installed plugins"
    echo 2. RetroBitSynth will appear in the list!
) else (
    echo [ERROR] Failed to copy files.
)

echo.
pause
