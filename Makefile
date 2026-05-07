SHELL = /bin/bash
MKDIR = mkdir
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Wno-unused-function -I$(SRC_DIR)/include -I$(SRC_DIR)/kernel -I$(SRC_DIR)/kernel/arch -I$(SRC_DIR)/kernel/mm -I$(SRC_DIR)/kernel/mm/guard -I$(SRC_DIR)/kernel/drivers -I$(SRC_DIR)/drivers/video -I$(SRC_DIR)/drivers/input -I$(SRC_DIR)/drivers/disk -I$(SRC_DIR)/fs -I$(SRC_DIR)/gui -I$(SRC_DIR)/apps -I$(SRC_DIR)/bios -c
AS = nasm
ASFLAGS = -f elf32
LD = ld
LDFLAGS = -m elf_i386 -T src/boot/linker.ld

SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build
KERNEL_BIN = kernel.bin
STORAGE_IMG = $(BUILD_DIR)/storage.img

OBJS = $(OBJ_DIR)/boot.o \
       $(OBJ_DIR)/bios.o \
       $(OBJ_DIR)/kernel.o \
       $(OBJ_DIR)/io.o \
       $(OBJ_DIR)/idt.o \
       $(OBJ_DIR)/isr.o \
       $(OBJ_DIR)/irq.o \
       $(OBJ_DIR)/serial.o \
       $(OBJ_DIR)/timer.o \
       $(OBJ_DIR)/rtc.o \
       $(OBJ_DIR)/keyboard.o \
       $(OBJ_DIR)/mouse_minimal.o \
       $(OBJ_DIR)/mouse_irq.o \
       $(OBJ_DIR)/vga.o \
       $(OBJ_DIR)/gui.o \
       $(OBJ_DIR)/gui_apps.o \
       $(OBJ_DIR)/browser_new.o \
       $(OBJ_DIR)/storage_gui.o \
       $(OBJ_DIR)/password_screen.o \
       $(OBJ_DIR)/ide.o \
       $(OBJ_DIR)/storage.o \
       $(OBJ_DIR)/storage_core.o \
       $(OBJ_DIR)/storage_meta.o \
       $(OBJ_DIR)/storage1.o \
       $(OBJ_DIR)/storage2.o \
       $(OBJ_DIR)/storage3.o \
       $(OBJ_DIR)/storage4.o \
       $(OBJ_DIR)/storage5.o \
       $(OBJ_DIR)/storage6.o \
       $(OBJ_DIR)/storage7.o \
       $(OBJ_DIR)/storage8.o \
       $(OBJ_DIR)/storage9.o \
       $(OBJ_DIR)/storage10.o \
       $(OBJ_DIR)/storage11.o \
       $(OBJ_DIR)/storage12.o \
       $(OBJ_DIR)/storage13.o \
       $(OBJ_DIR)/storage14.o \
       $(OBJ_DIR)/storage15.o \
       $(OBJ_DIR)/storage16.o \
       $(OBJ_DIR)/storage17.o \
       $(OBJ_DIR)/storage18.o \
       $(OBJ_DIR)/storage19.o \
       $(OBJ_DIR)/storage20.o \
       $(OBJ_DIR)/storage21.o \
       $(OBJ_DIR)/fs.o \
       $(OBJ_DIR)/storage_advanced.o \
       $(OBJ_DIR)/memory.o \
       $(OBJ_DIR)/paging.o \
       $(OBJ_DIR)/gdt.o \
       $(OBJ_DIR)/task.o \
       $(OBJ_DIR)/scheduler.o \
       $(OBJ_DIR)/syscall.o \
       $(OBJ_DIR)/driver.o \
       $(OBJ_DIR)/pci.o \
       $(OBJ_DIR)/e1000.o \
       $(OBJ_DIR)/arp.o \
       $(OBJ_DIR)/udp.o \
       $(OBJ_DIR)/tcp.o \
       $(OBJ_DIR)/dhcp.o \
       $(OBJ_DIR)/dns.o \
       $(OBJ_DIR)/http.o \
       $(OBJ_DIR)/net.o \
       $(OBJ_DIR)/vfs.o \
       $(OBJ_DIR)/ramfs.o \
       $(OBJ_DIR)/fat32.o \
       $(OBJ_DIR)/string.o \
       $(OBJ_DIR)/storage_blob.o \
       $(OBJ_DIR)/mm_guard_init.o \
       $(OBJ_DIR)/mm_guard_check.o \
       $(OBJ_DIR)/mm_guard_alloc.o \
       $(OBJ_DIR)/mm_guard_stack.o \
       $(OBJ_DIR)/mm_guard_heap.o \
       $(OBJ_DIR)/mm_guard_page.o \
       $(OBJ_DIR)/mm_guard_debug.o

