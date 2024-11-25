#
# OS Makefile
#
# Author: verner002
#

BOOT_TARGET ?= boot.bin
LOADER_TARGET ?= loader.sys
KERNEL_TARGET ?= kernel.sys

DEBUG ?= ./debug
INC ?= ./include
SRC ?= ./src
BUILD ?= ./build
BIN ?= ./bin

BOOT_SRC ?= $(SRC)/boot
DRIVERS_SRC ?= $(SRC)/drivers
KSTDLIB_SRC ?= $(SRC)/kstdlib
LOADER_SRC = $(SRC)/loader
KERNEL_SRC ?= $(SRC)/kernel

DRIVERS_BUILD ?= $(BUILD)/drivers
KSTDLIB_BUILD ?= $(BUILD)/kstdlib
LOADER_BUILD ?= $(BUILD)/loader
KERNEL_BUILD ?= $(BUILD)/kernel

DRIVERS_SRCS := $(shell find $(DRIVERS_SRC) -name *.c -exec basename {} .c \;)
DRIVERS_OBJS := $(DRIVERS_SRCS:%=$(DRIVERS_BUILD)/%.o)

KSTDLIB_SRCS := $(shell find $(KSTDLIB_SRC) -name *.c -exec basename {} .c \;)
KSTDLIB_OBJS := $(KSTDLIB_SRCS:%=$(KSTDLIB_BUILD)/%.o)

#LOADER_SRCS := $(shell find $(LOADER_SRC) -name *.c -exec basename {} .c \;)
#LOADER_OBJS := $(LOADER_SRCS:%=$(LOADER_BUILD)/%.o)

KERNEL_SRCS := $(shell find $(KERNEL_SRC) -name *.c -exec basename {} .c \;)
KERNEL_OBJS := $(KERNEL_SRCS:%=$(KERNEL_BUILD)/%.o)

INCS := $(INC)
INC_FLAGS := $(addprefix -I,$(INCS))

ASM_FLAGS ?= -I$(INC)/asm
C_FLAGS ?= $(INC_FLAGS) -Wno-pedantic -Wall -Wextra -masm=intel -m32 -nostdlib -nodefaultlibs -nostartfiles -fno-pie -fno-asynchronous-unwind-tables

MOUNT := $(shell tr -dc A-Za-z0-9 </dev/urandom | head -c 13)

all: dirs boot drivers kstdlib loader kernel

dirs:
	$(MKDIR) $(BUILD)
	$(MKDIR) $(DRIVERS_BUILD)
	$(MKDIR) $(KSTDLIB_BUILD)
	$(MKDIR) $(LOADER_BUILD)
	$(MKDIR) $(KERNEL_BUILD)
	$(MKDIR) $(BIN)

boot:
	$(ASM) $(ASM_FLAGS) $(BOOT_SRC)/main.asm -o $(BIN)/$(BOOT_TARGET)
	$(ASM) $(ASM_FLAGS) $(BOOT_SRC)/header.asm -o $(BIN)/header.bin

loader: #$(LOADER_OBJS) # produces raw binary file (loader objs must come first!!!)
#$(CL) $(LOADER_OBJS) $(DRIVERS_OBJS) $(KSTDLIB_OBJS) -m16 -Ttext=0x00010000 --oformat=binary -m elf_i386 -e entry -o $(BIN)/$(LOADER_TARGET)
	$(ASM) $(ASM_FLAGS) -I$(SRC)/loader $(LOADER_SRC)/main.asm -o $(BIN)/$(LOADER_TARGET)

kernel: $(KERNEL_OBJS) # temporarily changed image base so i can load kernel even without page manager
	$(CL) $(KERNEL_OBJS) $(DRIVERS_OBJS) $(KSTDLIB_OBJS) --oformat=pei-i386 -m i386pe --image-base 0x00090000 -e entry -o $(BIN)/$(KERNEL_TARGET)

drivers: $(DRIVERS_OBJS)

$(DRIVERS_BUILD)/%.o: $(DRIVERS_SRC)/%.c
	$(CC) $(C_FLAGS) -c $< -o $@

kstdlib: $(KSTDLIB_OBJS)

$(KSTDLIB_BUILD)/%.o: $(KSTDLIB_SRC)/%.c
	$(CC) $(C_FLAGS) -c $< -o $@

$(LOADER_BUILD)/%.o: $(LOADER_SRC)/%.c
	$(CC) $(C_FLAGS) -c $< -o $@

$(KERNEL_BUILD)/%.o: $(KERNEL_SRC)/%.c
	$(CC) $(C_FLAGS) -c $< -o $@

image: all
	$(DD) if=/dev/zero of=./fdd.img bs=512 count=2880
#$(DD) if=./bin/boot.bin of=./fdd.img bs=512 count=2 conv=notrunc
#$(DD) if=./bin/header.bin of=./fdd.img bs=512 count=2 seek=2 conv=notrunc
	$(MKFSFAT) ./fdd.img
	$(DD) if=./bin/boot.bin of=./fdd.img bs=512 count=1 conv=notrunc

	sudo mkdir /mnt/$(MOUNT)
	sudo mount ./fdd.img /mnt/$(MOUNT)
	sudo cp ./bin/loader.sys /mnt/$(MOUNT)/loader.sys
	sudo cp ./bin/kernel.sys /mnt/$(MOUNT)/kernel.sys
	sudo umount /mnt/$(MOUNT)
	sudo rm -rf /mnt/$(MOUNT)

debug-gdb:
	qemu-system-i386 -fda ./fdd.img -S -s & gdb --quiet -x $(DEBUG)/config.gdb

debug-bochs:
	bochs -q -f $(DEBUG)/config.bochs

.PHONY: clean

clean:
	$(RM) -r $(BUILD)
	$(RM) -r $(BIN)
	$(RM) ./fdd.img

ASM ?= nasm
CC ?= gcc
CL ?= ld
MKDIR ?= mkdir -p
DD ?= dd status=none
MKFSFAT ?= sudo mkfs.fat -f 2 -F 12 -s 1