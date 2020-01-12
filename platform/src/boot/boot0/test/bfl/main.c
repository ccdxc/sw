
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "bfl.h"
#include "dtls.h"

static void
write_n(int n)
{
    int i, r;

    for (i = 0; i < n; i++) {
        r = bfl_write("Test");
        assert(r == 0);
    }
}

static int
check_new_is(int idx, int seq)
{
    bfl_rec_t *rec = (bfl_rec_t *)bfl_desc.base + idx;
    if (rec->seq != seq || rec->len != 3 || memcmp(rec->data, "New", 3) != 0) {
        return -1;
    }
    return 0;
}

typedef struct {
    const char *name;
    int (*fn)(void);
    int line;
} test_t;
#define DEF_TEST(n) \
    static int do_##n(void); \
    static test_t tt_##n \
        __attribute__((aligned(sizeof (void *)), section("ttab"), used)) = \
        { .name = #n, .fn = do_##n, .line = __LINE__ }; \
    static int do_##n(void)

DEF_TEST(none)
{
    bfl_write("New");
    return (check_new_is(0, 0));
}

DEF_TEST(one)
{
    write_n(1);
    bfl_write("New");
    return (check_new_is(1, 1));
}
 
DEF_TEST(fullsec)
{
    write_n(BFL_RECS_PER_SEC);
    bfl_write("New");
    return (check_new_is(BFL_RECS_PER_SEC, BFL_RECS_PER_SEC));
}

DEF_TEST(fullsec_plus_one)
{
    write_n(BFL_RECS_PER_SEC + 1);
    bfl_write("New");
    return (check_new_is(BFL_RECS_PER_SEC + 1, BFL_RECS_PER_SEC + 1));
}

DEF_TEST(twosec)
{
    write_n(2 * BFL_RECS_PER_SEC);
    bfl_write("New");
    return (check_new_is(2 * BFL_RECS_PER_SEC, 2 * BFL_RECS_PER_SEC));
}

DEF_TEST(three_quarters)
{
    int n = BFL_NSECS * BFL_RECS_PER_SEC * 3 / 4;
    write_n(n);
    bfl_write("New");
    return (check_new_is(n, n));
}

DEF_TEST(sec_zero_erased_1p5)
{
    int n = BFL_RECS_PER_SEC * 3 / 2;
    write_n(n);
    flash_erase_sec(0);
    bfl_write("New");
    return (check_new_is(n, n));
}

DEF_TEST(sec_zero_erased_2)
{
    int n = BFL_RECS_PER_SEC * 2;
    write_n(n);
    flash_erase_sec(0);
    bfl_write("New");
    return (check_new_is(n, n));
}

static int
cmp_test(const void *a, const void *b)
{
    const test_t *ap = a;
    const test_t *bp = b;
    return (ap->line > bp->line);
}

static int
run_test_loop(void)
{
    extern char __start_ttab[];
    extern char __stop_ttab[];
    test_t *sp = (test_t *)__start_ttab;
    test_t *ep = (test_t *)__stop_ttab;

    qsort(sp, ep - sp, sizeof (*sp), cmp_test);
    for (; sp != ep; sp++) {
        flash_erase_all();
        printf("%-25s", sp->name);
        if (sp->fn() < 0) {
            puts("FAILED");
            flash_dump();
            return -1;
        }
        puts("PASS");
    }
    return 0;
}

static int verbose = 0;

static int
run_random(void)
{
    int capacity = BFL_NSECS * BFL_RECS_PER_SEC;
    int lim = capacity / 4;
    char buf[128 + 1];
    bfl_rec_t *rec;
    int exp_seq = 0;
    int exp_idx = 0;
    int exp_c;
    int len, i;

    srand(0);
    memset(buf, 'a', sizeof (buf));
    for (i = 1;; i++) {
        exp_c = 'A' + (i % 26);
        len = 1 + rand() % (sizeof (buf) - 1);
        if (verbose || i % 10000 == 0) {
            printf("## ITERATION %d: exp_idx=[%d][%03x], exp_seq=%04x, len=%3d, exp_c='%c'\n", i,
                    exp_idx / BFL_RECS_PER_SEC,
                    exp_idx % BFL_RECS_PER_SEC,
                    exp_seq, len, exp_c);
        }
        if (verbose) flash_dump();
        buf[0] = exp_c;
        buf[len] = '\0';
        if (verbose) printf("bfl_write(len=%d, c='%c')\n", len, exp_c);
        assert(strlen(buf) == len);
        if (bfl_write(buf) < 0) {
            puts("WRITE FAILED");
            return 1;
        }
        buf[len] = 'a';
        if (verbose) {
            puts("POST-WRITE");
            flash_dump();
        }
        if (bfl_recs_free() < lim) {
            if (verbose) printf("blf_recs_free = %d, Erase %d\n", bfl_recs_free(), bfl_first_sec());
            flash_erase_sec(bfl_first_sec());
            if (verbose) flash_dump();
        }
        if (bfl_fsck() < 0) {
            puts("FCSK FAILED");
            flash_dump();
            return 1;
        }
        rec = (bfl_rec_t *)bfl_desc.base + exp_idx;
        if (rec->seq != exp_seq || rec->data[0] != exp_c) {
            printf("%d: [%d][%03x] W:%04x,'%c'/G:%04x,'%c'\n", i,
                exp_idx / BFL_RECS_PER_SEC,
                exp_idx % BFL_RECS_PER_SEC,
                exp_seq, exp_c, rec->seq, rec->data[0]);
            flash_dump();
            return 1;
        }
        if (verbose) {
            printf("%d: IN: exp_idx=[%d][%03x] %03x\n", i,
                    exp_idx / BFL_RECS_PER_SEC,
                    exp_idx % BFL_RECS_PER_SEC,
                    exp_idx);
        }
        while (len > 0) {
            exp_seq = bfl_next_seq(exp_seq);
            exp_idx = (exp_idx + 1) % capacity;
            len -= BFL_REC_NCHARS;
        }
        if (verbose) {
            printf("%d: OUT: exp_idx=[%d][%03x] %03x\n", i,
                    exp_idx / BFL_RECS_PER_SEC,
                    exp_idx % BFL_RECS_PER_SEC,
                    exp_idx);
        }
    }
    return 0;
}

static int
run_auto(void)
{
    int capacity = BFL_NSECS * BFL_RECS_PER_SEC;
    int lim = capacity / 10;
    bfl_rec_t *rec;
    int exp_seq = 0;
    int exp_idx = 0;
    int i;

    for (i = 1;; i++) {
        bfl_write("Test");
        if (bfl_recs_free() < lim) {
            flash_erase_sec(bfl_first_sec());
        }
        rec = (bfl_rec_t *)bfl_desc.base + exp_idx;
        if (rec->seq != exp_seq) {
            printf("%d: W:%04x/G:%04x\n", i, exp_seq, rec->seq);
            flash_dump();
            return 1;
        }
        exp_seq = bfl_next_seq(exp_seq);
        exp_idx = (exp_idx + 1) % capacity;
    }
    return 0;
}

static int
run_loop(int dump_at)
{
    int capacity = BFL_NSECS * BFL_RECS_PER_SEC;
    int lim = capacity / 10;
    int idx, i;

    for (i = 0;; i++) {
        bfl_write("Test");
        if (bfl_recs_free() < lim) {
            idx = bfl_first_sec();
            printf("blf_recs_free=%d, Erase sector %d\n", bfl_recs_free(), idx);
            flash_erase_sec(idx);
        }
        if (i >= dump_at) {
            flash_dump();
            getchar();
        }
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    int cnt;

    flash_init();
    if (argc > 1 && strcmp(argv[1], "auto") == 0) {
        return run_auto();
    } else if (argc > 1 && strcmp(argv[1], "random") == 0) {
        return run_random();
    } else if (argc > 1 && strcmp(argv[1], "loop") == 0) {
        if (argc > 2) {
            cnt = strtoul(argv[2], NULL, 0);
        } else {
            cnt = 0;
        }
        return run_loop(cnt);
    } else {
        return run_test_loop();
    }
}

