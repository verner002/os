/**
 * @file queue.h
 * @author verner002
 * @date 15/09/2026
*/

#pragma once

typedef int qid_t;

#define QID_NONE    -1
#define QID_MAX     0x7fffffff
#define QID_MIN     0x80000000

#define ISBADQID(x) ((x))

struct queue {
    qid_t qid; // queue id
    
    qid_t next;
    qid_t previous;
};

extern struct queue quetab[];