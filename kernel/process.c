/**
 * @file process.c
 * @author verner002
 * @date 22/08/2026
*/

#include <string.h>
#include <errno.h>

#include <kernel/process.h>
#include <kernel/ctxsw.h>
#include <kernel/cpu.h>
#include <kernel/tss.h>
#include <kernel/atomic.h>
#include <kernel/system.h>
#include <kernel/pfa.h>
#include <kernel/paging.h>
#include <kernel/inode.h>

#define HEAP_TOP            0
#define HEAP_PARENT(i)      (((i) - 1) / 2)
#define HEAP_LEFT(i)        (2 * (i) + 1)
#define HEAP_RIGHT(i)       (2 * (i) + 2)

static atomic_t defers = 0;

static size_t sheap = 0;
static pid_t heap[NPROC];
static size_t lookup[NPROC];

extern volatile uint32_t ticks;
extern void syscall_return(void);

static void heapify(pid_t index) {
    pid_t parent_i = index;
    pid_t left_i = HEAP_LEFT(index);
    pid_t right_i = HEAP_RIGHT(index);

    if ((size_t)left_i < sheap && proctab[heap[left_i]].priority > proctab[heap[parent_i]].priority)
        parent_i = left_i;

    if ((size_t)right_i < sheap && proctab[heap[right_i]].priority > proctab[heap[parent_i]].priority)
        parent_i = right_i;

    if (parent_i == index)
        return;

    pid_t temp = heap[index];
    heap[index] = heap[parent_i];
    heap[parent_i] = temp;

    size_t lookup_temp = lookup[heap[index]];
    lookup[heap[index]] = lookup[temp];
    lookup[temp] = lookup_temp;

    heapify(parent_i);
}

static void sift(pid_t index) {
    size_t parent_i = HEAP_PARENT(index);

    while (index > 0 && proctab[heap[index]].priority > proctab[heap[parent_i]].priority) {
        pid_t temp = heap[index];
        heap[index] = heap[parent_i];
        heap[parent_i] = temp;

        size_t lookup_temp = lookup[heap[index]];
        lookup[heap[index]] = lookup[temp];
        lookup[temp] = lookup_temp;

        index = parent_i;
        parent_i = HEAP_PARENT(index);
    }
}

static pid_t top(void) {
    if (sheap <= 0)
        return -1;

    return heap[HEAP_TOP];
}

static pid_t pop(void) {
    if (sheap <= 0)
        return -1;

    pid_t temp = top();
    heap[HEAP_TOP] = heap[--sheap];

    lookup[top()] = HEAP_TOP;

    if (sheap > 1)
        heapify(0);
    
    return temp;
}

static int remove(pid_t pid) {
    size_t index = lookup[pid];
    
    heap[index] = heap[--sheap];

    if (index < sheap) {
        lookup[heap[index]] = index;
        heapify(index);
        sift(index);
    }

    return 0;
}

static void push(pid_t pid) {
    size_t current_i = sheap++;

    heap[current_i] = pid;
    lookup[pid] = current_i;

    sift(current_i);
}

static pid_t new_pid(void) {
    for (pid_t i = 1; i < NPROC; ++i)
        if (proctab[i].state == PROC_FREE)
            return i;

    return (pid_t)-1;
}

static int clone_address_space(uint32_t parent_cr3, uint32_t *child_cr3) {
    // TODO: implement copy-on-write (more efficient than
    //  copying whole address space)
    uint32_t *pd_parent = (uint32_t *)((parent_cr3 & ~0xfff) + KERNEL_VIRT_BASE);
    uint32_t pd_child_phys = (uint32_t)pfa_alloc();

    if (!pd_child_phys)
        return -ENOMEM;

    uint32_t *pd_child = (uint32_t *)(pd_child_phys + KERNEL_VIRT_BASE);

    memset(pd_child, 0, 4096);

    int const kernel_pde = (KERNEL_VIRT_BASE >> 22);

    // copy user pages
    for (int i = 0; i < kernel_pde; ++i) {
        uint32_t pde = pd_parent[i];

        if (!(pde & PAGE_P))
            continue;

        uint32_t *pt_parent = (uint32_t *)((pde & ~0xfff) + KERNEL_VIRT_BASE);
        uint32_t pt_child_phys = (uint32_t)pfa_alloc();

        if (!pt_child_phys)
            goto fail;

        uint32_t *pt_child = (uint32_t *)(pt_child_phys + KERNEL_VIRT_BASE);
        
        memset(pt_child, 0, 4096);
        
        for (int j = 0; j < 1024; ++j) {
            uint32_t pte = pt_parent[j];

            if (!(pte & PAGE_P))
                continue;

            uint32_t pg_child_phys = (uint32_t)pfa_alloc();

            if (!pg_child_phys)
                goto fail;

            memcpy((void *)(pg_child_phys + KERNEL_VIRT_BASE), (void *)((pte & ~0xfff) + KERNEL_VIRT_BASE), PAGE_SIZE);

            pt_child[j] = pg_child_phys | (pte & 0xfff);
        }

        pd_child[i] = pt_child_phys | (pde & 0xfff);
    }

    // share kernel pages
    for (int i = kernel_pde; i < 1024; ++i)
        pd_child[i] = pd_parent[i];

    *child_cr3 = pd_child_phys | (proctab[cproc].cr3 & 0xfff);
    return 0;

fail:
    for (size_t i = 0; i < 768; ++i) {
        if (!(pd_child[i] & PAGE_P))
            continue;
        
        uint32_t pt_phys = pd_child[i] & ~0xfffu;
        uint32_t *pt = (uint32_t *)(pt_phys + KERNEL_VIRT_BASE);

        for (size_t j = 0; j < 1024; ++j) {
            if (pt[j] & PAGE_P)
                pfa_dealloc((void *)(pt[j] & ~0xfffu));
        }

        pfa_dealloc((void *)pt_phys);
    }

    pfa_dealloc((void *)pd_child);
    return -ENOMEM;
}

