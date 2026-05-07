@echo off
echo ========================================
echo C-OS Ultimate Kernel v2.0 - Quick Start
echo ========================================
echo.

cd /d C:\Users\cs02z\C-OS_Ultimate_Kernel

if not exist build\c-os-ultimate.bin (
    echo Binary not found. Building with WSL...
    echo.
    wsl -d Ubuntu bash -c "cd /mnt/c/Users/cs02z/C-OS_Ultimate_Kernel && ./build-wsl.sh"
    if %errorlevel% neq 0 (
        echo Build failed!
        pause
        exit /b 1
    )
)

echo.
echo Starting C-OS Ultimate v2.0...
echo.

"C:\Program Files\qemu\qemu-system-i386.exe" -kernel build\c-os-ultimate.bin

if %errorlevel% neq 0 (
    echo.
    echo QEMU failed to start!
    echo Make sure QEMU is installed at C:\Program Files\qemu\
    pause
)
