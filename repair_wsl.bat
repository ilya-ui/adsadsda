@echo off
echo ==========================================
echo      WSL Repair/Reinstall Script
echo ==========================================
echo.
echo [1/3] Checking privileges...
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Success: Administrative permissions confirmed.
) else (
    echo Failure: Current permissions are inadequate.
    echo Please right-click this script and select "Run as Administrator".
    pause
    exit
)

echo.
echo [2/3] Attempting generic repair (wsl --update)...
wsl --update
if %errorlevel% equ 0 (
    echo Success: WSL updated and repaired.
    goto :End
)

echo.
echo [3/3] Update failed. Reinstalling Windows Feature...
echo Disabling WSL Feature...
dism.exe /online /disable-feature /featurename:Microsoft-Windows-Subsystem-Linux /norestart

echo.
echo Enabling WSL Feature...
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart

echo.
echo ==========================================
echo OPERATION COMPLETE
echo You MUST restart your computer if DISM ran above.
echo ==========================================

:End
pause
