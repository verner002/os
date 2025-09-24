/**
 * @file mount.c
 * @author verner002
 * @file 02/09/2025
*/

#include "fs/mount.h"
#include "kernel/kdev.h"
#include "kstdlib/stdio.h"
#include "kernel/task.h"

extern uint16_t root_dev;
extern TASK *current_task;


int32_t __mount_root(void) {
    switch (MAJOR(root_dev)) {
        case FLOPPY_MAJOR:

            //current_task->t_fs->t_users = 1;
            //current_task->t_fs->t_dentry = ;
            break;
    }
}