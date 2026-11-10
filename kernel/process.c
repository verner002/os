/**
 * @file process.c
 * @author verner002
 * @date 22/08/2026
*/

#include <errno.h>

#include <kernel/process.h>
#include <kernel/cpu.h>
#include <kernel/atomic.h>
#include <kernel/ctxsw.h>
#include <kernel/printk.h>

#define HEAP_TOP            0
#define HEAP_PARENT(i)      (((i) - 1) / 2)
#define HEAP_LEFT(i)        (2 * (i) + 1)
#define HEAP_RIGHT(i)       (2 * (i) + 2)

static atomic_t defers = 0;

static size_t sheap = 0;
static pid_t heap[NPROC];
static size_t lookup[NPROC];

extern volatile uint32_t ticks;

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

void procexit(void) {
    while (1);
}

#include <string.h>
#include <kernel/system.h>
#include <kernel/pfa.h>
#include <kernel/tss.h>
#include <kernel/paging.h>

uint32_t read_eip(void);

static int clone_address_space(uint32_t parent_cr3) {
    // TODO: implement copy-on-write (more efficient than
    //  copying whole address space)
    uint32_t *pd_parent = (uint32_t *)((parent_cr3 & ~0xfff) + KERNEL_VIRT_BASE);
    uint32_t pd_child_phys = (uint32_t)pfa_alloc();
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
        uint32_t *pt_child = (uint32_t *)(pt_child_phys + KERNEL_VIRT_BASE);
        
        memset(pt_child, 0, 4096);
        
        for (int j = 0; j < 1024; ++j) {
            uint32_t pte = pt_parent[j];

            if (!(pte & PAGE_P))
                continue;

            uint32_t pg_child_phys = (uint32_t)pfa_alloc();

            memcpy((void *)(pg_child_phys + KERNEL_VIRT_BASE), (void *)((pte & ~0xfff) + KERNEL_VIRT_BASE), 4096);

            pt_child[j] = pg_child_phys | (pte & 0xfff);
        }

        pd_child[i] = pt_child_phys | (pde & 0xfff);
    }

    // share kernel pages
    for (int i = kernel_pde; i < 1024; ++i) {
        uint32_t pde = pd_parent[i];

        if (!(pde & PAGE_P))
            continue;

        pd_child[i] = pde;        
    }

    return pd_child_phys | (proctab[cproc].cr3 & 0xfff);
}

extern void syscall_return(void);

pid_t sys_fork(struct regs *regs) {
    struct process *parent = &proctab[cproc];
    
    pid_t pid = new_pid();
    struct process *child = &proctab[pid];

    uint32_t child_kstack_base = (uint32_t)(pfa_alloc() + KERNEL_VIRT_BASE); // TODO: alloc_stack

    /**
     * FIXME:
     *  pid 1 (init) is now forced to use user stack in with virtual
     *  address in kernel address space, we should solve this!!!
     */
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

    //*(uint32_t *)child_kstack_base = STACK_CANARY;

    // ctxsw switches esp to child->stack_ptr
    // but we return kernel mode (returning from
    // kernel mode), so we have to use kernel
    // stack after context switch
    child->pid = pid;
    child->state = PROC_READY;
    child->priority = 1; // init should have priority 1
    child->s_time = child->u_time = 0;
    child->cwd = parent->cwd;
    child->cr3 = clone_address_space(parent->cr3);
    child->stack_base = parent->stack_base;
    child->stack_ptr = (unsigned long)child_esp;
    child->stack_size = parent->stack_size;
    child->kstack_base = (unsigned long)child_kstack_base;
    child->kstack_ptr = (unsigned long)child_esp;
    child->kstack_size = 4096;
    child->parent_pid = parent->pid;
    child->wait_list.previous = NULL;
    child->wait_list.next = NULL;

    for (size_t i = 0; i < NFDESC; ++i)
        child->fdescs[i] = parent->fdescs[i];

    printk("ch->stack_base: %p\nch->stack_ptr: %p\nch->stack_size: %p\n", child->stack_base, child->stack_ptr, child->stack_size);
    printk("ch->kstack_base: %p\nch->kstack_ptr: %p\nch->kstack_size: %p\n", child->kstack_base, child->kstack_ptr, child->kstack_size);

    push(pid);
    ++nproc;

    /*for (int i = 0; i < nproc; ++i) {
        struct process *proc = &proctab[i];

        printk("PID=%i, \n", proc->pid);
    }*/

    return pid;
}

int sys_pause(void) {
    asm volatile (
        "sti\n\t"
        "hlt"
    );

    return 0;
}

extern void trampoline(void);

