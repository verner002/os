/**
 * @file vfs.c
 * @author verner002
 * @date 17/08/2025
*/

#include "types.h"
#include "mm/heap.h"
#include "hal/vfs.h"
#include "hal/driver.h"
#include "kernel/kdev.h"
#include "kstdlib/stdio.h"

void __dentry_init(struct __dentry *dentry) {
    if (!dentry)
        return;

    dentry->name = NULL;
    dentry->d_refs = 1;
    dentry->d_parent = NULL;
    dentry->d_prev = NULL;
    dentry->d_next = NULL;
    dentry->d_inode = NULL;
    dentry->d_child = NULL;
}

void __dentry_add(struct __dentry *dentry, struct __dentry *parent) {
    if (!dentry || !parent)
        return;

    // add list head
    dentry->d_parent = parent;
    dentry->d_next = parent->d_child;
    dentry->io_ops = parent->io_ops;

    if (parent->d_child)
        parent->d_child->d_prev = dentry;

    parent->d_child = dentry;
}

void __inode_init(struct __inode *inode, struct __dentry *dentry) {
    if (!inode || !dentry)
        return;

    inode->i_dentry = dentry;
    inode->i_mode = 0;
    inode->i_refs = 1;
    inode->i_uid = 0777;
    inode->i_gid = 0777;
}

static struct __dentry *__internal_lookup(struct __dentry *node, char **saveptr, char *path, uint32_t max_depth, uint32_t depth) {
    printk("looking for [%s] in [%s]\n", path, node->name);
    
    if (depth > max_depth)
        return NULL;

    if (strcmp(node->name, path))
        return NULL;

    char *subpath = strtok_r(NULL, "/", saveptr);

    printk("sp=%p\n", subpath);

    if (!subpath)
        return node;

    struct __dentry *child = node->d_child;

    while (child) {
        struct __dentry *temp = __internal_lookup(child, saveptr, subpath, max_depth, depth + 1);

        if (temp)
            return temp;

        child = child->d_next;
    }
}

struct __dentry *__lookup(struct __dentry *node, char const *path, uint32_t max_depth) {
    struct __dentry *child = node->d_child;

    char *strtok_buffer;

    while (child) {
        struct __dentry *temp = __internal_lookup(child, &strtok_buffer, strtok_r(path, "/", &strtok_buffer), max_depth, 1);

        if (temp)
            return temp;

        child = child->d_next;
    }

    return NULL;
}

/*VFS_DIR_NODE *__init_vfs(DRIVER *root_dev_driver) {
    root = kmalloc(sizeof(VFS_DIR_NODE));

    if (!root)
        return NULL;

    root->h.name = "/";
    root->h.type = VFS_NODE_TYPE_DIR;
    
    root->h.attributes.directory = 1;
    root->h.attributes.owner_read = 1;
    root->h.attributes.owner_write = 1;
    root->h.attributes.owner_execute = 1;
    root->h.attributes.group_read = 1;
    root->h.attributes.group_write = 0;
    root->h.attributes.group_execute = 1;
    root->h.attributes.others_read = 1;
    root->h.attributes.others_write = 0;
    root->h.attributes.others_execute = 1;

    root->h.owner_id = 0;
    root->h.group_id = 0;
    root->h.size = 0;

    root->io_func.__create = (void *)__link_symbol(root_dev_driver, "__create");
    root->io_func.__list = (void *)__link_symbol(root_dev_driver, "__list");
    root->io_func.__delete = (void *)__link_symbol(root_dev_driver, "__delete");

    root->nodes_count = 0;
    root->nodes = NULL;

    return root;
}

VFS_FILE_NODE *__new_vfs_file_node(char const *name) {
    return NULL;
}

VFS_DIR_NODE *__new_vfs_dir_node(char const *name) {
    VFS_DIR_NODE *dir = kmalloc(sizeof(VFS_DIR_NODE));
    
    if (!dir)
        return NULL;

    dir->h.name = name;
    dir->h.type = VFS_NODE_TYPE_DIR;
    dir->h.parent = NULL;

    dir->io_func.__create = NULL;
    dir->io_func.__list = NULL;
    dir->io_func.__delete = NULL;

    dir->h.attributes.directory = 1;
    dir->h.attributes.owner_read = 1;
    dir->h.attributes.owner_write = 1;
    dir->h.attributes.owner_execute = 1;
    dir->h.attributes.group_read = 1;
    dir->h.attributes.group_write = 0;
    dir->h.attributes.group_execute = 1;
    dir->h.attributes.others_read = 1;
    dir->h.attributes.others_write = 0;
    dir->h.attributes.others_execute = 1;

    dir->h.owner_id = 0;
    dir->h.group_id = 0;
    dir->h.size = 0;
    
    dir->nodes_count = 0;
    dir->nodes = NULL;

    return dir;
}*/

/**
 * __update_vfs_node
 * 
 * NOTE: updates the given node
 *  used when a node is entered
 *  (data from device are fetched)
 * 
 * TODO: use flags to specify
 *  update type? (VISIT_SUBNODES,
 *  etc.)
*/

/*int32_t __update_vfs_node(VFS_NODE *node, uint8_t flags) {
    if (!node)
        return -1;

    switch (node->type) {
        case VFS_NODE_TYPE_FILE:
            // read file
            return -1;

        case VFS_NODE_TYPE_DIR:
            VFS_DIR_NODE *dir = (VFS_DIR_NODE *)node;
            return dir->io_func.__list((VFS_NODE *)dir);
    }
    
    return -1;
}*/

/**
 * __find_vfs_node
 * 
 * RFC: add `recursive` argument for recursive search?
 */

/*VFS_NODE *__find_vfs_node(VFS_DIR_NODE *root, char const *name) {
    if (root->h.type != VFS_NODE_TYPE_DIR)
        return NULL;

    // callee is responsible for memory allocation,
    // caching, etc.
    root->io_func.__list((VFS_NODE *)root);

    uint32_t subnodes_count = root->nodes_count;
    VFS_NODE **subnodes = root->nodes;

    for (uint32_t i = 0; i < subnodes_count; ++i) {
        VFS_NODE *node = subnodes[i];

        if (!strcmp(node->name, name))
            return node;
        else if (node->type == VFS_NODE_TYPE_DIR) {
            // RFC: is recursion a good idea for a bigger filesystems???
            // it'd be better to use breadth-first search
            node = __find_vfs_node((VFS_DIR_NODE *)node, name);

            if (node)
                return node;
        }
    }

    return NULL; // not found
}*/

struct __dentry *__file_add(struct __dentry *parent, char const *name, uint32_t uid, uint32_t gid, uint32_t flags) {
    if (!parent || !name)
        return NULL;
    
    struct __dentry *file_d = (struct __dentry *)kmalloc(sizeof(struct __dentry));

    if (!file_d)
        return NULL;

    struct __inode *file_i = (struct __inode *)kmalloc(sizeof(struct __inode));

    if (!file_i) {
        kfree(file_d);
        return NULL;
    }

    __inode_init(file_i, file_d);
    file_i->i_uid = uid;
    file_i->i_gid = gid;
    file_i->i_mode = flags;

    __dentry_init(file_d);
    file_d->name = name;
    file_d->d_inode = file_i;
    __dentry_add(file_d, parent);
    
    return file_d;
}