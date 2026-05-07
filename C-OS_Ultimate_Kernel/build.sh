#!/bin/bash
# C-OS Ultimate Build Script - Modular Version

set -e

echo "=== C-OS Ultimate Build ==="

# Directories
mkdir -p build/boot build/kernel isodir/boot/grub

# 1. Bootloader
echo "[1/5] Assembling bootloader..."
nasm -f elf32 src/boot/boot.asm -o build/boot/boot.o

# 2. Compile modules
echo "[2/5] Compiling kernel modules..."
g++ -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti \
    -I./src/kernel \
    -c src/kernel/vga.cpp -o build/kernel/vga.o

g++ -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti \
    -I./src/kernel \
    -c src/kernel/memory.cpp -o build/kernel/memory.o

g++ -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti \
    -I./src/kernel \
    -c src/kernel/keyboard.cpp -o build/kernel/keyboard.o

g++ -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti \
    -I./src/kernel \
    -c src/kernel/vfs.cpp -o build/kernel/vfs.o

g++ -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti \
    -I./src/kernel \
    -c src/kernel/kernel_new.cpp -o build/kernel/kernel.o

# 3. Link
echo "[3/5] Linking kernel..."
ld -m elf_i386 -T src/kernel/linker.ld -nostdlib \
    -o build/c-os-ultimate.bin \
    build/boot/boot.o \
    build/kernel/vga.o \
    build/kernel/memory.o \
    build/kernel/keyboard.o \
    build/kernel/vfs.o \
    build/kernel/kernel.o

# 4. Create ISO
echo "[4/5] Creating ISO..."
cp build/c-os-ultimate.bin isodir/boot/
cat > isodir/boot/grub/grub.cfg << 'EOF'
menuentry "C-OS Ultimate" {
    multiboot2 /boot/c-os-ultimate.bin
    boot
}
EOF
grub-mkrescue -o build/c-os-ultimate.iso isodir 2>&1 | tail -3

# 5. Done
echo "[5/5] Build complete!"
ls -lh build/c-os-ultimate.iso
