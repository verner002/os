ENTRY(_start)
STARTUP(build/boot/head.o)
OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)

SECTIONS
{
  . = 0xc0000000;

  .text : {
    *(.text*)
  }

  .rodata ALIGN(16) : {
    *(.rodata*)
  }

  .data ALIGN(16) : {
    *(.data*)
  }

  .bss ALIGN(16) : {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end = .;
  }

  __bss_size = (__bss_end - __bss_start) / 4;
  __kernel_end = .;
}