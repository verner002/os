# OS
Operating system written in x86 Assembly and C for the i486 architecture or later revisions.

# Building the project
## Dependencies
At least these versions of listed packages are required:
```
gcc >= 14.2.1
ld >= 2.44
nasm >= 2.16.03
make >= 4.4.1
dd >= 9.6
mkfs.fat >= 4.2
```

## Build
To build the project use:
```
$ make
```

To create a bootable image use:
```
$ make image
```

# Stage 1

## Physical Memory Map
This is a memory map for Stage 1 that assumes there is at most 1 MiB of memory.\
The main goal of this stage is to load `loader` and `kernel` binaries and pass control to the second stage.

| Start | End | Size | Type | Description |
|:-----:|:---:|:----:|------|-------------|
| `0x00000000` | `0x000003ff` | 1 KiB | Reserved | Interrupt Vector Table
| `0x00000400` | `0x000004ff` | 256 B | Reserved | BIOS Data Area
| `0x00000500` | `0x000077ff` | 29440 B | Free | Usable Memory
| `0x00007800` | `0x00007bff` | 1 KiB | Stage 1 | Stack
| `0x00007c00` | `0x00007dff` | 512 B | Stage 1 | Stage 1 (first part)
| `0x00007e00` | `0x00007fff` | 512 B | Stage 1 | Stage 1 (second part, optinal)
| `0x00008000` | `0x00008fff` | 4 KiB | Stage 2 | Stage 2
| `0x00009000` | `0x0000ffff` | 28 KiB | Free | Usable Memory
| `0x00010000` | `0x0007ffff` | 448 KiB | Stage 2 | Kernel Image
| `0x00080000` | `0x0009ffff` | 128 KiB | Reserved | Extended BIOS Data Area (mostly)
| `0x000a0000` | `0x000bffff` | 128 KiB | Reserved | Video Memory
| `0x000c0000` | `0x000c7fff` | 32 KiB | Reserved | Video BIOS
| `0x000c8000` | `0x000effff` | 160 KiB | Reserved | BIOS Expansion
| `0x000f0000` | `0x000fffff` | 64 KiB | Reserved | ROM BIOS

# Stage 2

## Physical Memory Map
The memory map for Stage 2 slightly differs from the first one because now we are able to detect the available system memory. The memory used by Stage 1 (type Stage 1) can be used to whatever we want to.

| Start | End | Size | Type | Description |
|:-----:|:---:|:----:|------|-------------|
| `0x00000000` | `0x000003ff` | 1 KiB | Reserved | Interrupt Vector Table
| `0x00000400` | `0x000004ff` | 256 B | Reserved | BIOS Data Area
| `0x00000500` | `0x00007bff` | 30464 B | Free | Usable Memory
| `0x00007c00` | `0x00007fff` | 1 KiB | Stage 2 | Stack
| `0x00008000` | `0x00008fff` | 4 KiB | Stage 2 | Stage 2
| `0x00009000` | `0x0000ffff` | 28 KiB | Free | Usable Memory
| `0x00010000` | `0x0007ffff` | 448 KiB | Stage 2 | Kernel Image
| `0x00080000` | `0x0009ffff` | 128 KiB | Reserved | Extended BIOS Data Area (mostly)
| `0x000a0000` | `0x000bffff` | 128 KiB | Reserved | Video Memory
| `0x000c0000` | `0x000c7fff` | 32 KiB | Reserved | Video BIOS
| `0x000c8000` | `0x000effff` | 160 KiB | Reserved | BIOS Expansion
| `0x000f0000` | `0x000fffff` | 64 KiB | Reserved | ROM BIOS
| `0x00100000` | `0x004fffff` | 4 MiB | Kernel | Kernel
| `0x00500000` | `0x00efffff` | 10 MiB | Free | Usable Memory
| `0x00f00000` | `0x00ffffff` | 1 MiB | ISA Memory Hole (possibly memory mapped hardware)
| `0x01000000` | `0x01ffffff` | 16 MiB | Free & MMIO | Usable Memory & Memory Mapped Hardware
| `0x02000000` | `0xffffffff` | 4064 MiB | Free & MMIO (Extra) | Usable Memory & Memory Mapped Hardware (Extra Memory)

# Kernel

## Physical Memory Map
The memory map for Stage 2 slightly differs from the first one because now we are able to detect the available system memory. The memory used by Stage 1 (type Stage 1) can be used to whatever we want to.

| Start | End | Size | Type | Description |
|:-----:|:---:|:----:|------|-------------|
| `0x00000000` | `0x000003ff` | 1 KiB | Reserved | Interrupt Vector Table
| `0x00000400` | `0x000004ff` | 256 B | Reserved | BIOS Data Area
| `0x00000500` | `0x0007ffff` | 523008 B | Free | Usable Memory
| `0x00080000` | `0x0009ffff` | 128 KiB | Reserved | Extended BIOS Data Area (mostly)
| `0x000a0000` | `0x000bffff` | 128 KiB | Reserved | Video Memory
| `0x000c0000` | `0x000c7fff` | 32 KiB | Reserved | Video BIOS
| `0x000c8000` | `0x000effff` | 160 KiB | Reserved | BIOS Expansion
| `0x000f0000` | `0x000fffff` | 64 KiB | Reserved | ROM BIOS
| `0x00100000` | `0x004fffff` | 4 MiB | Kernel | Kernel
| `0x00500000` | `0x00efffff` | 10 MiB | Free | Usable Memory
| `0x00f00000` | `0x00ffffff` | 1 MiB | ISA Memory Hole (possibly memory mapped hardware)
| `0x01000000` | `0x01ffffff` | 16 MiB | Free & MMIO | Usable Memory & Memory Mapped Hardware
| `0x02000000` | `0xffffffff` | 4064 MiB | Free & MMIO (Extra) | Usable Memory & Memory Mapped Hardware (Extra Memory)

## Virtual Memory Map
| Start | End | Size | Type | Description |
|:-----:|:---:|:----:|------|-------------|
| `0x00000000` | `0x000fffff` | 1 MiB | Reserved | First MiB
| `0x80000000` | `0x80400000` | 4 MiB | Kernel | Kernel