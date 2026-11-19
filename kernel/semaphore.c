/**
 * @file semaphore.c
 * @author verner002
 * @date 18/11/2026
*/

#include <errno.h>

#include <kernel/semaphore.h>
#include <kernel/process.h>
#include <kernel/cpu.h>

#define SEM_VALUE_MAX           ((1u << 31) - 1)

int sem_init(sem_t *sem, unsigned int value) {
    if (!sem || value > SEM_VALUE_MAX)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    sem->value = value;
    list_init(&(sem->l_wait));

    restore_flags(flags);
    return 0;
}

int sem_wait(sem_t *sem) {
    if (!sem)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    if (--sem->value < 0) {
        list_add(&(sem->l_wait), &(current->l_wait));
        wait(); // PROC_WAITING and reschedule
    }

    restore_flags(flags);
    return 0;
}

int sem_trywait(sem_t *sem) {
    if (!sem)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    if (sem->value <= 0) {
        restore_flags(flags);
        return -EAGAIN;
    }

    --sem->value;

    restore_flags(flags);
    return 0;
}

int sem_signal(sem_t *sem) {
    if (!sem)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    if (sem->value == SEM_VALUE_MAX) {
        restore_flags(flags);
        return -EOVERFLOW;
    }

    if (sem->value++ < 0) {
        if (!list_empty(&(sem->l_wait))) {
            struct process *process = containerof(
                list_pop(&(sem->l_wait)),
                struct process,
                l_wait
            );
            
            ready(process->pid); // PROC_READY
        }
    }

    restore_flags(flags);
    return 0;
}

int sem_read(sem_t *sem, int *value) {
    if (!sem || !value)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    *value = sem->value;

    restore_flags(flags);
    return 0;
}