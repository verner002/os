#
# OS Makefile
#
# Author: verner002
#

BOOT_TARGET ?= boot.bin
LOADER_TARGET ?= loader.sys
KERNEL_TARGET ?= kernel.sys

BOOT_SRC ?= ./src/boot
DRIVERS_SRC ?= ./src/drivers
KSTDLIB_SRC ?= ./src/kstdlib
LOADER_SRC = ./src/loader
KERNEL_SRC ?= ./src/kernel
INC ?= ./include
BUILD ?= ./build
DRIVERS_BUILD ?= $(BUILD)/drivers
KSTDLIB_BUILD ?= $(BUILD)/kstdlib
LOADER_BUILD ?= $(BUILD)/loader
KERNEL_BUILD ?= $(BUILD)/kernel
BIN ?= ./bin

DRIVERS_SRCS := $(shell find $(DRIVERS_SRC) -name *.c -exec basename {} .c \;)
DRIVERS_OBJS := $(DRIVERS_SRCS:%=$(DRIVERS_BUILD)/%.o)

KSTDLIB_SRCS := $(shell find $(KSTDLIB_SRC) -name *.c -exec basename {} .c \;)
KSTDLIB_OBJS := $(KSTDLIB_SRCS:%=$(KSTDLIB_BUILD)/%.o)

LOADER_SRCS := $(shell find $(LOADER_SRC) -name *.c -exec basename {} .c \;)
LOADER_OBJS := $(LOADER_SRCS:%=$(LOADER_BUILD)/%.o)

KERNEL_SRCS := $(shell find $(KERNEL_SRC) -name *.c -exec basename {} .c \;)
KERNEL_OBJS := $(KERNEL_SRCS:%=$(KERNEL_BUILD)/%.o)

INCS := $(INC)
INC_FLAGS := $(addprefix -I,$(INCS))

ASM_FLAGS ?= -I$(INC)/asm
C_FLAGS ?= $(INC_FLAGS) -Wno-pedantic -Wall -Wextra -masm=intel -m32 -nostdlib -nodefaultlibs -nostartfiles -fno-pie

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

loader: $(LOADER_OBJS) # produces raw binary file (loader objs must come first!!!)
	$(CL) $(LOADER_OBJS) $(DRIVERS_OBJS) $(KSTDLIB_OBJS) -Ttext=0x00010000 --oformat=binary -m elf_i386 -e entry -o $(BIN)/$(LOADER_TARGET)

kernel: $(KERNEL_OBJS)
	$(CL) $(KERNEL_OBJS) $(DRIVERS_OBJS) $(KSTDLIB_OBJS) --oformat=pei-i386 -m i386pe --image-base 0x00010000 -e entry -o $(BIN)/$(KERNEL_TARGET)

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

.PHONY: clean

clean:
	$(RM) -r $(BUILD)
	$(RM) -r $(BIN)

ASM ?= nasm
CC ?= gcc
CL ?= ld
MKDIR ?= mkdir -p
ECHO ?= echo