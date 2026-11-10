/**
 * @file list.h
 * @author verner002
 * @date 15/09/2026
*/

#pragma once

#define containerof(ptr, type, member) ((type *)((char *)/*(typeof(((type *)0)->member) *)*/ptr - (char *)&((type *)0)->member))

struct list {
    struct list *previous;
    struct list *next;
};