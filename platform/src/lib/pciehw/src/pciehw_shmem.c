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
#include <sys/stat.h>
#include <sys/mman.h>

#include "pciehcfg.h"
#include "pciehw.h"
#include "pciehw_impl.h"

int
pciehw_openmem(pciehw_t *phw, int inithw)
{
    char path[128];
    int fd, prot, oflags, mapflags;
    size_t size;
    int pagesize;
    char z;
    struct stat sb;
    void *mem;

    snprintf(path, sizeof(path), "%s/.pciehwmem", getenv("HOME"));

    oflags = O_RDWR;
    if (inithw) {
        /* if inithw, create mmap file if doesn't exit */
        oflags |= O_CREAT;
    }
    fd = open(path, oflags, 0666);
    if (fd < 0) {
        return -ENOENT;
    }
    pagesize = getpagesize();
    size = roundup(sizeof(pciehw_mem_t), pagesize);
    if (inithw) {
        /* if inithw, grow file to proper size */
        lseek(fd, size, SEEK_SET);
        z = 0;
        if (write(fd, &z, 1) != 1) {
            return -errno;
        }
    }
    if (fstat(fd, &sb) < 0) {
        (void)close(fd);
        return -EINVAL;
    }
    if (sb.st_size < size) {
        (void)close(fd);
        return -EINVAL;
    }
    prot = PROT_READ | PROT_WRITE;
    mapflags = MAP_SHARED;
    mem = mmap(NULL, size, prot, mapflags, fd, 0);
    (void)close(fd);
    if (mem == (void *)-1) {
        return -EFAULT;
    }
    phw->pciehwmem = mem;
    return 0;
}

void
pciehw_closemem(pciehw_t *phw)
{
    if (phw->pciehwmem) {
        munmap(phw->pciehwmem, sizeof(pciehw_mem_t));
    }
}
