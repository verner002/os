BOOT_DIR		:= boot

BOOT_SRCS_S		:= $(BOOT_DIR)/boot/boot.s $(BOOT_DIR)/boot/setup.s
BOOT_OBJS		:= $(patsubst $(BOOT_DIR)/boot/%.s,$(BUILD_DIR)/boot/%.bin,$(BOOT_SRCS_S))

HEAD_SRCS_S		:= $(BOOT_DIR)/boot/head.s
HEAD_OBJS		:= $(patsubst $(BOOT_DIR)/boot/%.s,$(BUILD_DIR)/boot/%.o,$(HEAD_SRCS_S))

ARCH_OBJS		:= $(BOOT_OBJS) $(HEAD_OBJS)

SRCS_C			+= $(wildcard $(BOOT_DIR)/kernel/*.c) $(wildcard $(BOOT_DIR)/mm/*.c)
SRCS_S			+= $(wildcard $(BOOT_DIR)/kernel/*.s) $(wildcard $(BOOT_DIR)/mm/*.s)

$(BUILD_DIR)/boot/%.bin: $(BOOT_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) -I$(BOOT_DIR) -f bin $< -o $@

$(BUILD_DIR)/head/%.o: $(BOOT_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@