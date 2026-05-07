CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -I$(SRC_DIR)/include -I$(SRC_DIR)/kernel -I$(SRC_DIR)/kernel/arch -I$(SRC_DIR)/kernel/mm -I$(SRC_DIR)/drivers/video -I$(SRC_DIR)/drivers/input -I$(SRC_DIR)/drivers/disk -I$(SRC_DIR)/fs -I$(SRC_DIR)/gui -c
AS = nasm
ASFLAGS = -f elf32
LD = ld
LDFLAGS = -m elf_i386 -T src/boot/linker.ld

SRC_DIR = src
OBJ_DIR = obj
KERNEL_BIN = kernel.bin

OBJS = $(OBJ_DIR)/boot.o \
       $(OBJ_DIR)/kernel.o \
       $(OBJ_DIR)/io.o \
       $(OBJ_DIR)/idt.o \
       $(OBJ_DIR)/isr.o \
       $(OBJ_DIR)/irq.o \
       $(OBJ_DIR)/serial.o \
       $(OBJ_DIR)/timer.o \
       $(OBJ_DIR)/rtc.o \
       $(OBJ_DIR)/keyboard.o \
       $(OBJ_DIR)/mouse.o \
       $(OBJ_DIR)/mouse_simple.o \
       $(OBJ_DIR)/mouse_minimal.o \
       $(OBJ_DIR)/vga.o \
       $(OBJ_DIR)/gui.o \
       $(OBJ_DIR)/password_screen.o \
       $(OBJ_DIR)/ide.o \
       $(OBJ_DIR)/fs.o \
       $(OBJ_DIR)/memory.o \
       $(OBJ_DIR)/string.o

all: iso

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

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

$(OBJ_DIR)/serial.o: $(SRC_DIR)/kernel/serial.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/timer.o: $(SRC_DIR)/kernel/timer.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/rtc.o: $(SRC_DIR)/kernel/rtc.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Memory management
$(OBJ_DIR)/memory.o: $(SRC_DIR)/kernel/mm/memory.c | $(OBJ_DIR)
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

# Video drivers
$(OBJ_DIR)/vga.o: $(SRC_DIR)/drivers/video/vga.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Disk drivers
$(OBJ_DIR)/ide.o: $(SRC_DIR)/drivers/disk/ide.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# GUI
$(OBJ_DIR)/gui.o: $(SRC_DIR)/gui/gui.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/password_screen.o: $(SRC_DIR)/gui/password_screen.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Filesystem
$(OBJ_DIR)/fs.o: $(SRC_DIR)/fs/fs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Library
$(OBJ_DIR)/string.o: $(SRC_DIR)/include/string.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS)
	cp $(KERNEL_BIN) iso/boot/kernel.bin

iso: kernel.bin
	grub-mkrescue -o c-os.iso iso

clean:
	rm -rf $(OBJ_DIR) $(KERNEL_BIN) c-os.iso