all: storage iso

# Storage directory setup
$(BUILD_DIR):
	$(MKDIR) -p $(BUILD_DIR)
	$(MKDIR) -p $(SRC_DIR)/storage

# Pack storage files into binary image using C tool
$(STORAGE_IMG): $(BUILD_DIR) tools/pack_storage.c
	gcc tools/pack_storage.c -o $(BUILD_DIR)/pack_storage
	$(BUILD_DIR)/pack_storage

# Convert storage image to object file for linking
$(OBJ_DIR)/storage_blob.o: $(STORAGE_IMG) | $(OBJ_DIR)
	objcopy -I binary -O elf32-i386 -B i386 $< $@

# Add dependency: rebuild storage when files change
storage: $(STORAGE_IMG) $(OBJ_DIR)/storage_blob.o

$(OBJ_DIR):
	$(MKDIR) -p $(OBJ_DIR)

# Boot files
$(OBJ_DIR)/boot.o: $(SRC_DIR)/boot/boot.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/isr.o: $(SRC_DIR)/boot/isr.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/irq.o: $(SRC_DIR)/boot/irq.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Kernel core
$(OBJ_DIR)/kernel.o: $(SRC_DIR)/kernel/kernel.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/io.o: $(SRC_DIR)/kernel/io.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/idt.o: $(SRC_DIR)/kernel/idt.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/net.o: $(SRC_DIR)/kernel/drivers/net.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/vfs.o: $(SRC_DIR)/fs/vfs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/ramfs.o: $(SRC_DIR)/fs/ramfs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/fat32.o: $(SRC_DIR)/fs/fat32.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/serial.o: $(SRC_DIR)/kernel/serial.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/timer.o: $(SRC_DIR)/kernel/timer.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/rtc.o: $(SRC_DIR)/kernel/rtc.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Memory management
$(OBJ_DIR)/memory.o: $(SRC_DIR)/kernel/mm/memory.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/paging.o: $(SRC_DIR)/kernel/mm/paging.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Architecture
$(OBJ_DIR)/gdt.o: $(SRC_DIR)/kernel/arch/gdt.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Kernel
$(OBJ_DIR)/task.o: $(SRC_DIR)/kernel/task.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/scheduler.o: $(SRC_DIR)/kernel/scheduler.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/syscall.o: $(SRC_DIR)/kernel/syscall.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Driver framework
$(OBJ_DIR)/driver.o: $(SRC_DIR)/kernel/drivers/driver.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/pci.o: $(SRC_DIR)/kernel/drivers/pci.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/e1000.o: $(SRC_DIR)/kernel/drivers/e1000.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/arp.o: $(SRC_DIR)/kernel/drivers/arp.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/udp.o: $(SRC_DIR)/kernel/drivers/udp.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/tcp.o: $(SRC_DIR)/kernel/drivers/tcp.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/dhcp.o: $(SRC_DIR)/kernel/drivers/dhcp.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/dns.o: $(SRC_DIR)/kernel/drivers/dns.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/http.o: $(SRC_DIR)/kernel/drivers/http.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Storage apps
$(OBJ_DIR)/storage1.o: $(SRC_DIR)/apps/storage1.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage2.o: $(SRC_DIR)/apps/storage2.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage3.o: $(SRC_DIR)/apps/storage3.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage4.o: $(SRC_DIR)/apps/storage4.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage5.o: $(SRC_DIR)/apps/storage5.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage6.o: $(SRC_DIR)/apps/storage6.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage7.o: $(SRC_DIR)/apps/storage7.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage8.o: $(SRC_DIR)/apps/storage8.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage9.o: $(SRC_DIR)/apps/storage9.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage10.o: $(SRC_DIR)/apps/storage10.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage11.o: $(SRC_DIR)/apps/storage11.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage12.o: $(SRC_DIR)/apps/storage12.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage13.o: $(SRC_DIR)/apps/storage13.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage14.o: $(SRC_DIR)/apps/storage14.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage15.o: $(SRC_DIR)/apps/storage15.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage16.o: $(SRC_DIR)/apps/storage16.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage17.o: $(SRC_DIR)/apps/storage17.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage18.o: $(SRC_DIR)/apps/storage18.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage19.o: $(SRC_DIR)/apps/storage19.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage20.o: $(SRC_DIR)/apps/storage20.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage21.o: $(SRC_DIR)/apps/storage21.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/browser_new.o: $(SRC_DIR)/gui/browser_new.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage_gui.o: $(SRC_DIR)/gui/storage_gui.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Memory guard system
$(OBJ_DIR)/mm_guard_init.o: $(SRC_DIR)/kernel/mm/guard/mm_guard_init.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mm_guard_check.o: $(SRC_DIR)/kernel/mm/guard/mm_guard_check.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mm_guard_alloc.o: $(SRC_DIR)/kernel/mm/guard/mm_guard_alloc.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mm_guard_stack.o: $(SRC_DIR)/kernel/mm/guard/mm_guard_stack.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mm_guard_heap.o: $(SRC_DIR)/kernel/mm/guard/mm_guard_heap.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mm_guard_page.o: $(SRC_DIR)/kernel/mm/guard/mm_guard_page.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mm_guard_debug.o: $(SRC_DIR)/kernel/mm/guard/mm_guard_debug.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Input drivers
$(OBJ_DIR)/keyboard.o: $(SRC_DIR)/drivers/input/keyboard.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mouse.o: $(SRC_DIR)/drivers/input/mouse.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mouse_simple.o: $(SRC_DIR)/drivers/input/mouse_simple.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mouse_minimal.o: $(SRC_DIR)/drivers/input/mouse_minimal.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/mouse_irq.o: $(SRC_DIR)/drivers/input/mouse_irq.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/usb_mouse.o: $(SRC_DIR)/drivers/input/usb_mouse.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/vga.o: $(SRC_DIR)/drivers/video/vga.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Disk drivers
$(OBJ_DIR)/ide.o: $(SRC_DIR)/drivers/disk/ide.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage.o: $(SRC_DIR)/drivers/disk/storage.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage_core.o: $(SRC_DIR)/drivers/disk/storage_core.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage_meta.o: $(SRC_DIR)/drivers/disk/storage_meta.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# GUI
$(OBJ_DIR)/gui.o: $(SRC_DIR)/gui/gui.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/gui_draw.o: $(SRC_DIR)/gui/gui_draw.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/gui_input.o: $(SRC_DIR)/gui/gui_input.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/gui_windows.o: $(SRC_DIR)/gui/gui_windows.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/password_screen.o: $(SRC_DIR)/gui/password_screen.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Filesystem
$(OBJ_DIR)/fs.o: $(SRC_DIR)/fs/fs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/storage_advanced.o: $(SRC_DIR)/fs/storage_advanced.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Library
$(OBJ_DIR)/string.o: $(SRC_DIR)/include/string.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# BIOS
$(OBJ_DIR)/bios.o: $(SRC_DIR)/bios/bios.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS)
	cp $(KERNEL_BIN) iso/boot/kernel.bin

iso: kernel.bin
	grub-mkrescue -o c-os-4-beta.iso iso

clean:
	rm -rf $(OBJ_DIR) $(KERNEL_BIN) c-os-4-beta.iso

$(OBJ_DIR)/gui_apps.o: $(SRC_DIR)/gui/gui_apps.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@
