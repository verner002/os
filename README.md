# OS
Operating system written in x86 Assembly and C for the i486 architecture or later revisions.

# Bootloader

## Memory Map (for FAT12)
A minimum of 128 MiB of RAM is required for booting.

- `0x00000000 - 0x000003ff` - Interrupt Vector Table
- `0x00000400 - 0x000004ff` - BIOS Data Area
- `0x00000500 - 0x00007bff` - Free (Bootloader Stack)
- `0x00007c00 - 0x00007dff` - Bootloader
- `0x00008000 - 0x00008fff` - File Allocation Table
- `0x00010000 - 0x00020000` - Loader/Kernel