include config.mk

CFLAGS			+= -Iinclude

SRCS_C			:=
SRCS_S			:=

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
	@mkfs.fat -f 2 -F 12 -s 1 -R 5 fdd.img
	@dd status=none if=build/boot/boot.bin of=fdd.img bs=512 count=1 conv=notrunc
	@dd status=none if=build/boot/setup.bin of=fdd.img bs=512 seek=1 count=4 conv=notrunc
	@mcopy -i fdd.img build/image ::/

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)