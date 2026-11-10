%define TASK_STATE_SEGMENT  (8 * 5)

global flush_tss

flush_tss:
    mov ax, TASK_STATE_SEGMENT | 0
    ltr ax
    ret