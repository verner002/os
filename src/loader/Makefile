#
# @file Makefile
# @author verner002
# @date 18/09/2025
#

BIN ?= ../../bin

all: directories loader

directories:
	mkdir -p $(BIN)

loader:
	$(ASM) -I../../include/asm -I. ./main.asm -o $(BIN)/loader.sys

ASM ?= nasm