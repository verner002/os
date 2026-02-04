#
# @file Makefile
# @author verner002
# @date 18/09/2025
#

MODULES := $(shell find ./src -mindepth 1 -maxdepth 1 -type d)
OBJECTS ?= $(shell find ./build -name "*.o" -not -type d)
MOUNT := $(shell tr -dc A-Za-z0-9 </dev/urandom | head -c 13)

all: directories modules link image

directories:
	mkdir -p ./bin
	mkdir -p ./build

modules:
	for module in $(MODULES) ; do \
		$(MAKE) -C $$module ; \
	done ;

link:
#	$(CL) $(OBJECTS) --oformat=pei-i386 -m i386pe --image-base 0x80000000 -e entry -o ./bin/kernel -g
	$(CL) -T./link.ld $(OBJECTS) -m elf_i386 -o ./bin/image
#	$(CL) -T./lds $(OBJECTS) -m elf_i386 -o ./bin/kernel
#	cat ./bin/head > ./bin/image
#	cat ./bin/kernel >> ./bin/image

image:
	$(DD) if=/dev/zero of=./fdd.img bs=512 count=2880
	$(MKFSFAT) -R 5 ./fdd.img
	$(DD) if=./bin/boot of=./fdd.img bs=512 count=1 conv=notrunc
	$(DD) if=./bin/setup of=./fdd.img bs=512 seek=1 count=4 conv=notrunc
	sudo mkdir /mnt/$(MOUNT)
	sudo mount ./fdd.img /mnt/$(MOUNT)
	sudo mkdir /mnt/$(MOUNT)/etc
	sudo cp ./bin/image /mnt/$(MOUNT)/image
	ls -l /mnt/$(MOUNT)
	sudo umount /mnt/$(MOUNT)
	sudo rm -rf /mnt/$(MOUNT)

.PHONY: image

clean:
	$(RM) -r ./build
	$(RM) -r ./bin
	$(RM) ./fdd.img

.PHONY: clean

CL ?= ld
DD ?= dd status=none
MKFSFAT ?= sudo mkfs.fat -f 2 -F 12 -s 1