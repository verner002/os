/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
*/

#include "null.h"
#include "bool.h"
#include "macros.h"
#include "kernel/tty.h"
#include "mm/heap.h"
#include "kstdlib/stdio.h"
#include "kernel/task.h"
#include "kernel/task.h"
#include "drivers/graphics/graphix.h"

static uint32_t buffer_count = 0;
static FILE *buffers[16];

static int nonblocking_getc(FILE *stream) {
    if (__test_set(&stream->__lock))
        return -1;

    if (!stream->__count) {
        __mutex_unlock(&stream->__lock);
        return -1;
    }

    /*if (!stream->__count) {
        // no data available
        errno = EAGAIN; // EWOULDBLOCK
        return -1;
    }*/

    --stream->__count;

    int c = *stream->__ptr;
    stream->__ptr = (stream->__ptr - stream->__base + 1) % stream->__size + stream->__base;
    __mutex_unlock(&stream->__lock);
    return c;
}

int32_t __alloc_buffer(FILE *file, char const *name, uint32_t size) {
    if (!file)
        return -1;

    if (buffer_count >= sizeofarray(buffers))
        return -1;

    char *buffer = (char *)kmalloc(sizeof(char) * size);

    if (!buffer)
        return -1;

    // this function shall not use mutex lock
    // since it can result in yield being called
    // and the problem is that this function is
    // used inside __create_thread (could result
    // in dead lock)
    //__mutex_lock(&file->__lock);
    file->__base = buffer;
    file->__ptr = buffer;
    file->__index = 0;
    file->__count = 0;
    file->__flags = 0;
    file->__size = size;
    file->__fname = name;
    __mutex_unlock(&file->__lock);

    // TODO: increment better be atomic
    //  or we should use mutex
    buffers[buffer_count++] = file;
    return 0;
} 

int32_t __tty0(int argc, char **argv) {
    while (1)
        for (uint32_t i = 0; i < buffer_count; ++i) {
            FILE *stream = buffers[i];

            if (__test_set(&stream->__lock))
                continue;

            if (stream->__count) {
                --stream->__count;
                int c = *stream->__ptr;
                stream->__ptr = (stream->__ptr - stream->__base + 1) % stream->__size + stream->__base;
                __putc(c); //__graphix_putc(c);
            }

            __mutex_unlock(&stream->__lock);

            /*char c;

            while ((c = nonblocking_getc(buffers[i])) != -1)
                __graphix_putc(c);*/
        }
}

int32_t __tty_init(void) {
    int32_t pid = __create_thread("tty0", &__tty0, THREAD_RING_0, THREAD_PRIORITY_HIGH);

    if (pid == -1) {
        //printk("failed to start tty0\n");
        return -1;
    }

    return 0;
}