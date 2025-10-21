/**
 * @file vfs.h
 * @author verner002
 * @date 17/08/2025
*/

#pragma once

#define VFS_NODE_TYPE_FILE 0
#define VFS_NODE_TYPE_DIR 1

struct __file_io_ops {
    int32_t (* __open)(struct __inode *inode, uint8_t mode);
    int32_t (* __seek)(struct __inode *inode, uint32_t offset);
    int32_t (* __read)(struct __inode *inode, char *buffer, uint32_t count);
    int32_t (* __write)(struct __inode *inode, char const *buffer, uint32_t count, uint32_t offset);
    int32_t (* __close)(struct __inode *inode);
};

struct __dir_io_ops {
    int32_t (* __list)(struct __inode *inode);
};

/**
 * __inode (index node)
 * inode is a metadata struct for each __dentry
 * that contains flags, owner and group ids,
 * attributes specifing allowed operations,
 * i/o operations block for inode and for
 * data specified by file system driver and
 * dentry reference
*/

struct __inode {
    uint32_t i_mode; // permissions and modes
    struct __dentry *i_dentry; // directory entry
    uint32_t i_uid; // owner id
    uint32_t i_gid; // group id
    uint32_t i_refs; // reference counter
};

struct __dentry_io_block {
    struct __dentry *(* create)(struct __dentry *entry);
    struct __dentry *(* lookup)(struct __dentry *entry, char const *name);
};

/**
 * __dentry (directory entry)
 * 
 * represents a file or directory entry
 * containing entry name (can resolve paths),
 * inode reference and parent node reference
 * alongside with double linked-list referencing
 * previous and next node
*/

struct __dentry {
    char const *name;
    struct __dentry_io_block io_ops;
    struct __inode *d_inode;
    struct __dentry *d_parent;
    uint32_t d_refs;
    struct __dentry *d_prev;
    struct __dentry *d_next;
    struct __dentry *d_child;
};

void __dentry_init(struct __dentry *dentry);
void __dentry_add(struct __dentry *dentry, struct __dentry *parent);
void __inode_init(struct __inode *inode, struct __dentry *dentry);
struct __dentry *__lookup(struct __dentry *node, char const *path, uint32_t max_depth);

/*VFS_DIR_NODE *__init_vfs(DRIVER *root_dev_driver);
VFS_FILE_NODE *__new_vfs_file_node(char const *name);
VFS_DIR_NODE *__new_vfs_dir_node(char const *name);
int32_t __update_vfs_node(VFS_NODE *node, uint8_t flags);
VFS_NODE *__find_vfs_node(VFS_DIR_NODE *root, char const *name);*/

struct __dentry *__file_add(struct __dentry *parent, char const *name, uint32_t uid, uint32_t gid, uint32_t flags);