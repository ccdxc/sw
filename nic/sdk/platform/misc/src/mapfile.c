/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>

#include "misc.h"

void *
mapfile_region(const char *file, 
               const off_t off, const size_t sz, const int oflags)
{
    int fd, prot, mapflags;
    const int pagesize = getpagesize();
    const off_t aligned_off = off & ~(pagesize - 1);
    size_t aligned_sz = roundup(off + sz, pagesize) - aligned_off;
    void *va;

    fd = open(file, oflags, 0666);
    if (fd < 0) {
        return NULL;
    }
    if (oflags & O_CREAT) {
        char z = 0;

        /* grow file to proper size */
        lseek(fd, aligned_sz - 1, SEEK_SET);
        if (write(fd, &z, 1) != 1) {
            (void)close(fd);
            return NULL;
        }
    }
    prot = PROT_READ | PROT_WRITE;
    mapflags = MAP_SHARED;
    va = mmap(NULL, aligned_sz, prot, mapflags, fd, aligned_off);
    (void)close(fd);
    if (va == (void *)-1) {
        return NULL;
    }
    return va + (off & (pagesize - 1));
}

void *
mapfile(const char *file, const size_t sz, const int oflags)
{
    return mapfile_region(file, 0, sz, oflags);
}

void
unmapfile(void *va, const size_t sz)
{
    munmap(va, roundup(sz, getpagesize()));
}
