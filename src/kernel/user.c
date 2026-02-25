/**
 * @file user.c
 * @author verner002
 * @date 25/02/2026
*/

#include "null.h"
#include "macros.h"
#include "kernel/spinlock.h"
#include "kernel/mutex.h"
#include "kernel/user.h"
#include "mm/heap.h"

struct user {
    uid_t uid;
    gid_t gid;
    //gid_t groups;
    char const *login;
    // TODO: use sha256?
    // TODO: store database somewhere
    char const *pwd;
    spinlock_t lock;

};

static mutex_t user_mutex = {
    .locked = false
};

static int uid_seq;
static struct user *users[16];

int user_add(char const *login, char const *pwd) {
    mutex_lock(&user_mutex);

    if (uid_seq >= sizeofarray(users)) {
        mutex_unlock(&user_mutex);
        return -1; // too many users
    }

    struct user *user = (struct user *)kmalloc(sizeof(struct user));

    if (!user) {
        mutex_unlock(&user_mutex);
        return -2; // out of memory
    }

    uid_t uid = uid_seq++;

    user->uid = uid;
    user->gid = uid;
    user->login = login;
    user->pwd = pwd;
    user->lock = (spinlock_t){
        .locked = false
    };
    users[uid] = user;

    mutex_unlock(&user_mutex);
    return 0;
}

int user_init(void) {
    if (user_add("root", "root")) {
        printk("failed to add user 'root'\n");
        return -1;
    }

    for (int i = 1; i < sizeofarray(users); ++i)
        users[i] = NULL;
    
    return 0;
}

int user_login(char const *login, char const *pwd) {
    mutex_lock(&user_mutex);

    for (int i = 0; i < uid_seq; ++i) {
        struct user *user = users[i];

        spin_lock(&user->lock);

        if (!strcmp(user->login, login) && !strcmp(user->pwd, pwd)) {
            spin_unlock(&user->lock);
            mutex_unlock(&user_mutex);
            return 0; // ok
        }

        spin_unlock(&user->lock);
    }

    mutex_unlock(&user_mutex);
    return -1; // invalid login or password
}