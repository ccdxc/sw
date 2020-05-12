#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <inttypes.h>
#include "pal.h"

#define MAXFILENAME 128

// Lock failure retry
#define RETRY_LIMIT    4
#define RETRY_DELAY    (10 * 1000)    // 10 msec

typedef struct {
    int fd;
    int in_use;
    char filename[MAXFILENAME];
} pal_lock_int_t;

static pal_lock_int_t locks[LOCK_LAST] = {
    { .filename = "/tmp/.pal_mem.lck" },
    { .filename = "/tmp/.pal_cpld.lck" },
    { .filename = "/tmp/.pal_sbus.lck" },
};

static int
pal_get_lock_fd(const pal_lock_id_t lock_id)
{
    assert(lock_id < LOCK_LAST);

    if (!locks[lock_id].in_use) {
        locks[lock_id].fd = open(locks[lock_id].filename,
                                 O_CREAT | O_RDWR, 0666);
        locks[lock_id].in_use = 1;
    }

    return locks[lock_id].fd;
}

static pal_lock_ret_t
pal_lock(const pal_lock_id_t lock_id, const short l_type)
{
    int ret;
    int try = 1;
    struct flock lock = {
        .l_type = l_type,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };
    int fd = pal_get_lock_fd(lock_id);
    assert(fd >= 0);

    do {
        ret = fcntl(fd, F_SETLKW, &lock);
        if (ret < 0)
            usleep(RETRY_DELAY);
    } while ((ret < 0) && (++try <= RETRY_LIMIT));

    if (ret == -1) {
        return LCK_FAIL;
    } else {
        return LCK_SUCCESS;
    }
}

static pal_lock_ret_t
pal_unlock(const pal_lock_id_t lock_id)
{
    int ret;
    int try = 1;
    struct flock lock = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };
    int fd = locks[lock_id].fd;
    assert(fd >= 0);

    do {
        ret = fcntl(fd, F_SETLKW, &lock);
        if (ret < 0)
            usleep(RETRY_DELAY);
    } while ((ret < 0) && (++try <= RETRY_LIMIT));

    if (ret == -1) {
        return LCK_FAIL;
    } else {
        return LCK_SUCCESS;
    }
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
    return pal_lock(lock_id, F_WRLCK);
}

pal_lock_ret_t
pal_rd_lock(pal_lock_id_t lock_id)
{
    return pal_lock(lock_id, F_RDLCK);
}

pal_lock_ret_t
pal_wr_unlock(pal_lock_id_t lock_id)
{
    return pal_unlock(lock_id);
}

pal_lock_ret_t
pal_rd_unlock(pal_lock_id_t lock_id)
{
    return pal_unlock(lock_id);
}

