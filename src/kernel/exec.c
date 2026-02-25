/**
 * @file exec.c
 * @author verner002
 * @date 04/11/2025
*/

#include "kernel/exec.h"
#include "kernel/peldr.h"
#include "kernel/task.h"

int32_t exec(char const *path, char const **args) {
    // TODO: load image from drive
    /*uint8_t *image;
    char const *name;
    
    int32_t (*entry)(int argc, char **argv) = __parse_image(image);

    // TODO: check pe last error

    return __create_thread(name, entry, THREAD_RING_0, THREAD_PRIORITY_LOW, NULL);*/
    return -1;
}