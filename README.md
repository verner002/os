# OS
Operating system written in x86 Assembly and C for the i486 architecture or later revisions.

# Boot Loader

## Physical Memory Map
A minimum of 32 MiB of RAM is required for booting.\
This memory map is designed for FAT12 boot loader. 

| Start | End | Size | Description |
|:-----:|:---:|:----:|-------------|
| `0x00000000` | `0x000003ff` | 1 KiB | Interrupt Vector Table
| `0x00000400` | `0x000004ff` | 256 B | BIOS Data Area
| `0x00000500` | `0x00005fff` | 22.75 KiB | Boot Loader Stack
| `0x00006000` | `0x000069ff` | 2.5 KiB | Free
| `0x00006a00` | `0x00007bff` | 4.5 KiB | File Allocation Table (up to 9 sectors)
| `0x00007c00` | `0x00007dff` | 512 B | Boot Loader
| `0x00007e00` | `0x00007fff` | 512 B | Free
| `0x00008000` | `0x00009bff` | 7 KiB | Root Directory
| `0x00008000` | `0x0000bfff` | 16 KiB | Loader
| `0x0000c000` | `0x0000cfff` | 4 KiB | SMAP (up to 341 12-byte address range descriptors)
| `0x0000d000` | `0x0000dfff` | 4 KiB | Page Directory
| `0x0000e000` | `0x0000efff` | 4 KiB | First MiB Page Table
| `0x0000f000` | `0x0000ffff` | 4 KiB | Kernel Page Table
| `0x00010000` | `0x0007ffff` | 448 KiB | Kernel Image
| `0x00080000` | `0x0009ffff` | 128 KiB | Extended BIOS Data Area (mostly)
| `0x000a0000` | `0x000bffff` | 128 KiB | Video Memory
| `0x000c0000` | `0x000c7fff` | 32 KiB | Video BIOS
| `0x000c8000` | `0x000effff` | 160 KiB | BIOS Expansion
| `0x000f0000` | `0x000fffff` | 64 KiB | ROM BIOS
| `0x00100000` | `0x004fffff` | 4 MiB | Kernel
| `0x00500000` | `0x00efffff` | 10 MiB | Free
| `0x00f00000` | `0x00ffffff` | 1 MiB | ISA Memory Hole (possibly memory mapped hardware)
| `0x01000000` | `0xffffffff` | 4080 MiB | Free and memory mapped hardware