
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __BFL_H__
#define __BFL_H__

typedef struct {
    void        *base;          // pointer to memory-mapped BFL
    int         log2_secsz;     // size of one sector
    int         nsecs;          // number of BFL sectors
} bfl_desc_t;
extern bfl_desc_t bfl_desc;

#define BFL_LOG2_RECSZ          5
#define BFL_LOG2_SECSZ          bfl_desc.log2_secsz
#define BFL_NSECS               bfl_desc.nsecs
#define BFL_LOG2_RECS_PER_SEC   (BFL_LOG2_SECSZ - BFL_LOG2_RECSZ)

#define BFL_SECSZ               (1 << BFL_LOG2_SECSZ)
#define BFL_RECSZ               (1 << BFL_LOG2_RECSZ)
#define BFL_RECS_PER_SEC        (1 << BFL_LOG2_RECS_PER_SEC)

#define BFL_REC_NCHARS          (BFL_RECSZ - 3)

typedef struct {
    uint16_t    seq;
    uint8_t     len;
    char        data[BFL_REC_NCHARS];
} bfl_rec_t;

// application defined write function
int bfl_sys_write(bfl_rec_t *dst, int seq, const void *src, int len);

static inline bfl_rec_t *
bfl_sec(int idx)
{
    return (bfl_rec_t *)((char *)bfl_desc.base + (idx << BFL_LOG2_SECSZ));
}

static inline bfl_rec_t *
bfl_rec(bfl_rec_t *sec, int idx)
{
    return (bfl_rec_t *)((char *)sec + (idx << BFL_LOG2_RECSZ));
}

static inline int
bfl_seq_gt(int a, int b)
{
    return ((int16_t)((a - b) & 0xffff) > 0);
}

static inline int
bfl_next_seq(int seq)
{
    return ((seq + 1) + (seq == 0xfffe)) & 0xffff;
}

int bfl_write(const char *buf);
int bfl_fsck(void);
int bfl_first_sec(void);
void bfl_iter(void (*cb)(const bfl_rec_t *rec, void *arg), void *arg);
int bfl_recs_free(void);

#endif
