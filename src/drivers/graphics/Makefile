#
# @file Makefile
# @author verner002
# @date 22/11/2025
#

BUILD ?= ../../../build/drivers/graphics

C_FLAGS ?= -Wno-pedantic -Wall -Wextra -masm=intel -m32 -nostdlib -nodefaultlibs -nostartfiles -fno-pie -fno-asynchronous-unwind-tables -save-temps=obj -mgeneral-regs-only -fno-builtin -std=gnu17 -g

SOURCES := $(shell find -mindepth 1 -maxdepth 1 -name "*.c" -not -type d -exec basename {} .c \;)
OBJECTS := $(SOURCES:%=$(BUILD)/%.o)

all: directories $(OBJECTS)

directories:
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c
	$(CC) -I../../../include $(C_FLAGS) -c $< -o $@

ASM ?= nasm