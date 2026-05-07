# Makefile for C-OS Ultimate Kernel

CC = g++
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -std=c++17
ASM = nasm
ASFLAGS = -f elf32
LD = ld
LDFLAGS = -m elf_i386 -nostdlib

SRCDIR = src
BUILDDIR = build

KERNEL_SRCS = $(wildcard $(SRCDIR)/kernel/*.cpp)
KERNEL_OBJS = $(patsubst $(SRCDIR)/kernel/%.cpp,$(BUILDDIR)/kernel/%.o,$(KERNEL_SRCS))
BOOT_OBJ = $(BUILDDIR)/boot/boot.o

TARGET = $(BUILDDIR)/c-os-ultimate.bin
ISO = $(BUILDDIR)/c-os-ultimate.iso

QEMU = qemu-system-i386

.PHONY: all clean run iso

all: $(TARGET)

$(BUILDDIR)/kernel:
	mkdir -p $(BUILDDIR)/kernel

$(BUILDDIR)/boot:
	mkdir -p $(BUILDDIR)/boot

$(BUILDDIR)/kernel/%.o: $(SRCDIR)/kernel/%.cpp | $(BUILDDIR)/kernel
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/boot/%.o: $(SRCDIR)/boot/%.asm | $(BUILDDIR)/boot
	$(ASM) $(ASFLAGS) $< -o $@

$(TARGET): $(BOOT_OBJ) $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -T $(SRCDIR)/kernel/linker.ld $^ -o $@

iso: $(TARGET)
	mkdir -p isodir/boot/grub
	cp $(TARGET) isodir/boot/
	echo 'menuentry "C-OS Ultimate" {' > isodir/boot/grub/grub.cfg
	echo '    multiboot2 /boot/c-os-ultimate.bin' >> isodir/boot/grub/grub.cfg
	echo '    boot' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) isodir

run: $(TARGET)
	$(QEMU) -kernel $(TARGET)

run-iso: $(ISO)
	$(QEMU) -cdrom $(ISO)

clean:
	rm -rf $(BUILDDIR)/* isodir/boot/*.bin

# Windows build script
win-build:
	@echo "Building on Windows with WSL..."
	wsl -d Ubuntu bash -c 'cd /mnt/c/Users/cs02z/C-OS_Ultimate_Kernel && make clean && make'

win-run:
	"C:\Program Files\qemu\qemu-system-i386.exe" -kernel $(TARGET)
