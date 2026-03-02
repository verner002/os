/**
 * @file inode.h
 * @author verner002
 * @date 20/02/2026
*/

#pragma once

#include "kernel/atomic.h"
#include "kernel/user.h"
#include "fs/super.h"
#include "hal/device.h"

#define S_IROTH     1
#define S_IWOTH     2
#define S_IXOTH     4
#define S_IRWXO     (S_IROTH | S_IWOTH | S_IXOTH)

#define S_IRGRP     8
#define S_IWGRP     16
#define S_IXGRP     32
#define S_IRWXG     (S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IRUSR     64
#define S_IWUSR     128
#define S_IXUSR     256
#define S_IRWXU     (S_IRUSR | S_IWUSR | S_IXUSR)

// these are the common file types
// so it is pretty cheap test them
// (they are bit masks)
#define S_IFREG     0x0200
#define S_IFDIR     0x0400
#define S_IFCHR     0x0800
#define S_IFBLK     0x1000

// on the other side there will be
// some extended file types but you
// will have to use comparison to
// test them (they are numbers)
//#define S_IFSOCK    0x2000,0x4000,0x6000,...


typedef short mode_t;

struct inode_ops {

};

/**
 * NOTE: in fact there are some fields that
 *  are useless (for example if the type is
 *  regular file) and you can find rendundancy
 *  in this structure (kdev is in sb, the inode
 *  itself and in the *_device structure but that
 *  is intended since it is more efficient and
 *  doesn't require to perform multiple memory
 *  accesses)
*/

struct inode {
    mode_t mode; // permissions & mode
    uid_t uid; // user id
    gid_t gid; // group id
    atomic_t refs; // reference counter

    struct dentry *child;

    uint32_t size; // size in bytes
    
    struct super_block *super_block;
    struct inode_ops *i_ops;

    kdev_t kdev;

    union {
        struct blk_device *i_blkdev;
        //struct char_device *i_chardev;
    };
};

/*struct file_ops {
    int (* open)();
    int (* read)();
    int (* lseek)();
    int (* write)();
    int (* readdir)();
    int (* release)();
};*/

struct inode *get_inode(uid_t uid, gid_t gid, mode_t mode);