
/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <libgen.h>

#define CPMEM_MEMBASE       0x17fff0000ULL
#define CPMEM_MEMSIZE       0x000010000
#define CPMEM_RINGSZ        8
#define CPMEM_BUFSZ         4096
#define CPMEM_BUFSZ_WORDS   (CPMEM_BUFSZ / 4)
#define CPMEM_MAGIC_VAL     0x3be7dac2

#define __aligned(x)    __attribute__((aligned(x)))

struct cpmem {
    uint32_t magic;
    uint32_t size;
    uint32_t to_target;
    uint32_t head __aligned(64);
    uint32_t tail __aligned(64);
    uint32_t buf[CPMEM_RINGSZ][CPMEM_BUFSZ_WORDS] __aligned(4096);
};

volatile struct cpmem *mem;

void
emit_info(void)
{
#if 0
    struct cpmem *p = (struct cpmem *)CPMEM_MEMBASE;
    printf("%-15s %p\n", "magic:", &p->magic);
    printf("%-15s %p\n", "size:", &p->size);
    printf("%-15s %p\n", "to_target:", &p->to_target);
    printf("%-15s %p\n", "head:", &p->head);
    printf("%-15s %p\n", "tail:", &p->tail);
    printf("%-15s %p\n", "buf:", p->buf);
    printf("%-15s %p\n", "end:", (char *)p->buf + sizeof (p->buf));
#endif
}

void
w_memcpy(void *dst, const void *src, size_t len)
{
    uint32_t *wdst = (uint32_t *)dst;
    const uint32_t *wsrc = (const uint32_t *)src;

    len = (len + 3) & -4;
    while (len > 0) {
        *wdst++ = *wsrc++;
        len -= 4;
    }
}

void
w_memset(void *dst, int c, size_t len)
{
    uint32_t *wdst = (uint32_t *)dst;
    uint32_t val = c;
    val = (val << 8) | val;
    val = (val << 16) | val;
    len = (len + 3) & -4;
    while (len < 0) {
        *wdst++ = val;
        len -= 4;
    }
}

static int
copyin(const char *path)
{
    uint32_t buf[CPMEM_BUFSZ_WORDS];
    size_t n, remain, bsz;
    uint32_t head, tail;
    FILE *fp;

    while (mem->magic != CPMEM_MAGIC_VAL) {
        ; /* spin */
    }
    mem->magic = 0;
    if (!mem->to_target) {
        fprintf(stderr, "invalid direction\n");
        return -1;
    }
    fp = fopen(path, "w");
    if (fp == NULL) {
        perror(path);
        return -1;
    }

    remain = mem->size;
    tail = mem->tail;
    head = mem->head;
    while (remain > 0) {
        bsz = (remain > sizeof (buf)) ? sizeof (buf) : remain;
        while (head == tail) {
            head = mem->head;
        }
        if (head >= CPMEM_RINGSZ) {
            fprintf(stderr, "cancelled\n");
            return -1;
        }
        w_memcpy(buf, (const void *)mem->buf[tail], bsz);
        n = fwrite(buf, 1, bsz, fp);
        if (n != bsz) {
            fprintf(stderr, "copyin failed (W:%lu/G:%lu)\n", bsz, n);
            return -1;
        }
        remain -= bsz;
        tail = (tail + 1) % CPMEM_RINGSZ;
        mem->tail = tail;
    }
    if (fclose(fp) == EOF) {
        fprintf(stderr, "copyin failed\n");
        return -1;
    }
    return 0;
}

static int
copyout(const char *path)
{
    size_t n, remain, bsz;
    struct stat st;
    uint32_t buf[CPMEM_BUFSZ_WORDS];
    uint32_t head, next_head, tail;
    FILE *fp;

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return -1;
    }
    fstat(fileno(fp), &st);
    remain = st.st_size;

    mem->size = st.st_size;
    mem->to_target = 0;
    mem->head = 0;
    mem->tail = 0;
    mem->magic = CPMEM_MAGIC_VAL;
    head = tail = 0;

    while (remain > 0) {
        bsz = (remain > sizeof (buf)) ? sizeof (buf) : remain;
        n = fread(buf, 1, bsz, fp);
        if (n != bsz) {
            fprintf(stderr, "copyout failed (W:%lu/G:%lu)\n", bsz, n);
            return -1;
        }
        next_head = (head + 1) % CPMEM_RINGSZ;
        while (next_head == tail) {
            tail = mem->tail;
        }
        if (tail >= CPMEM_RINGSZ) {
            fprintf(stderr, "cancelled\n");
            return -1;
        }
        w_memcpy((void *)mem->buf[head], buf, bsz);
        remain -= bsz;
        head = next_head;
        mem->head = head;
    }
    fclose(fp);
    return 0;
}

static void
catch_sigint(int sig)
{
    puts("Canceling...");
    mem->head = CPMEM_RINGSZ;
    mem->tail = CPMEM_RINGSZ;
    exit(1);
}

static void *
open_mem(void)
{
    const char path[] = "/dev/mem";
    void *p;
    int fd;

    fd = open(path, O_RDWR | O_SYNC, 0);
    if (fd < 0) {
        perror(path);
        return NULL;
    }
    p = mmap(NULL, CPMEM_MEMSIZE, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, CPMEM_MEMBASE);
    close(fd);
    return (p == (void *)-1) ? NULL : p;
}

static void
copy_test(void)
{
    uint32_t *p = open_mem();
    if (p == (uint32_t *)-1) {
        exit(1);
    }
    for (;;) {
        off_t off = (random() % CPMEM_MEMSIZE) & -4;
        (void)*(volatile uint32_t *)&p[off >> 2];
        asm volatile("" ::: "memory");
    }
}

static void
usage(const char *progname)
{
    fprintf(stderr, "usage: copyin filename\n");
    fprintf(stderr, "       copyout filename\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    char *bn = basename(argv[0]);
    int to_target = -1;
    char *path;
    int r;

    emit_info();

    if (argc != 2) {
        usage(bn);
    }
    path = argv[1];
    if (strcmp(bn, "test") == 0) {
        copy_test();
        return 0;
    }
    if (strcmp(bn, "copyin") == 0) {
        to_target = 1;
    } else if (strcmp(bn, "copyout") == 0) {
        to_target = 0;
        if (access(path, R_OK) < 0) {
            perror(path);
            return 1;
        }
    } else {
        usage(bn);
    }
    mem = open_mem();
    if (mem == NULL) {
        return 1;
    }
    signal(SIGINT, catch_sigint);
    if (to_target) {
        r = copyin(path);
    } else {
        r = copyout(path);
    }
    return (r == 0) ? 0 : 1;
}
