# Operating System
Operating system written in x86 Assembly and C for the i486 architecture or later revisions.

## Compilation

### Configuration

### Build


## Building The Project
### Dependencies
At least these versions of listed packages are recommended:
```
gcc >= 16.1.1
ld >= 2.46.0
nasm >= 3.01
make >= 4.4.1
dd >= 9.11
mkfs.fat >= 4.2
```

### Build
To build the project (including the bootable floppy image) use:
```
$ make
```

## Stage 1

### Physical Memory Map
This memory map is related to `boot.s` which is the first code to be run after loaded by BIOS. `boot.s` is responsible for loading `setup.s` at `0x00090200` and image at `0x00010000`.

| Start | End | Size| Description |
|:-----:|:---:|:----:|-------------|
| `0x00000000` | `0x000003ff` | 1 KiB | Interrupt Vector Table
| `0x00000400` | `0x000004ff` | 256 B | BIOS Data Area
| `0x00000500` | `0x0000ffff` | ... KiB  | Free
| `0x00010000` | `0x0008ffff` | 512 KiB | Image
| `0x00090000` | `0x000901ff` | 512 B  | Boot Loader
| `0x00090200` | `0x000909ff` | 2 KiB  | Setup
| `0x00090a00` | `0x00094fff` | ??? KiB | Free
| `0x00095000` | `0x0009ffff` | ??? KiB  | Stack
| `0x000a0000` | `0x000bffff` | 128 KiB | Video Memory
| `0x000c0000` | `0x000c7fff` | 32 KiB | Video BIOS
| `0x000c8000` | `0x000effff` | 160 KiB | BIOS Expansion
| `0x000f0000` | `0x000fffff` | 64 KiB | ROM BIOS

## Stage 2

### Physical Memory Map
Now the `setup.s` takes control: Its task is to gather system information and place them into system information block starting at `0x00090000`. After that it tells CPU to enter the protected mode.

| Start | End | Size| Description |
|:-----:|:---:|:----:|-------------|
| `0x00000000` | `0x0007ffff` | 512 KiB | Image
| `0x00080000` | `0x0008ffff` | 64 KiB | Free
| `0x00090000` | `0x000901ff` | 512 B  | System Information Block
| `0x00090200` | `0x000909ff` | 2 KiB  | Setup
| `0x00090a00` | `0x00093fff` | ??? KiB | Free
| `0x00094000` | `0x00094fff` | 4 KiB | System Memory Map
| `0x00095000` | `0x0009ffff` | ??? KiB  | Stack
| `0x000a0000` | `0x000bffff` | 128 KiB | Video Memory
| `0x000c0000` | `0x000c7fff` | 32 KiB | Video BIOS
| `0x000c8000` | `0x000effff` | 160 KiB | BIOS Expansion
| `0x000f0000` | `0x000fffff` | 64 KiB | ROM BIOS

## Stage 3

### Physical Memory Map

| Start | End | Size| Description |
|:-----:|:---:|:----:|-------------|
| `0x00000000` | `0x0007ffff` | 512 KiB | Image
| `0x00080000` | `0x0008ffff` | 64 KiB | Free
| `0x00090000` | `0x000901ff` | 512 B  | System Information Block
| `0x00090200` | `0x00090fff` | 2 KiB  | System Configuration
| `0x00091000` | `0x00091fff` | 4 KiB | Page Directory
| `0x00092000` | `0x00092fff` | 4 KiB | Self-Map Page Table
| `0x00093000` | `0x00093fff` | 4 KiB | Kernel Page Table
| `0x00094000` | `0x00094fff` | 4 KiB | System Memory Map
| `0x00095000` | `0x0009ffff` | ??? KiB  | Stack
| `0x000a0000` | `0x000bffff` | 128 KiB | Video Memory
| `0x000c0000` | `0x000c7fff` | 32 KiB | Video BIOS
| `0x000c8000` | `0x000effff` | 160 KiB | BIOS Expansion
| `0x000f0000` | `0x000fffff` | 64 KiB | ROM BIOS
| `0x00100000` | `0x004fffff` | 4 MiB | Kernel (possibly memory mapped hardware!!!)
| `0x00500000` | `0x00efffff` | 10 MiB | Free (possibly memory mapped hardware)
| `0x00f00000` | `0x00ffffff` | 1 MiB | ISA Memory Hole (possibly memory mapped hardware)
| `0x01000000` | `0x01ffffff` | 16 MiB | Usable Memory & Memory Mapped Hardware
| `0x02000000` | `0xffffffff` | 4064 MiB | Usable Memory & Memory Mapped Hardware (Extra Memory)

### Virtual Memory Map

# Kernel

## Physical Memory Map

## Virtual Memory Map

