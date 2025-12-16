@echo off
echo ==========================================
echo    Comprehensive WSL Fix Script
echo ==========================================
echo.
echo [1/4] Checking privileges...
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
echo [2/4] Enabling Required Windows Features...
echo Enable: Microsoft-Windows-Subsystem-Linux...
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
echo.
echo Enable: VirtualMachinePlatform...
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart

echo.
echo [3/4] Attempting to update WSL kernel...
wsl --update
if %errorlevel% neq 0 (
    echo.
    echo [WARNING] 'wsl --update' failed.
    echo Use this link to manually install the kernel update if issues persist:
    echo https://wslstorestorage.blob.core.windows.net/wslblob/wsl_update_x64.msi
)

echo.
echo [4/4] Attempting to install Ubuntu...
wsl --install -d Ubuntu

echo.
echo ==========================================
echo OPERATION COMPLETE
echo.
echo If you saw "The operation completed successfully" for the features, 
echo YOU MUST RESTART YOUR COMPUTER NOW.
echo.
echo If wsl --install works after restart, you are good to go.
echo ==========================================
pause
