#
# @file Makefile
# @author verner002
# @date 18/09/2025
#

BIN ?= ../../bin

all: directories boot

directories:
	mkdir -p $(BIN)

boot:
	$(ASM) -I../../include/asm -I. ./boot.s -o $(BIN)/boot
	$(ASM) -I../../include/asm -I. ./setup.s -o $(BIN)/setup
	$(ASM) -I../../include/asm -I. -felf ./head.s -o $(BIN)/head

ASM ?= nasm