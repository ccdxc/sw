
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

static const char *progname;
static struct opts opts;

static int
usage(void)
{
    fprintf(stderr, "usage: %s cmd [opts]\n"
        "\tcat [-x]                 - output the whole file\n"
        "\ttail [-x] [-m]           - output the entries since the last mark\n"
        "\twrite [-x] message       - write a message to the BFL\n"
        "\terase                    - erase the whole BFL\n"
        "\tstats                    - show BFL occupancy\n"
        "\tdump                     - dump raw BFL entries\n"
        "\t-m                       - write a mark entry (tail)\n"
        "\t-x                       - expand: ensure 10%% space is available\n"
        "\t-r                       - read-only; no modifications to flash\n",
        progname);
    return 1;
}

static int
parse_opts(int argc, char *argv[])
{
    int c;

    while ((c = getopt(argc, argv, "mxr")) != -1) {
        switch (c) {
        case 'm': opts.mark = 1; break;
        case 'x': opts.expand = 1; break;
        case 'r': opts.readonly = 1; break;
        default:
            return -1;
        }
    }
    argc -= optind;
    argv += optind;
    if (argc < 1) {
        return -1;
    }
    if (strcmp(argv[0], "cat") == 0) {
        opts.cmd = CM_CAT;
    } else if (strcmp(argv[0], "tail") == 0) {
        opts.cmd = CM_TAIL;
    } else if (strcmp(argv[0], "dump") == 0) {
        opts.cmd = CM_DUMP;
    } else if (strcmp(argv[0], "erase") == 0) {
        opts.cmd = CM_ERASE;
    } else if (strcmp(argv[0], "stats") == 0) {
        opts.cmd = CM_STATS;
    } else if (strcmp(argv[0], "write") == 0) {
        int pass, alen, i, len;
        char *wp;

        opts.cmd = CM_WRITE;
        if (argc < 2) {
            return -1;
        }
        opts.message = wp = NULL;
        for (pass = 1; pass <= 2; pass++) {
            len = 0;
            for (i = 1; i < argc; i++) {
                alen = strlen(argv[i]);
                if (pass == 2) {
                    if (i > 1) {
                        *wp++ = ' ';
                    }
                    strcpy(wp, argv[i]);
                    wp += alen;
                }
                len += (i > 1) + alen;
            }
            ++len; // for '\n'
            if (pass == 1) {
                opts.message = wp = malloc(len + 1);
            } else {
                *wp++ = '\n';
                *wp++ = '\0';
            }
        }
    } else {
        return -1;
    }
    if (opts.readonly && (
            opts.mark || opts.expand ||
            opts.cmd == CM_WRITE || opts.cmd == CM_ERASE)) {
        return -1;
    }
    return 0;
}

static int
is_mark(const bfl_rec_t *rec)
{
    return (rec->len == 5 && memcmp(rec->data, "@MARK", 5) == 0);
}

static void
show_rec(const bfl_rec_t *rec, void *arg)
{
    struct ctx *ctx = arg;

    if (rec->len == 0 || rec->len == 0xff) {
        return;
    }
    if (ctx->pass == 1) {
        /*
         * On pass 1, look for the last @MARK record.
         */
        if (is_mark(rec)) {
            ctx->l_mark = rec->seq;
        }
    } else {
        /*
         * On pass 2, emit the records.
         */
        if (ctx->emitting) {
            if (!is_mark(rec)) {
                fwrite(rec->data, 1, rec->len, stdout);
                ctx->output_done = 1;
            }
        } else if (rec->seq == ctx->l_mark) {
            ctx->emitting = 1;
        }
    }
}

int
main(int argc, char *argv[])
{
    int recs_capacity, recs_free, pass;
    struct ctx ctx;

    progname = argv[0];
    if (parse_opts(argc, argv) < 0) {
        return usage();
    }

    if (flash_init(opts.readonly) < 0) {
        return -1;
    }

    if (opts.cmd == CM_ERASE) {
        return flash_erase_all();
    }

    if (opts.cmd == CM_DUMP) {
        flash_dump();
        return 0;
    }

    if (bfl_fsck() < 0) {
        if (opts.readonly) {
            fprintf(stderr, "BFL corrupt; terminating\n");
            return 1;
        }
        fprintf(stderr, "BFL corrupt; resetting\n");
        if (flash_erase_all() < 0) {
            return 1;
        }
    }

    if (opts.cmd == CM_STATS) {
        recs_capacity = BFL_NSECS * BFL_RECS_PER_SEC;
        recs_free = bfl_recs_free();
        printf("capacity:  %4d\n", recs_capacity);
        printf("used:      %4d\n", recs_capacity - recs_free);
        printf("free:      %4d\n", recs_free);
        printf("occupancy: %.0f%%\n",
                100.0 - (100.0 * (double)recs_free / (double)recs_capacity));
        return 0;
    }

    /*
     * 'cat', 'tai', or 'write'
     * These commands can include the -x flag to perform a check to make sure
     * at least 10% of the BFL is available for writing.
     * The order of operations is:
     *      1. Read (cat, tail)
     *      2. Expand check (if -x selected)
     *      3. Write (write, or tail -m)
     */
    if (opts.cmd == CM_CAT || opts.cmd == CM_TAIL) {
        /*
         * Two passes are made over the records:
         * Pass 1: Look for the last @MARK record.  This will be the starting
         *         point for the 'tail' command.
         * Pass 2: Emit records to stdout.  For 'cat' this will be all records,
         *         whereas for 'tail' the output will start from the last
         *         observed @MARK (or the whole log if no @MARK was found).
         */
        ctx.l_mark = -1;
        ctx.emitting = (opts.cmd == CM_CAT);
        ctx.output_done = 0;
        for (pass = 1; pass <= 2; pass++) {
            ctx.pass = pass;
            bfl_iter(show_rec, &ctx);
            if (pass == 1 && ctx.l_mark < 0) {
                ctx.emitting = 1;
            }
        }
    }

    if (opts.expand) {
        // Ensure 10% availability
        recs_capacity = BFL_NSECS * BFL_RECS_PER_SEC;
        recs_free = bfl_recs_free();
        if ((100 * recs_free) / recs_capacity < 10) {
            if (flash_erase_sector(bfl_first_sec()) < 0) {
                return 1;
            }
        }
    }

    if (opts.cmd == CM_WRITE) {
        if (bfl_write(opts.message) < 0) {
            fprintf(stderr, "bfl_write failed\n");
            return 1;
        }
    } else if (opts.cmd == CM_TAIL && ctx.output_done && opts.mark) {
        if (bfl_write("@MARK") < 0) {
            fprintf(stderr, "bfl_write failed\n");
            return 1;
        }
    }

    return 0;
}
