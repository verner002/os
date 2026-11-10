CFLAGS			+= -m32 -march=i386 -mgeneral-regs-only -masm=intel -Wno-unused-parameter

ARCH_DIR		:= arch/x86

BOOT_SRCS_S		:= $(ARCH_DIR)/boot/boot.s $(ARCH_DIR)/boot/setup.s
BOOT_OBJS		:= $(patsubst $(ARCH_DIR)/boot/%.s,$(BUILD_DIR)/arch/x86/boot/%.bin,$(BOOT_SRCS_S))

HEAD_SRCS_S		:= $(ARCH_DIR)/boot/head.s
HEAD_OBJS		:= $(patsubst $(ARCH_DIR)/boot/%.s,$(BUILD_DIR)/arch/x86/boot/%.o,$(HEAD_SRCS_S))

ARCH_OBJS		:= $(BOOT_OBJS) $(HEAD_OBJS)

SRCS_C			+= $(wildcard $(ARCH_DIR)/kernel/*.c) $(wildcard $(ARCH_DIR)/mm/*.c)
SRCS_S			+= $(wildcard $(ARCH_DIR)/kernel/*.s) $(wildcard $(ARCH_DIR)/mm/*.s)

$(BUILD_DIR)/arch/x86/boot/%.bin: $(ARCH_DIR)/boot/%.s
	@mkdir -p $(dir $@)
	$(AS) -I$(ARCH_DIR)/boot -f bin $< -o $@

$(BUILD_DIR)/arch/x86/head/%.o: $(ARCH_DIR)/head/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@