
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <assert.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include "dtls.h"

void
dump(int indent, const uint8_t *buf, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            printf("%*s%04x:", indent, "", i);
        }
        printf(" %02x", buf[i]);
        if ((i % 16) == 15) {
            putchar('\n');
        }
    }
    if ((i % 16) != 0) {
        putchar('\n');
    }
}

void
w_memcpy(void *dst, const void *src, size_t len)
{
    assert((((intptr_t)dst | (intptr_t)src) & 0x3) == 0);
    size_t nwords = (len + 3) >> 2;
    volatile uint32_t *dp = (uint32_t *)dst;
    volatile const uint32_t *sp = (uint32_t *)src;
    for (size_t i = 0; i < nwords; i++) {
        *dp++ = *sp++;
    }
}

void
w_memzero(void *dst, size_t len)
{
    assert(((intptr_t)dst & 0x3) == 0);
    size_t nwords = (len + 3) >> 2;
    volatile uint32_t *dp = (uint32_t *)dst;
    for (size_t i = 0; i < nwords; i++) {
        *dp++ = 0;
    }
}

int
open_mem(const char *path, off_t offset, size_t size, void **memp)
{
    void *mem;
    int fd;

    fd = open(path, O_RDWR | O_SYNC, 0);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (mem == (void *)-1) {
        perror("mmap");
        return -1;
    }
    *memp = mem;
    return 0;
}
