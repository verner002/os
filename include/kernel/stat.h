/**
 * @file stat.h
 * @author verner002
 * @date 13/10/2026
*/

#pragma once

#include <kernel/inode.h>

struct stat {
    ino_t s_ino;
    uid_t s_uid;
    gid_t s_gid;
    mode_t s_mode;
    kdev_t s_dev;
    unsigned long s_size;
};