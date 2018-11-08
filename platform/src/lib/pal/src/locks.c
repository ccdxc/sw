#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <inttypes.h>
#include "pal.h"

#define MAXFILENAME 128

typedef struct {
    int fd;
    int in_use;
    char filename[MAXFILENAME];
} pal_lock_int_t;

static pal_lock_int_t locks[LOCK_LAST] = {
					    {0, 0, "/tmp/.pal_mem.lck"},
					    {0, 0, "/tmp/.pal_cpld.lck"},
					 };

static int
pal_get_lock_fd(pal_lock_id_t lock_id)
{
    assert(lock_id < LOCK_LAST);

    if (!locks[lock_id].in_use) {
        locks[lock_id].fd = open(locks[lock_id].filename, O_CREAT | O_RDWR);
        locks[lock_id].in_use = 1;
    }

    return locks[lock_id].fd;
}


/*
 * Locks are Co-Operative.
 *
 * It is expected that the developer uses these APIs for
 * process level synchronization to access PAL library.
 * Both Read locks (shareable by multiple applications) and
 * Write locks (Exclusive access) are provided.
 */
pal_lock_ret_t
pal_wr_lock(pal_lock_id_t lock_id)
{
    struct flock lock = {F_WRLCK, 0, SEEK_SET, 0};
    struct flock get_lock_int;
    int fd = -1;

    fd = pal_get_lock_fd(lock_id);
    assert(fd >= 0);

    /* Checks to see if the lock is already with the process */
    fcntl(fd, F_GETLK, &get_lock_int);  /* Overwrites lock structure with preventors. */
    if (get_lock_int.l_type == F_WRLCK) {
        return LCK_FAIL;
    } else if (get_lock_int.l_type == F_RDLCK) {
        return LCK_FAIL;
    }

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        return LCK_FAIL;
    } else {
        return LCK_SUCCESS;
    }
}

pal_lock_ret_t
pal_rd_lock(pal_lock_id_t lock_id)
{
    struct flock lock = {F_RDLCK, 0, SEEK_SET, 0};
    struct flock get_lock_int;
    int fd = -1;

    fd = pal_get_lock_fd(lock_id);
    assert(fd >= 0);

    /* Checks to see if the lock is already with the process */
    fcntl(fd, F_GETLK, &get_lock_int);  /* Overwrites lock structure with preventors. */
    if (get_lock_int.l_type == F_WRLCK) {
        return LCK_FAIL;
    }

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        return LCK_FAIL;
    } else {
        return LCK_SUCCESS;
    }
}


pal_lock_ret_t
pal_wr_unlock(pal_lock_id_t lock_id)
{
    struct flock lock = {F_UNLCK, 0, SEEK_SET, 0};
    int fd = locks[lock_id].fd;

    assert(fd >= 0);

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        return LCK_FAIL;
    } else {
        return LCK_SUCCESS;
    }
}

pal_lock_ret_t
pal_rd_unlock(pal_lock_id_t lock_id)
{
    struct flock lock = {F_UNLCK, 0, SEEK_SET, 0};
    int fd = locks[lock_id].fd;

    assert(fd >= 0);

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        return LCK_FAIL;
    } else {
        return LCK_SUCCESS;
    }
}

