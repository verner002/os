#
# @file Makefile
# @author verner002
# @date 18/09/2025
#

VERSION := "\"0.0.1\""
ARCH := "\"x86\""

BUILD ?= ../../build/kernel

C_FLAGS ?= -Wno-pedantic -Wall -Wextra -masm=intel -m32 -nostdlib -nodefaultlibs -nostartfiles -fno-pie -fno-asynchronous-unwind-tables -save-temps=obj -mgeneral-regs-only -fno-builtin -std=gnu17 -g

SOURCES := $(shell find -mindepth 1 -maxdepth 1 -name "*.c" -not -type d -exec basename {} .c \;)
OBJECTS := $(SOURCES:%=$(BUILD)/%.o)

all: directories pdac $(OBJECTS)

directories:
	mkdir -p $(BUILD)

pdac: # make platform dependent assembly code
	$(ASM) ts.s -felf -o $(BUILD)/ts.o
	$(ASM) tas.s -felf -o $(BUILD)/tas.o

$(BUILD)/%.o: %.c
	$(CC) -I../../include $(C_FLAGS) -D VERSION=$(VERSION) -D ARCH=$(ARCH) -c $< -o $@

ASM ?= nasm