pid_t sys_fork(struct regs *regs) {
    struct process *parent = current;
    
    pid_t pid = new_pid();
    struct process *child = proctab + pid;

    uint32_t child_kstack_base_phys = (uint32_t)pfa_alloc();

    if (!child_kstack_base_phys)
        return -ENOMEM;

    uint32_t child_kstack_base = child_kstack_base_phys + KERNEL_VIRT_BASE; // TODO: alloc_stack

    uint32_t child_kstack_top = child_kstack_base + 4096;

    struct regs *child_regs = (struct regs *)(child_kstack_top - sizeof(struct regs));
    memcpy((void *)child_regs, (void *)regs, sizeof(struct regs));
    child_regs->eax = 0; // fork return value

    uint32_t *child_esp = (uint32_t *)child_regs;
    *--child_esp = (uint32_t)&syscall_return;
    
    *--child_esp = 0; // ebp

    *--child_esp = 0x00000002; // eflags (if=0)

    // general purpose registers
    for (int i = 0; i < 8; ++i)
        *--child_esp = 0;

    child->pid = pid;
    child->priority = 1; // init should have priority 1
    child->s_time = child->u_time = 0;
    child->cwd = parent->cwd;

    if (clone_address_space(parent->cr3, &child->cr3)) {
        pfa_dealloc((void *)child_kstack_base_phys);
        return -ENOMEM;
    }

    child->ustack_base = parent->ustack_base;
    child->ustack_size = parent->ustack_size;
    child->kstack_base = (unsigned long)child_kstack_base;
    child->kstack_ptr = (unsigned long)child_esp;
    child->kstack_size = 4096;
    child->parent_pid = parent->pid;
    list_init(&(child->l_wait));

    for (size_t i = 0; i < NFDESC; ++i) {
        struct file *fdesc = parent->fdescs[i];

        if (fdesc)
            ++(fdesc->count);

        child->fdescs[i] = fdesc;
    }

    child->state = PROC_READY;
    push(pid);
    ++nproc;
    return pid;
}

int sys_pause(void) {
    asm volatile (
        "sti\n\t"
        "hlt"
    );

    return 0;
}

pid_t getpid(void) {
    return cproc;
}

void delay_ms(uint32_t ms) {
    uint32_t ticks_end = ticks + ms;

    while (ticks < ticks_end);
}

int ready(pid_t pid) {
    if (ISBADPID(pid) || !pid)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    // TODO: check process state
    struct process *proc = proctab + pid;
    proc->state = PROC_READY;
    push(pid);

    restore_flags(flags);
    return 0;
}

int wait(void) {
    unsigned long flags;
    save_flags(flags);
    cli();

    current->state = PROC_WAITING;
    schedule();

    restore_flags(flags);
    return 0;
}

int suspend(pid_t pid) {
    if (ISBADPID(pid) || !pid)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    struct process *proc = proctab + pid;

    if (proc->state == PROC_RUNNING) {
        proc->state = PROC_SUSPENDED;
        schedule();
    } else if (proc->state == PROC_READY) {
        remove(pid);
        proc->state = PROC_SUSPENDED;
    } else {
        restore_flags(flags);
        return -EINVAL;
    }

    restore_flags(flags);
    return 0;
}

int resume(pid_t pid) {
    if (ISBADPID(pid))
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    struct process *proc = proctab + pid;

    if (proc->state != PROC_SUSPENDED) {
        restore_flags(flags);
        return -EINVAL;
    }

    proc->state = PROC_READY;

    push(pid);

    restore_flags(flags);
    return 0;
}

int kill(pid_t pid) {
    unsigned long flags;

    if (ISBADPID(pid) || !pid)
        return -EINVAL;

    save_flags(flags);
    cli();

    struct process *proc = proctab + pid;

    // TODO:
    //  free heap
    //  free stack
    //  send signal to parent
    //  close devices

    switch (proc->state) {
        case PROC_RUNNING:
            proc->state = PROC_FREE;
            schedule();
            __builtin_unreachable();

        case PROC_READY:
            remove(pid);
            __attribute__((fallthrough));
        
        default:
            proc->state = PROC_FREE;
            break;
    }

    restore_flags(flags);
    return 0;
}

void enable_preemtion(void) {
    atomic_increment(&defers);
}

void disable_preemtion(void) {
    atomic_decrement(&defers);
}

void schedule(void) {
    if (defers > 0)
        return;

    struct process *old_proc = proctab + cproc;

    // current process is still running
    if (old_proc->state == PROC_RUNNING) {
        if (!sheap)
            return;

        // currently running process has the highest priority
        if (old_proc->priority > proctab[top()].priority)
            return;

        old_proc->state = PROC_READY;
        push(cproc);
    }

    // pop process with higher or equal priority
    cproc = pop();
    current = proctab + cproc;

    struct process *new_proc = proctab + cproc;
    new_proc->state = PROC_RUNNING;

    tss_set_esp0(new_proc->kstack_base + new_proc->kstack_size);

    // perform the context switch
    ctxsw(&old_proc->kstack_ptr, &new_proc->kstack_ptr, new_proc->cr3);
}