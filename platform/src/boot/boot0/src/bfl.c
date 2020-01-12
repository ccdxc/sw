
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "bfl_sys.h"
#include "bfl.h"

/*
 * Find the (first) lowest numbered sector.
 */
int
bfl_first_sec(void)
{
    int i, seq, l_seq, l_idx;
    bfl_rec_t *rec;

    l_seq = -1;
    l_idx = -1;
    for (i = 0; i < BFL_NSECS; i++) {
        rec = bfl_sec(i);
        seq = rec->seq;
        if (seq != 0xffff) {
            if (l_idx == -1 || bfl_seq_gt(l_seq, seq)) {
                l_seq = seq;
                l_idx = i;
            }
        }
    }
    return (l_idx < 0) ? 0 : l_idx;
}

#if defined(WANT_BFL_WRITE) || defined(WANT_BFL_RECS_FREE)
/*
 * Find the next free record in a sector.
 * Returns:
 *      Pointer to next free record, or NULL if full.
 *      *seqp is seq of next free record, or 0xffff if sec is empty.
 */
static bfl_rec_t *
sec_next_free_rec(bfl_rec_t *sec, int *seqp)
{
    int idx = 0, i, seq;
    int l_seq = 0xffff;

    for (i = BFL_LOG2_RECS_PER_SEC - 1; i >= 0; i--) {
        idx |= (1 << i);
        seq = bfl_rec(sec, idx)->seq;
        if (seq == 0xffff) {
            idx &= ~(1 << i);
        } else {
            l_seq = seq;
        }
    }
    seq = bfl_rec(sec, idx)->seq;
    if (seq != 0xffff) {
        l_seq = seq;
    }
    seq = l_seq;
    if (seq != 0xffff) {
        ++idx;
        seq = bfl_next_seq(seq);
    }
    *seqp = seq;
    return (idx == BFL_RECS_PER_SEC) ? NULL :  bfl_rec(sec, idx);
}

/*
 * Find the next free record in the BFL.
 * Returns:
 *      Pointer to next free record, or NULL if BFL full.
 *      *seqp is the seq of the next free record (on success).
 */
static bfl_rec_t *
next_free_rec(int *seqp)
{
    int first_sec = bfl_first_sec();
    bfl_rec_t *sec, *rec;
    int i, l_seq = 0xffff;

    // find the first non-full sec
    sec = NULL;
    for (i = 0; i < BFL_NSECS; i++) {
        sec = bfl_sec((first_sec + i) % BFL_NSECS);
        rec = bfl_rec(sec, BFL_RECS_PER_SEC - 1);
        if (rec->seq == 0xffff) {
            // not full
            if (sec->seq == 0xffff) {
                // empty.  seq is prev + 1, or 0 if no prev
                *seqp = (l_seq == 0xffff) ? 0 : bfl_next_seq(l_seq);
                return sec;
            } else {
                // not empty. find next rec
                return sec_next_free_rec(sec, seqp);
            }
        }
        l_seq = rec->seq;   // remember last seq
    }
    // no space
    return NULL;
}
#endif

#ifdef WANT_BFL_WRITE
int
bfl_write(const char *buf)
{
    const char *e = buf + strlen(buf);
    const char *p = buf;
    bfl_rec_t *rec;
    int seq, len;

    while (p != e) {
        rec = next_free_rec(&seq);
        if (rec == NULL) {
            return -1;  // no space
        }
        len = e - p;
        len = (len > BFL_REC_NCHARS) ? BFL_REC_NCHARS : len;
        if (bfl_sys_write(rec, seq, p, len) < 0) {
            return -1;
        }
        p += len;
    }
    return 0;
}
#endif

#ifdef WANT_BFL_READ
static int
erase_check(bfl_rec_t *sec, int first)
{
    const uint32_t *sp = (uint32_t *)bfl_rec(sec, first);
    const uint32_t *ep = (uint32_t *)bfl_rec(sec, BFL_RECS_PER_SEC);
    while (sp != ep) {
        if (*sp++ != 0xffffffff) {
            return -1;
        }
    }
    return 0;
}

int
bfl_fsck(void)
{
    int idx, done, exp_seq, seq, i, j;
    bfl_rec_t *sec, *rec;

    // check written sectors have consecutive seqs
    idx = bfl_first_sec();
    sec = bfl_sec(idx);
    exp_seq = sec->seq;
    done = (exp_seq == 0xffff);
    for (i = 0; !done && i < BFL_NSECS; i++) {
        sec = bfl_sec(idx);
        rec = sec;
        for (j = 0; j < BFL_RECS_PER_SEC; j++) {
            rec = bfl_rec(sec, j);
            seq = rec->seq;
            if (seq == 0xffff) {
                // erased rec.  check that all remaining recs are empty
                if (erase_check(sec, j) < 0) {
                    fprintf(stderr, "sector %d blank-check corruption\n", idx);
                    return -1;
                }
                done = 1;
                break;
            }
            if (rec->len == 0 ||
                (rec->len != 0xff && rec->len > BFL_REC_NCHARS)) {
                fprintf(stderr, "sector %d rec-len corruption (%d)\n",
                        idx, rec->len);
                return -1;
            }
            if (seq != exp_seq) {
                fprintf(stderr, "sector %d rec-seq corruption\n", idx);
                return -1;
            }
            exp_seq = bfl_next_seq(exp_seq);
        }
        idx = (idx + 1) % BFL_NSECS;
    }
    // check that remaining sectors are all blank
    for (; i < BFL_NSECS; i++) {
        sec = bfl_sec(idx);
        if (erase_check(sec, 0) < 0) {
            fprintf(stderr, "sector %d blank-check corruption\n", idx);
            return -1;
        }
        idx = (idx + 1) % BFL_NSECS;
    }
    return 0;
}

void
bfl_iter(void (*cb)(const bfl_rec_t *rec, void *arg), void *arg)
{
    bfl_rec_t *sec, *rec;
    int idx, i, j;

    idx = bfl_first_sec();
    for (i = 0; i < BFL_NSECS; i++) {
        sec = bfl_sec(idx);
        for (j = 0; j < BFL_RECS_PER_SEC; j++) {
            rec = bfl_rec(sec, j);
            if (rec->seq == 0xffff) {
                return;
            }
            cb(rec, arg);
        }
        idx = (idx + 1) % BFL_NSECS;
    }
}

#ifdef WANT_BFL_RECS_FREE
int
bfl_recs_free(void)
{
    int capacity = BFL_NSECS * BFL_RECS_PER_SEC;
    int first_seq, next_seq, used_recs;
    bfl_rec_t *sec, *rec;

    sec = bfl_sec(bfl_first_sec());
    first_seq = sec->seq;
    if (first_seq == 0xffff) {
        return capacity;
    }
    rec = next_free_rec(&next_seq);
    if (rec == NULL) {
        return 0; // no space
    }
    used_recs = (next_seq - first_seq) & 0xffff;
    if (next_seq < first_seq) {
        --used_recs; /* wrap-around skips 0xffff */
    }
    return capacity - used_recs;
}
#endif

#endif
