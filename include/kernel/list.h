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

struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

static inline void list_init(struct list_head *head) {
    head->prev = head;
    head->next = head;
}

static inline int list_empty(struct list_head *head) {
    return head->next == head;
}

static inline void list_add(struct list_head *item, struct list_head *prev, struct list_head *next) {
    prev->next = item;
    item->prev = prev;
    item->next = next;
    next->prev = item;
}

static inline void list_head_add(struct list_head *head, struct list_head *item) {
    list_add(item, head, head->next);
}

static inline void list_tail_add(struct list_head *head, struct list_head *item) {
    list_add(item, head->prev, head);
}

static inline void list_link(struct list_head *prev, struct list_head *next) {
    prev->next = next;
    next->prev = prev;
}

static inline void list_move(struct list_head *head, struct list_head *item) {
    list_link(item->prev, item->next);
    list_head_add(head, item);
}

//static inline void list_remove_last()