/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
*/

#include "null.h"
#include "bool.h"
#include "kernel/tty.h"
#include "mm/heap.h"
#include "kstdlib/stdio.h"
#include "kernel/task.h"

struct __tty_msg {
    uint32_t t_priority;
    char const *t_data;
    struct __tty_msg *next;
};

bool __tty0_mutex = FALSE;

struct __tty_msg
    *head = NULL,
    *tail = NULL;

int32_t __tty0(int argc, char **argv) {
    while (1) {
        if (head) {
            __mutex_lock(&__tty0_mutex);

            do {
                printf(head->t_data);

                struct __tty_msg *temp = head->next;
                kfree(head);
                head = temp;
            } while (head);

            __mutex_unlock(&__tty0_mutex);
        }
    }
}

int32_t __tty_init(void) {
    int32_t pid = __create_thread("tty0", &__tty0, THREAD_RING_0);

    if (pid == -1) {
        printk("failed to start tty0\n");
        return -1;
    }

    return 0;
}

int32_t __tty_put(char const *data) {
    struct __tty_msg *msg = (struct __tty_msg *)kmalloc(sizeof(struct __tty_msg));

    if (!msg)
        return -1;

    msg->t_priority = 0;
    msg->t_data = data;
    msg->next = NULL;

    if (tail)
        tail->next = msg;

    tail = msg;

    if (!head)
        head = msg;

    return 0;
}