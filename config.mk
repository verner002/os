ARCH		?= x86

CC			:= gcc
AS			:= nasm
LD			:= ld

BUILD_DIR	:= build
TARGET		:= $(BUILD_DIR)/image

CFLAGS		:= -Wall -Wextra -Wno-pedantic \
			   -nostartfiles -nodefaultlibs -fno-builtin -nostdlib -nostdinc \
			   -fno-stack-protector -ffreestanding  -fno-pic \
			   -std=gnu17 \
			   -fno-asynchronous-unwind-tables -save-temps=obj -MMD -MP \
			   -O0 \
			   -m32 -march=i386 -mgeneral-regs-only -masm=intel -Wno-unused-parameter

ASFLAGS		:= -f elf32

LDFLAGS		:= -T linker.ld -nostdlib