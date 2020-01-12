
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "dtls.h"
#include "bfl.h"

#define FL_LOG2_SECSZ          16
#define FL_NSECS               2

bfl_desc_t bfl_desc;

void
flash_init(void)
{
    bfl_desc.base       = malloc(FL_NSECS * (1 << FL_LOG2_SECSZ));
    bfl_desc.log2_secsz = FL_LOG2_SECSZ;
    bfl_desc.nsecs      = FL_NSECS;
    memset(bfl_desc.base, 0xff, FL_NSECS * (1 << FL_LOG2_SECSZ));
}

int
bfl_sys_write(bfl_rec_t *dst, int seq, const void *src, int len)
{
    dst->seq = seq; // written first to claim the rec
    asm volatile("" ::: "memory");
    memcpy(dst->data, src, len);
    asm volatile("" ::: "memory");
    dst->len = len; // written last to complete the rec
    return 0;
}

void
flash_erase_sec(int idx)
{
    memset((char *)bfl_desc.base + BFL_SECSZ * idx, 0xff, BFL_SECSZ);
}

void
flash_erase_all(void)
{
    memset((char *)bfl_desc.base, 0xff, BFL_NSECS * BFL_SECSZ);
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
            printf("[%d][%03x]: %04x", s, r, rec->seq);
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
