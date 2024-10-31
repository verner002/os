
# Extent B+Tree File System
- uses little-endian.
- is based on modified B+Tree structure.
- reserves 1024 bytes for boot loader.
- reserved 1024 bytes for EBTFS header.

## Layout
```
    1024 bytes     1024 bytes
 ------------- -------------- ------------------------------
| Boot Loader | EBTFS Header | Block Group Descriptor Table |
 ------------- -------------- ------------------------------
 0             1024
```

## Block
- is a group of sectors (typically 1 KiB to 8 KiB).
- contains the data of the file/directory/etc.
- blocks are numbered from 0.
- first (logical) block in the file system doesn't have to be the first physical block (drive can be separated into partitions).

## Block Group
- is a group of blocks.

## INode
- represents a file, directory, etc.
- doesn't contain the data, only metadata.
- links to blocks that actually contain the data.

## Boot Loader
- first 1024 bytes are reserved for a boot loader.

## EBTFS Header
| Offset | Size (in bytes) | Description |
|:------:|:---------------:|:------------|
| 0 | 4 | Magic number (`0x000EB7F5`) |
| 4 | 1 | File system state |
| 5 | 1 | What to do when there is an error |
| 6 | 1 | log<sub>2</sub>(block size) - 10 |
| 7 | 4 | Starting block number (not necessary 0) |
| 11 | 4 | Total number of blocks |
| 15 | 4 | Total number of inodes |
| 19 | 4 | Total number of free blocks |
| 23 | 4 | Total number of free inodes |
| 27 | 4 | Number of blocks in each block group |
| 31 | 4 | Number of inodes in each block group |

### Magic Number
- helps identify the file system

### File System State
| Number | Description |
|:------:|:------------|
| 0 | File system is clean |
| 1 | File system has some errors |

### What To Do When There Is An Error
| Number | Description |
|:------:|:------------|
| 0 | Ignore the error |
| 1 | Remount file system as read-only |
| 255 | Kernel panic |

### Block Size
- is represented in a form of a base 2 logarithm.
- can be interpreted as a number to shift 1024 to the left by to obtain the block size.

### Starting Block Number
- tells us the physical (block) address of the first logical block (EBTFS header) in the file system.

- for block size = 1024:
```
    1024 bytes     1024 bytes 1024 bytes
 ------------- -------------- ----------
| Boot Loader | EBTFS Header | ...      |
 ------------- -------------- ----------
 block n+0     block n+1      block n+2
```

- for block size = 2048:
```
    1024 bytes     1024 bytes 1024 bytes
 ------------- -------------- ----------
| Boot Loader | EBTFS Header | ...      |
 ------------- -------------- ----------
 block n+0                    block n+1
```

## Block Group Descriptor Table
- **[verner002: actually i can put bgdt immediately after header and align only the block group 0 so that header and bgdt are not part of block group]**
- for block size = 1024 bytes, the BGDT will begin at logical block 2.
- for any other block size, it will begin at logical block 1.
- the number of entries can be obtained by rounding up the total number of blocks divided by the number of blocks in block group.
- the number of sectors can be obtained by rouding up the number of entries times 32 divided by the number of bytes per sector.
- the number of block can be obtained by roungin up the number of sectors divided by the block size.

### Block Group Descriptor
| Offset | Size (in bytes) | Description |
|:------:|:---------------:|:------------|
| 0 | 4 | Block address of block usage bitmap |
| 4 | 4 | Block address of inode usage bitmap |
| 8 | 4 | Starting block address of inode table |
| 12 | 