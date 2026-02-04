ENTRY(_start)
STARTUP(./bin/head)
OUTPUT_FORMAT(binary)

SECTIONS
{
  . = 0x00000000;

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
}