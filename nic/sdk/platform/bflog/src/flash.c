
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include "dtls.h"
#include "bfl.h"

/*
 * There are at most 2 BFL sectors, even if the mtd map says different.
 */
#define FLASH_BFL_MAX_SECTORS       2

#ifdef __aarch64__
static const char procmtd[] = "/proc/mtd";
#else
static const char procmtd[] = "data/proc/mtd";  /* testing fake path */
#endif

bfl_desc_t bfl_desc;
static int flash_fd = -1;
static char flash_path[128];

static char *
get_part_mtd(const char *name, size_t *partsizep, size_t *erasesizep)
{
    char buf[128];
    char *tok[4];
    int found, i;
    FILE *fp;

    fp = fopen(procmtd, "r");
    if (fp == NULL) {
        perror(procmtd);
        return NULL;
    }
    found = 0;
    while (fgets(buf, sizeof (buf), fp) != NULL) {
        if (buf[0] != 'm') {
            continue;
        }
        for (i = 0; i < 4; i++) {
            tok[i] = strtok(i ? NULL : buf, ": \"\n");
            if (tok[i] == NULL) {
                break;
            }
        }
        if (i < 4) {
            continue;
        }
        if (strcmp(tok[3], name) == 0) {
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (!found) {
        return NULL;
    }
    *partsizep = strtoull(tok[1], NULL, 16);
    *erasesizep = strtoull(tok[2], NULL, 16);
    return strdup(tok[0]);
}

static int
flash_readcache(void)
{
    ssize_t n;

    if (flash_fd < 0) {
        return -1;
    }
    lseek(flash_fd, 0, SEEK_SET);
    n = read(flash_fd, bfl_desc.base, BFL_NSECS * BFL_SECSZ);
    if (n < 0) {
        perror(flash_path);
        return -1;
    }
    if (n != BFL_NSECS * BFL_SECSZ) {
        fprintf(stderr, "%s: short read\n", flash_path);
        return -1;
    }
    return 0;
}

int
flash_init(int readonly)
{
    size_t partsize, erasesize;
    char *name;
    int mode;

    name = get_part_mtd("bootlog", &partsize, &erasesize);
    if (name == NULL) {
        return -1;
    }
#ifdef __aarch64__
    snprintf(flash_path, sizeof (flash_path), "/dev/%s", name);
#else
    snprintf(flash_path, sizeof (flash_path), "data/dev/%s", name); /* fake */
#endif
    free(name);

    bfl_desc.base = malloc(partsize);
    if (bfl_desc.base == NULL) {
        return -1;
    }
    bfl_desc.log2_secsz = __builtin_ctzll(erasesize);
    bfl_desc.nsecs = partsize / erasesize;
    if (bfl_desc.nsecs > FLASH_BFL_MAX_SECTORS) {
        bfl_desc.nsecs = FLASH_BFL_MAX_SECTORS;
    }

    mode = readonly ? O_RDONLY : O_RDWR;
    flash_fd = open(flash_path, mode, 0);
    if (flash_fd < 0) {
        goto bail;
    }
    if (flash_readcache() < 0) {
        goto bail2;
    }
    return 0;
bail:
    perror(flash_path);
bail2:
    if (flash_fd >= 0) {
        close(flash_fd);
        flash_fd = -1;
    }
    free(bfl_desc.base);
    bfl_desc.base = NULL;
    return -1;
}

int
bfl_sys_write(bfl_rec_t *dst, int seq, const void *src, int len)
{
    bfl_rec_t rec;
    off_t offs;

    memset(&rec, 0xff, sizeof (rec));
    rec.seq = seq;
    rec.len = len;
    memcpy(rec.data, src, len);
    offs = (char *)dst - (char *)bfl_desc.base;
    lseek(flash_fd, offs, SEEK_SET);
    if (write(flash_fd, &rec, sizeof (rec)) < 0) {
        perror("write");
        return -1;
    }
    memcpy((char *)bfl_desc.base + offs, &rec, sizeof (rec));
    return 0;
}

#ifdef __aarch64__
int
flash_erase_sector(int i)
{
    erase_info_t er;

    if (flash_fd < 0) {
        return -1;
    }
    er.start = i * BFL_SECSZ;
    er.length = BFL_SECSZ;
    if (ioctl(flash_fd, MEMERASE, &er) < 0) {
        perror("erase");
        return -1;
    }
    memset((char *)bfl_desc.base + er.start, 0xff, er.length);
    return 0;
}
#else
int
flash_erase_sector(int i)
{
    char buf[1024];
    int j;

    if (flash_fd < 0) {
        return -1;
    }
    memset(buf, 0xff, sizeof (buf));
    lseek(flash_fd, i * BFL_SECSZ, SEEK_SET);
    for (j = 0; j < BFL_SECSZ; j += sizeof (buf)) {
        if (write(flash_fd, buf, sizeof (buf)) < 0) {
            perror("flash");
            return -1;
        }
    }
    memset((char *)bfl_desc.base + i * BFL_SECSZ, 0xff, BFL_SECSZ);
    return 0;
}
#endif

int
flash_erase_all(void)
{
    int i;

    for (i = 0; i < bfl_desc.nsecs; i++) {
        if (flash_erase_sector(i) < 0) {
            return -1;
        }
    }
    return 0;
}

void
flash_dump(void)
{
    int s, r, len, i, c;
    bfl_rec_t *sec, *rec;

    for (s = 0; s < BFL_NSECS; s++) {
        sec = bfl_sec(s);
        for (r = 0; r < BFL_RECS_PER_SEC; r++) {
            rec = bfl_rec(sec, r); 
            printf("[%d][%03x]: %04x |", s, r, rec->seq);
            if (rec->seq != 0xffff) {
                len = rec->len;
                if (len == 0 || len > BFL_REC_NCHARS) {
                    printf(" INVALID (%d)", len);
                } else {
                    putchar(' ');
                    for (i = 0; i < len; i++) {
                        c = rec->data[i];
                        if (c >= ' ' && c < 127) {
                            putchar(c);
                        } else {
                            putchar('.');
                        }
                    }
                    for (; i < BFL_REC_NCHARS; i++) {
                        putchar('_');
                    }
                }
            }
            putchar('\n');
        }
    }
}
