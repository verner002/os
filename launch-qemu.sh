#!/bin/bash

qemu-system-x86_64 -fda ./fdd.img -device piix3-ide,id=ide -drive id=disk,file=disk.img,format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -nic model=e1000