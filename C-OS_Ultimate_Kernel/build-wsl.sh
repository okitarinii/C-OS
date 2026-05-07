#!/bin/bash
# WSL Build Script for C-OS Ultimate Kernel v2.0

set -e

echo "========================================"
echo "C-OS Ultimate Kernel v2.0 - WSL Build"
echo "========================================"
echo ""

cd /mnt/c/Users/cs02z/C-OS_Ultimate_Kernel

echo "[1/5] Cleaning build directory..."
rm -rf build/*
mkdir -p build/kernel build/boot

echo "[2/5] Compiling bootloader..."
nasm -f elf32 src/boot/boot.asm -o build/boot/boot.o

echo "[3/5] Compiling kernel..."
g++ -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -std=c++17 -c src/kernel/kernel.cpp -o build/kernel/kernel.o

echo "[4/5] Linking kernel..."
ld -m elf_i386 -nostdlib -T src/kernel/linker.ld -o build/c-os-ultimate.bin build/boot/boot.o build/kernel/kernel.o

echo "[5/5] Build successful!"
echo ""
echo "Files created:"
echo "  - build/c-os-ultimate.bin (Kernel binary)"
echo ""
echo "Next steps:"
echo "  1. QEMU: make run"
echo "  2. VirtualBox: make iso, then mount ISO"
echo ""
