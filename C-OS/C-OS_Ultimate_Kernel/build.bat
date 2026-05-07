@echo off
echo ========================================
echo C-OS Ultimate Kernel Build Script
echo ========================================
echo.

set PROJECT=C-OS_Ultimate_Kernel
cd /d C:\Users\cs02z\%PROJECT%

echo [1/5] Cleaning build directory...
if exist build rmdir /s /q build 2>nul
mkdir build\kernel 2>nul
mkdir build\boot 2>nul

echo [2/5] Compiling bootloader...
nasm -f elf32 src\boot\boot.asm -o build\boot\boot.o
if %errorlevel% neq 0 (
    echo ERROR: Bootloader compilation failed!
    pause
    exit /b 1
)

echo [3/5] Compiling kernel (all apps included)...
g++ -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -std=c++17 -c src\kernel\kernel.cpp -o build\kernel\kernel.o
if %errorlevel% neq 0 (
    echo ERROR: Kernel compilation failed!
    pause
    exit /b 1
)

echo [4/5] Linking kernel...
ld -m elf_i386 -nostdlib -T src\kernel\linker.ld -o build\c-os-ultimate.bin build\boot\boot.o build\kernel\kernel.o
if %errorlevel% neq 0 (
    echo ERROR: Linking failed!
    pause
    exit /b 1
)

echo [5/5] Build successful!
echo.
echo Binary: build\c-os-ultimate.bin
echo.
echo Starting QEMU...
echo.

"C:\Program Files\qemu\qemu-system-i386.exe" -kernel build\c-os-ultimate.bin

pause
