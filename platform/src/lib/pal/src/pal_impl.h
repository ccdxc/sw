/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PAL_IMPL_H__
#define __PAL_IMPL_H__

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "cap_top_csr_defines.h"
#include "cap_ms_c_hdr.h"

/* TODO: Find a better home for this */
#define MAXUUID 32
#define FREEACCESS 0x01
#define CONTROLLEDACCESS 0x00
#define MEMLOCKFILE "/pal_mem.lck"
#define CPLDLOCKFILE "/pal_cpld.lck"

typedef struct pal_mmap_region_s {
    u_int64_t pa;
    u_int64_t sz;
    u_int32_t mapped;
    void *va;
    struct pal_mmap_region_s *next;
    struct pal_mmap_region_s *prev;
} pal_mmap_region_t;

typedef struct {
    u_int32_t memopen:1;
    u_int32_t trace_init:1;
    u_int32_t reg_trace_en:1;
    u_int32_t mem_trace_en:1;
    u_int32_t init_done:1;
    int memfd; // "/dev/mem"

    /* 
     * File based locking is selected as they are released by the OS
     * in event of the process crash/restart.
     */ 
    int memlckfd;
    int cpldlckfd;
    FILE *trfp;
    size_t nregions;
    char app_uuid[MAXUUID];

    pal_mmap_region_t *regions;
} pal_data_t;

typedef enum {
   LCK_FAIL = 0,
   LCK_SUCCESS = 1
} pal_lock_t;

static inline int
pal_lock_init(char *file) {
    int fd = 0;

    if(strcmp(file, MEMLOCKFILE) == 0) {
        /* Note: O_CREAT flag is not added here as we want the open to fail
         * if the lock file does not already exist. This is required as we
         * should be executing these lines of code iff the library has
         * already been initialized.
         */
        fd = open(MEMLOCKFILE, O_RDWR);
    } else if (strcmp(file, CPLDLOCKFILE)) {
    	fd = open(MEMLOCKFILE, O_RDWR | O_CREAT);
    }

    return fd;
}

/*
 * Locks are Co-Operative.
 *
 * It is expected that the developer uses these APIs for
 * process level synchronization to access PAL library.
 * Both Read locks (shareable by multiple applications) and
 * Write locks (Exclusive access) are provided.
 */
static inline pal_lock_t
pal_wr_lock_int(int fd)
{
    struct flock lock = {F_WRLCK, 0, SEEK_SET, 0};
    struct flock get_lock_int;

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

static inline pal_lock_t
pal_rd_lock_int(int fd)
{
    struct flock lock = {F_RDLCK, 0, SEEK_SET, 0};
    struct flock get_lock_int;

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


static inline pal_lock_t
pal_wr_unlock_int(int fd)
{
   struct flock lock = {F_UNLCK, 0, SEEK_SET, 0};

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        return LCK_FAIL;
    } else {
        return LCK_SUCCESS;
    }
}

static inline pal_lock_t
pal_rd_unlock_int(int fd)
{
   struct flock lock = {F_UNLCK, 0, SEEK_SET, 0};

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        return LCK_FAIL;
    } else {
        return LCK_SUCCESS;
    }
}

pal_data_t *pal_get_data(void);
pal_mmap_region_t *pr_getpa(const u_int64_t pa, const u_int64_t sz, u_int8_t access);

void *pr_ptov(const u_int64_t pa, const u_int64_t sz, u_int8_t access);
u_int64_t pr_vtop(const void *va, const u_int64_t sz);
void pr_mem_unmap(void *va);

#endif /* __PAL_IMPL_H__ */
