/**
 * @file semaphore.h
 * @author verner002
 * @date 18/11/2026
*/

#pragma once

#include <kernel/list.h>

typedef struct {
    int value;
    struct list_head l_wait;
} sem_t;

int sem_init(sem_t *sem, unsigned int value);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_signal(sem_t *sem);
int sem_read(sem_t *sem, int *value);