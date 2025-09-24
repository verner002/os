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
	$(ASM) -I../../include/asm -I. ./main.asm -o $(BIN)/boot.bin

ASM ?= nasm