## Atomic Types & Operations
- `atomic_t` is basically an integer type:
```c
typedef int atomic_t;
```
- What actually differs is a set of operations used to increment or decrement value of an `atomic_t` variable. This is because the compiler can sometimes try to optimize our code which is not always what we want it to do.
- This means these operations are platform dependent and must be defined for each architecture.
- For x86 the increment and decrement operations are defined:
```c
static inline void atomic_increment(atomic_t *ptr) { ... }
static inline void atomic_decrement(atomic_t *ptr) { ... }
static inline int atomic_fetch_add(atomic_t *ptr, int value) { ... }
static inline int atomic_xchg(atomic_t *ptr, int value) { ... }
```

# Boot Configuration
- Boot configuration is a way to tell kernel which device it should use a root device, to set environment variables and many other stuff.

## Root Configuration
- The kernel is able to mount whatever drive you want it to as a root file system. The easiest way to do so is to set the `root` property in boot configuration:
```
root=/dev/hda
```
- Here is the list of supported root devices in the current kernel version:

| Name | Path | Description |
|:-----|:-----|:------------|
| `fd0` | `/dev/fd0` | First floppy disk drive |
| `hda` | `/dev/hda` | First hard disk drive |
| `hdb` | `/dev/hdb` | Second hard disk drive |
| `hdc` | `/dev/hda` | Third hard disk drive |
| `hdd` | `/dev/hda` | Fourth hard disk drive |

# Environment Variables Configuration
- Not implemented yet. :\-\(

# Virtual File System
- Virtual file system (VFS) provides an abstraction for all file systems which could be present on a physical media.
- File system is represented by a graph structure consisting of `dentry`s and `inode`s.
- A `dentry` maps the structure (how are all the entries connected), `inode`, on the other hand, contains metadata describing the associated `dentry` (what the entry represents).

# Device & Driver Model

# Task Address Space
- Task address space (TAS for short) is always associated, as it name suggests, with a task. This means that structures describing the TAS don't have to contain a pointer to page directory but only a pointer to a task.
- When creating a TAS we're actually creating a page directory and an AVL tree (root node) that tells us which areas of virtual memory are used.

When creating a new task, kernel allocates (mapped) virtual memory for task's page directory and creates a new AVL tree with kernel area and dynamically mapped area.

## Useful Links
### Design
[1] https://download.oldlinux.org/ECLK-5.0-WithCover.pdf

### x86 Instruction Set
[1] https://www.felixcloutier.com/x86

### PIC Driver
[1] https://wiki.osdev.org/8259_PIC\
[2] https://wiki.osdev.org/Interrupts

### PIT Driver

### PS/2 Keyboard Driver
[1] https://wiki.osdev.org/PS/2_Keyboard

### Speaker Driver

### ISA DMA Driver
[1] https://wiki.osdev.org/ISA_DMA

### VGA Driver

### VESA Driver

### ACPI Driver
[1] https://uefi.org/sites/default/files/resources/ACPI_1.pdf (ACPI v1.0)\
[2] https://uefi.org/specs/ACPI/6.5/05_ACPI_Software_Programming_Model.html (ACPI v6.5)

### FDD Driver

### FAT12 Driver
[1] https://grumbeer.dyndns.org/ftp/linux/dist/cnix/FAT.pdf (FAT12)\
[2] https://en.wikipedia.org/wiki/8.3_filename (8.3 File Name)\
[3] https://en.wikipedia.org/wiki/Long_filename (Long File Name)\
[4] http://www.maverick-os.dk/FileSystemFormats/VFAT_LongFileNames.html (Long File Name)

### Multitasking
[1] https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial\
[2] http://www.brokenthorn.com/Resources/OSDev25.html\
[3] https://wiki.osdev.org/Kernel_Multitasking\
[4] https://wiki.osdev.org/Getting_to_Ring_3 (User Space)\
[5] https://wiki.osdev.org/Context_Switching

### PCI
[1] https://wiki.osdev.org/PCI

### IDE Driver
[1] https://wiki.osdev.org/PCI_IDE_Controller

### Network Driver
[1] https://courses.cs.washington.edu/courses/cse451/16au/readings/e1000.pdf (Intel E1000)\
[2] https://wiki.osdev.org/Address_Resolution_Protocol (ARP)\
[3] https://en.wikipedia.org/wiki/Address_Resolution_Protocol (ARP)\
[4] https://wiki.osdev.org/User:KemyLand (IPv4)\
[5] https://datatracker.ietf.org/doc/html/rfc792 (ICMP)\
[6] https://wiki.osdev.org/Internet_Control_Message_Protocol (ICMP)\
[7] https://www.ietf.org/rfc/rfc1071.txt (Internet Checksum)\
[8] https://wiki.osdev.org/Intel_Ethernet_i217 (Intel E1000)\
[9] https://qiita.com/kagurazakakotori/items/a1bf45a9c2dea204f66f (Intel E1000)