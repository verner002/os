#!/bin/bash

objdump -b binary -m i386 --adjust-vma=0xc0000000 -D ./build/image > ./dump