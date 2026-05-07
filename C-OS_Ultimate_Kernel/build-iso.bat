@echo off
echo ========================================
echo C-OS Ultimate - ISO Build for VirtualBox
echo ========================================
echo.

cd /d C:\Users\cs02z\C-OS_Ultimate_Kernel

echo [1/2] Building kernel with WSL...
wsl -d Ubuntu bash -c "cd /mnt/c/Users/cs02z/C-OS_Ultimate_Kernel && make clean && make"
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo [2/2] Creating ISO image...
wsl -d Ubuntu bash -c "cd /mnt/c/Users/cs02z/C-OS_Ultimate_Kernel && make iso"
if %errorlevel% neq 0 (
    echo ISO creation failed!
    pause
    exit /b 1
)

echo.
echo ISO created: build\c-os-ultimate.iso
echo.
echo Next steps:
echo 1. Open VirtualBox
echo 2. Create new VM: C-OS Ultimate
echo 3. Type: Other/Unknown 32-bit
echo 4. Mount ISO: build\c-os-ultimate.iso
echo 5. Start VM
echo.
pause
