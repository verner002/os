include config.mk

CFLAGS			+= -Iinclude

SRCS_C			:=
SRCS_S			:=

MOUNT_POINT		:= $(shell tr -dc A-Za-z0-9 </dev/urandom | head -c 13)

include boot/boot.mk
include kernel/kernel.mk

OBJS_C			:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS_C))
OBJS_S			:= $(patsubst %.s,$(BUILD_DIR)/%.o,$(SRCS_S))
OBJS			:= $(OBJS_C) $(OBJS_S)
DEPS			:= $(OBJS_C:.o=.d)

.PHONY: all clean floppy-image

all: $(TARGET) floppy-image

$(TARGET): $(ARCH_OBJS) $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

floppy-image:
	@dd status=none if=/dev/zero of=fdd.img bs=512 count=2880
	@sudo mkfs.fat -f 2 -F 12 -s 1 -R 5 fdd.img
	@dd status=none if=build/boot/boot.bin of=fdd.img bs=512 count=1 conv=notrunc
	@dd status=none if=build/boot/setup.bin of=fdd.img bs=512 seek=1 count=4 conv=notrunc
	@sudo mkdir -p /mnt/$(MOUNT_POINT)
	@sudo mount fdd.img /mnt/$(MOUNT_POINT)
	@sudo cp build/image /mnt/$(MOUNT_POINT)
	@sudo umount /mnt/$(MOUNT_POINT)
	@sudo rm -rf /mnt/$(MOUNT_POINT)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)