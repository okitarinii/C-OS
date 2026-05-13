SHELL := /bin/bash

CC := gcc
AS := nasm
LD := ld
OBJCOPY := objcopy
HOSTCC := gcc

CFLAGS := -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -fno-builtin -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-sign-compare -Isrc/include -Isrc/kernel -Isrc/kernel/arch -Isrc/drivers/video -Isrc/drivers/input -Isrc/drivers/disk -Isrc/fs -Isrc/gui -Isrc/bios
ASFLAGS := -f elf32
LDFLAGS := -m elf_i386 -T src/boot/linker.ld

OBJ_DIR := obj
BUILD_DIR := build
ISO_DIR := iso
GRUB_DIR := $(ISO_DIR)/boot/grub
GRUB_CFG := $(GRUB_DIR)/grub.cfg
KERNEL := kernel.bin
ISO := C-OS-4.0.2.iso
STORAGE_IMG := $(BUILD_DIR)/storage.img
PACK_STORAGE := $(BUILD_DIR)/pack_storage

KERNEL_OBJS := \
	$(OBJ_DIR)/boot/boot.o \
	$(OBJ_DIR)/boot/isr.o \
	$(OBJ_DIR)/boot/irq.o \
	$(OBJ_DIR)/boot/storage_blob.o \
	$(OBJ_DIR)/bios/bios.o \
	$(OBJ_DIR)/kernel/kernel.o \
	$(OBJ_DIR)/kernel/io.o \
	$(OBJ_DIR)/kernel/idt.o \
	$(OBJ_DIR)/kernel/irq.o \
	$(OBJ_DIR)/kernel/serial.o \
	$(OBJ_DIR)/kernel/timer.o \
	$(OBJ_DIR)/kernel/memory.o \
	$(OBJ_DIR)/kernel/keyboard.o \
	$(OBJ_DIR)/kernel/string.o \
	$(OBJ_DIR)/kernel/arch/gdt.o \
	$(OBJ_DIR)/drivers/input/mouse.o \
	$(OBJ_DIR)/drivers/video/vga.o \
	$(OBJ_DIR)/drivers/disk/storage.o \
	$(OBJ_DIR)/drivers/disk/storage_core.o \
	$(OBJ_DIR)/drivers/disk/storage_meta.o \
	$(OBJ_DIR)/fs/fs.o \
	$(OBJ_DIR)/gui/gui.o \
	$(OBJ_DIR)/gui/gui_apps.o \
	$(OBJ_DIR)/kernel/rtc.o

.PHONY: all kernel iso run clean dirs

all: iso
kernel: $(KERNEL)

$(OBJ_DIR) $(BUILD_DIR):
	mkdir -p $(OBJ_DIR) $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)/boot $(OBJ_DIR)/bios $(OBJ_DIR)/kernel $(OBJ_DIR)/kernel/arch
	mkdir -p $(OBJ_DIR)/drivers/input $(OBJ_DIR)/drivers/video $(OBJ_DIR)/drivers/disk
	mkdir -p $(OBJ_DIR)/fs $(OBJ_DIR)/gui

$(PACK_STORAGE): tools/pack_storage.c | $(BUILD_DIR)
	$(HOSTCC) tools/pack_storage.c -o $(PACK_STORAGE)

$(STORAGE_IMG): $(PACK_STORAGE) | $(BUILD_DIR)
	./$(PACK_STORAGE)

$(OBJ_DIR)/boot/storage_blob.o: $(STORAGE_IMG) | $(OBJ_DIR)
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 $(STORAGE_IMG) $@

$(OBJ_DIR)/boot/boot.o: src/boot/boot.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/boot/isr.o: src/boot/isr.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/boot/irq.o: src/boot/irq.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/bios/bios.o: src/bios/bios.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/kernel.o: src/kernel/kernel.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/io.o: src/kernel/io.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/idt.o: src/kernel/idt.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/irq.o: src/kernel/irq.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/serial.o: src/kernel/serial.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/timer.o: src/kernel/timer.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/memory.o: src/kernel/memory.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/keyboard.o: src/kernel/keyboard.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/string.o: src/kernel/string.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/arch/gdt.o: src/kernel/arch/gdt.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/input/mouse.o: src/drivers/input/mouse.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/video/vga.o: src/drivers/video/vga.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/disk/storage.o: src/drivers/disk/storage.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/disk/storage_core.o: src/drivers/disk/storage_core.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/disk/storage_meta.o: src/drivers/disk/storage_meta.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/fs/fs.o: src/fs/fs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/gui/gui.o: src/gui/gui.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/gui/gui_apps.o: src/gui/gui_apps.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/rtc.o: src/kernel/rtc.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(GRUB_CFG):
	mkdir -p $(GRUB_DIR)
	printf 'set timeout=0\nset default=0\n\nmenuentry "C-OS 4.0.2" {\n    multiboot /boot/kernel.bin\n    boot\n}\n' > $(GRUB_CFG)

$(KERNEL): $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o $(KERNEL) $(KERNEL_OBJS)
	mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin

iso: $(KERNEL) $(GRUB_CFG)
	grub-mkrescue -o $(ISO) $(ISO_DIR)

run: iso
	qemu-system-i386 -cdrom $(ISO) -m 256M -serial stdio -display sdl

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR) $(KERNEL) $(ISO) $(ISO_DIR)/boot/kernel.bin $(GRUB_DIR)