pid_t sys_create(void (*function)(void), unsigned char priority, size_t stack_size) {
    unsigned long flags;
    save_flags(flags);
    cli();

    uint32_t *saddr = (uint32_t *)(pfa_alloc() + KERNEL_VIRT_BASE); // TODO: alloc_stack
    uint32_t *ksaddr = (uint32_t *)(pfa_alloc() + KERNEL_VIRT_BASE); // TODO: alloc_stack

    pid_t pid = new_pid();

    struct process *proc = &proctab[pid];

    proc->pid = pid;
    proc->state = PROC_READY;
    proc->priority = priority;
    proc->cr3 = proctab[cproc].cr3;
    proc->stack_base = (unsigned long)saddr;
    proc->stack_size = 4096; // stack_size
    proc->kstack_base = (unsigned long)ksaddr;
    proc->kstack_size = 4096;
    proc->parent_pid = proctab[cproc].pid;
    proc->wait_list.previous = NULL;
    proc->wait_list.next = NULL;

    // stack setup
    saddr += 1024; // stack_size / sizeof(uint32_t)

    *--saddr = STACK_CANARY;

    *--saddr = (uint32_t)&procexit;

    uint32_t temp_esp = (uint32_t)saddr;

    *--saddr = (4 * 8) | 3; // userland data selector
    *--saddr = temp_esp;
    *--saddr = 0x00000202; // eflags (if=1)
    *--saddr = (3 * 8) | 3; // userland code selector
    *--saddr = (uint32_t)function;
    *--saddr = (uint32_t)&trampoline;
    
    temp_esp = (uint32_t)saddr;

    *--saddr = temp_esp; // push ebp

    uint32_t temp_ebp = (uint32_t)saddr; // corresponds to mov ebp, esp after popad

    *--saddr = 0x00000002; // eflags (if=0)

    temp_esp = (uint32_t)saddr; // pushad temp variable for esp

    *--saddr = 0; // eax
    *--saddr = 0; // ecx
    *--saddr = 0; // edx
    *--saddr = 0; // ebx
    *--saddr = temp_esp; // esp
    *--saddr = temp_ebp; // ebp
    *--saddr = 0; // esi
    *--saddr = 0; // edi

    proc->stack_ptr = (unsigned long)saddr;

    // kernel stack setup
    ksaddr += 1024; // kstack_size / sizeof(uint32_t)

    *--ksaddr = STACK_CANARY;

    proc->kstack_ptr = (unsigned long)ksaddr;

    push(pid);
    ++nproc;

    restore_flags(flags);
    return pid;
}

pid_t getpid(void) {
    return cproc;
}

void delay_ms(uint32_t ms) {
    uint32_t ticks_end = ticks + ms;

    while (ticks < ticks_end);
}

int suspend(pid_t pid) {
    unsigned long flags;

    if (ISBADPID(pid) || !pid)
        return -EINVAL;

    save_flags(flags);
    cli();

    struct process *proc = &proctab[pid];

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
    unsigned long flags;

    if (ISBADPID(pid))
        return -EINVAL;

    save_flags(flags);
    cli();

    struct process *proc = &proctab[pid];

    if (proc->state != PROC_SUSPENDED) {
        restore_flags(flags);
        return -EINVAL;
    }

    proc->state = PROC_READY;

    push(pid);

    restore_flags(flags);
    return 0;
}

int wait(struct semaphore *sem) {
    if (!sem)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    atomic_decrement(&(sem->count));

    if (sem->count < 0) {
        struct process *proc = &proctab[cproc];
        proc->state = PROC_WAITING;

        struct list *tail = sem->wait_list;

        if (tail) {
            while (tail->next)
                tail = tail->next;

            tail->next = &(proc->wait_list);
            proc->wait_list.next = NULL;
        } else
            sem->wait_list = &(proc->wait_list);

        schedule();
    }

    restore_flags(flags);
    return 0;
}

int signal(struct semaphore *sem) {
    if (!sem)
        return -EINVAL;

    unsigned long flags;
    save_flags(flags);
    cli();

    if (sem->count < 0) {
        // TODO: dequeue process id from semaphore wait list
        struct list *head = sem->wait_list;
        
        sem->wait_list = sem->wait_list->next;
        //head->next = NULL;

        struct process *proc = containerof(head, struct process, wait_list);
        proc->state = PROC_READY;
        push(proc->pid);
    }

    atomic_increment(&(sem->count));

    restore_flags(flags);
    return 0;
}

int kill(pid_t pid) {
    unsigned long flags;

    if (ISBADPID(pid) || !pid)
        return -EINVAL;

    save_flags(flags);
    cli();

    struct process *proc = &proctab[pid];

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

    struct process *old_proc = &proctab[cproc];

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

    struct process *new_proc = &proctab[cproc];
    new_proc->state = PROC_RUNNING;

    tss_set_esp0(new_proc->kstack_ptr);

    // perform the context switch
    ctxsw(&old_proc->stack_ptr, &new_proc->stack_ptr, new_proc->cr3);
}