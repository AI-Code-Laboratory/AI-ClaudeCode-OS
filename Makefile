# Makefile — windOS 0.01
#
# Produit un noyau ELF 32 bits multiboot (windos.elf), lançable
# directement avec `qemu-system-i386 -kernel windos.elf`, ou via
# une ISO GRUB pour une vraie machine (voir README.md).

CC      = gcc
AS      = as
LD      = ld

CFLAGS  = -m32 -std=gnu99 -ffreestanding -fno-stack-protector \
          -fno-pic -Wall -Wextra -Os -nostdlib -c
ASFLAGS = --32
LDFLAGS = -m elf_i386 -T src/linker.ld -nostdlib

SRC_DIR = src
BUILD_DIR = build

OBJS = $(BUILD_DIR)/boot.o \
       $(BUILD_DIR)/kernel.o \
       $(BUILD_DIR)/vga.o \
       $(BUILD_DIR)/keyboard.o \
       $(BUILD_DIR)/strutil.o \
       $(BUILD_DIR)/dosexec.o \
       $(BUILD_DIR)/basicexec.o

.PHONY: all clean run size

all: windos.elf

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.S | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

windos.elf: $(OBJS) src/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Lance le noyau dans QEMU (a executer sur TA machine, pas ici).
run: windos.elf
	qemu-system-i386 -kernel windos.elf

size: windos.elf
	@echo "Taille du noyau :"
	@ls -lh windos.elf
	@echo "Detail des sections :"
	@objdump -h windos.elf | grep -E '\.text|\.rodata|\.data|\.bss'

clean:
	rm -rf $(BUILD_DIR) windos.elf
