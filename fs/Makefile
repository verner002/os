#
# @file Makefile
# @author verner002
# @date 18/09/2025
#

BUILD ?= ../../build/fs

C_FLAGS ?= -Wno-pedantic -Wall -Wextra -masm=intel -m32 -nostdlib -nodefaultlibs -nostartfiles -fno-pie -fno-asynchronous-unwind-tables -save-temps=obj -mgeneral-regs-only -fno-builtin -std=gnu17 -O0

SOURCES := $(shell find -mindepth 1 -maxdepth 1 -name "*.c" -not -type d -exec basename {} .c \;)
OBJECTS := $(SOURCES:%=$(BUILD)/%.o)

DRIVERS := $(shell find -mindepth 1 -maxdepth 1 -type d)

all: directories $(OBJECTS) drivers

directories:
	mkdir -p $(BUILD)

drivers:
	@for driver in $(DRIVERS) ; do \
		$(MAKE) -C $$driver ; \
	done ;

$(BUILD)/%.o: %.c
	$(CC) -I../../include $(C_FLAGS) -c $< -o $@

ASM ?= nasm