# OS
Operating system written in x86 Assembly and C for the i486 architecture or later revisions.

# Building The Project
## Dependencies
At least these versions of listed packages are required:
```
gcc >= 15.2.1
ld >= 2.45.0
nasm >= 2.16.03
make >= 4.4.1
dd >= 9.7
mkfs.fat >= 4.2
```

## Build
To build the project (including the bootable floppy image) use:
```
$ make
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

# Portability
- Even though this is just a hobby operating system I am doing my best to make it portable.

## Atomic Types & Operations
- `atomic_t` and `uatomic_t` are basically (unsigned) integer types:
```c
typedef int atomic_t;
typedef unsigned int uatomic_t;
```
- What actually differs is a set of operations used to increment or decrement value of an `atomic_t` variable. This is because the compiler can sometimes try to optimize our code which is not always what we want it to do.
- This means these operations are platform dependent and must be defined for each architecture.
- For x86 the increment and decrement operations are defined like this:
```c
#define __atomic_inc(x) asm volatile (  \
    "inc dword [%0]"                        \
    :                                       \
    : "m" (x)                               \
    :                                       \
);

#define __atomic_inc(x) asm volatile (  \
    "dec dword [%0]"                        \
    :                                       \
    : "m" (x)                               \
    :                                       \
);
```

# Boot Configuration
- Boot configuration is a way to tell kernel which device it should use a root device, to set environment variables and many other stuff.
- The pointer to the command line is passed as an arument to the `__entry` function along with other arguments like cursor position, E820 map, etc.

## Root Configuration
- The kernel is able to mount whatever drive you want it to as a root file system. The easiest way to do so is to set the `root` property in boot configuration:
```
root=/dev/hda
```
- Here is the list of supported root devices in the current kernel version:

| Name | Path | Description |
|:-----|:-----|:------------|
| fd0 | /dev/fd0 | First floppy disk drive |
| hda | /dev/hda | First hard disk drive |
| hdb | /dev/hdb | Second hard disk drive |
| hdc | /dev/hda | Third hard disk drive |
| hdd | /dev/hda | Fourth hard disk drive |

# Environment Variables Configuration
- Not implemented yet. :\-\(

# Virtual File System
- Virtual file system (VFS) provides an abstraction for all file systems which could be present on a physical media.
- File system is represented by a graph structure consisting of `__dentry`s and `__inode`s.
- A `__dentry` maps the structure (how are all the entries connected), `__inode`, on the other hand, contains metadata describing the associated `__dentry` (what the entry represents).
- The structutes look like this:
```c
struct __dentry {
    char const *name;               // entry name
    char sname[10];                 // short name
    atomic_t refs;                  // reference counter
    struct __dentry_ops ops;        // entry operations
    struct __inode *inode;          // metadata node
    struct __dentry *parent;        // parent entry
    struct __dentry *previous;      // previous entry
    struct __dentry *next;          // next entry
    struct __dentry *child_head;    // head of children linked-list
};

struct __inode {
    uint32_t mode;                  // entry mode
    atomic_t refs;                  // reference counter
    struct __dentry *dentry;        // entry node
    uint32_t uid;                   // owner id
    uint32_t gid;                   // owner group id
    uint32_t block_n;               // block number
    uint32_t block_cnt;             // block count
    uint32_t block_sz;              // block size
};
```

# Device & Driver Model
## Kernel Object
- `__kobj` is similar to the one used in GNU/Linux. On its own the kernel object is useless. To become usefull we need to attach `__kobj_type` to and assign it to a device(/bus/driver/etc.). This way we are able to create a tree-like structure which allows us to operate with each device the same way.
- The structure is defined like this:
```c
struct __kobj {
    char const *k_name;
    char k_sname[10];
    atomic_t k_refs;
    struct __kobj *k_parent;
    struct __kobj *k_previous;
    struct __kobj *k_next;
    struct __kobj_type *k_type;
    struct __dentry *k_dentry;
};
```

## Kernel Object Type

## Kernel Object Set

## Useful Links
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