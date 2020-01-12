
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "defs.h"
#include "bfl.h"

bfl_desc_t bfl_desc;

static int
bfl_init(void)
{
    static int init_done;
    intptr_t addr;
    uint32_t size;

    if (init_done) {
        return (bfl_desc.base == NULL) ? -1 : 0;
    }
    init_done = 1;

    if (board_get_part("bootlog", &addr, &size) < 0) {
        // No bootlog partition
        return -1;
    }
    bfl_desc.base       = (void *)addr;
    bfl_desc.log2_secsz = board_get_bfl_log2_secsize();
    bfl_desc.nsecs      = size / (1 << bfl_desc.log2_secsz);
    return 0;
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
logf(const char *fmt, ...)
{
    char buf[256];
    va_list ap;

    if (bfl_init() < 0) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(buf, sizeof (buf), fmt, ap);
    bfl_write(buf);
    va_end(ap);
}

void
logf_printf(const char *fmt, ...)
{
    char buf[256];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof (buf), fmt, ap);
    va_end(ap);
    printf("%s", buf);
    if (bfl_init() < 0) {
        return;
    }
    bfl_write(buf);
}
