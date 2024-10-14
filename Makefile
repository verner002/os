#
# OS Makefile
#
# Author: verner002
#

TARGET ?= kernel.exe

BOOT_SRC ?= ./src/boot
KSTDLIB_SRC ?= ./src/kstdlib
KERNEL_SRC ?= ./src/kernel
INC ?= ./include
BUILD ?= ./build
BIN ?= ./bin

KSTDLIB_SRCS := $(shell find $(KSTDLIB_SRC) -name *.c -exec basename {} .c \;)
KSTDLIB_OBJS := $(KSTDLIB_SRCS:%=$(BUILD)/%.o)

KERNEL_SRCS := $(shell find $(KERNEL_SRC) -name *.c -exec basename {} .c \;)
KERNEL_OBJS := $(KERNEL_SRCS:%=$(BUILD)/%.o)

INCS := $(INC)
INC_FLAGS := $(addprefix -I,$(INCS))

ASM_FLAGS ?= -I$(INC)/asm
C_FLAGS ?= $(INC_FLAGS) -Wno-pedantic -Wall -Wextra -masm=intel -m32 -nostdlib -nodefaultlibs -nostartfiles -fno-pie

all: init boot kernel

init:
	$(MKDIR) $(BUILD)
	$(MKDIR) $(BIN)

boot:
	$(ASM) $(ASM_FLAGS) $(BOOT_SRC)/boot.asm -o $(BIN)/boot.bin

kernel: $(KSTDLIB_OBJS) $(KERNEL_OBJS)
	$(CL) $(KSTDLIB_OBJS) $(KERNEL_OBJS) --oformat=pei-i386 -m i386pe --image-base 0x00010000 -e entry -o $(BIN)/$(TARGET) 

$(BUILD)/%.o: $(KSTDLIB_SRC)/%.c
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD)/%.o: $(KERNEL_SRC)/%.c
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