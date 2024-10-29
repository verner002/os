# OS
Operating system written in x86 Assembly and C for the i486 architecture or later revisions.

# Boot Loader

## Memory Map (for ext2)
A minimum of 128 MiB of RAM is required for booting.

- `0x00000000 - 0x000003ff` - Interrupt Vector Table
- `0x00000400 - 0x000004ff` - BIOS Data Area
- `0x00000500 - 0x00007bff` - Free (stack)
- `0x00007c00 - 0x00007dff` - Boot Loader (stage 1, always 512 bytes)
- `0x00007e00 - 0x00007fff` - Boot Loader (stage 2, always 512 bytes)
- `0x00008000 - 0x00008400` - Superblock (always 1024 bytes)
- `0x00010000 - 0x00020000` - Kernel