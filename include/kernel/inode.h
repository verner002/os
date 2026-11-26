/**
 * @file inode.h
 * @author verner002
 * @date 20/02/2026
*/

#pragma once

#include <stddef.h>

#include <kernel/kdev.h>
#include <kernel/semaphore.h>

#define S_IROTH         (1 << 0)
#define S_IWOTH         (1 << 1)
#define S_IXOTH         (1 << 2)
#define S_IRWXO         (S_IROTH | S_IWOTH | S_IXOTH)

#define S_IRGRP         (1 << 3)
#define S_IWGRP         (1 << 4)
#define S_IXGRP         (1 << 5)
#define S_IRWXG         (S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IRUSR         (1 << 6)
#define S_IWUSR         (1 << 7)
#define S_IXUSR         (1 << 8)
#define S_IRWXU         (S_IRUSR | S_IWUSR | S_IXUSR)

// file type mask
#define S_IFMT          0xf000

// file types
#define S_IFREG         0x1000
#define S_IFDIR         0x2000
#define S_IFCHR         0x4000
#define S_IFBLK         0x8000
//#define S_IFSOCK    0x3000,0x5000,0x6000,...

#define S_ISREG(mode)   (((mode) & S_IFMT) == S_IFREG)
#define S_ISDIR(mode)   (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode)   (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode)   (((mode) & S_IFMT) == S_IFBLK)

#define FMODE_READ      1
#define FMODE_WRITE     2

typedef unsigned long ino_t;

// ttttsssuuugggooo
// t - type
// s - special/reserved
// u - user permissions
// g - group permissions
// o - others permissions
typedef unsigned short mode_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;

// forward declarations
struct inode;
struct file;
struct dentry;

struct inode_ops {
    int (* create)(struct inode *inode, struct dentry *dentry, mode_t mode);
};

struct file_ops {
    ssize_t (* open)(struct inode *inode, struct file *file);
    ssize_t (* read)(struct file *file, char *buffer, size_t count);
    ssize_t (* write)(struct file *file, char const *buffer, size_t count);
};

/**
 * NOTE: in fact there are some fields that
 *  are useless (for example if the type is
 *  regular file) and you can find rendundancy
 *  in this structure (kdev is in sb, the inode
 *  itself, etc. but that is intended since it
 *  is more efficient and doesn't require to
 *  perform multiple memory accesses)
*/

struct inode {
    ino_t ino;
    uid_t uid; // user id
    gid_t gid; // group id
    mode_t mode; // type, special/reserved and permission bits
    size_t refs; // reference counter

    sem_t rw_lock;
    sem_t turnstile;
    int readers;

    unsigned long size; // size in bytes
    
    struct super_block *sb;
    struct inode_ops *ops;
    struct file_ops const *fops;

    kdev_t dev; // where inode is
    kdev_t rdev; // what inode represents

    union {
        int blk_index;
        int char_offset;
    };

    union {
        struct blk_device *blkdev;
        //struct char_device *chardev;
    };
};

struct file {
    unsigned short mode; // FMODE_READ, FMODE_WRITE, etc.
    unsigned short flags; // O_CREAT, O_APPEND, etc.
    struct dentry *dentry;
    unsigned long offset;
    size_t count;
    struct file_ops const *fops; // same ptr as dentry->inode->fops
};

void inode_init(void);
struct inode *alloc_inode(uid_t uid, gid_t gid, mode_t mode);
struct inode *iget(struct inode *inode);
void iput(struct inode *inode);