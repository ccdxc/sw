/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/time.h>

#include "nic/sdk/platform/misc/include/misc.h"

#include "cmd.h"
#include "pcieutilpd.h"

/*
idx  typ vfst 0xaddr____ size INP
 */
static void
prt_show_res_entry_hdr(void)
{
    printf("%-4s %-4s %-4s %-11s %-4s %-5s\n",
           "idx", "type", "vfst", "address", "size", "flags");
}

static void
prt_show_res_entry(const int prti, prt_t *prt)
{
    const prt_res_t *r = &prt->res;

    printf("%-4d %-4s %-4lu 0x%09" PRIx64 " %-4s %c%c%c%c\n",
           prti,
           prt_type_str(r->type),
           r->vfstride,
           (u_int64_t)r->addrdw << 2,
           human_readable(prt_size_decode(r->sizedw)),
           r->indirect ? 'i' : '-',
           r->notify ? 'n' : '-',
           r->aspace ? 'h' : '-',
           r->pmvdis ? 'p' : '-');
}

static char *
updvec_str(const u_int64_t updvec, const int raw)
{
    static char buf[32];
    char sched, pici, pidchk, *bp;
    int i;

    bp = buf;
    for (i = 0; i < 8; i++) {
        const u_int8_t upde = (updvec >> (i * 5)) & 0x1f;

        if (raw) {
            snprintf(bp, sizeof(buf) - i * 3,
                     "%02x%s", upde, i < 7 ? ":" : "");
            bp += 3;
            continue;
        }

        switch (upde & 0x3) { /* sched bits */
        case 0: sched = '-'; break;
        case 1: sched = 'e'; break;
        case 2: sched = 'c'; break;
        case 3: sched = 's'; break;
        }
        switch ((upde >> 2) & 0x3) { /* pici bits */
        case 0: pici = '-'; break;
        case 1: pici = 'c'; break;
        case 2: pici = 'p'; break;
        case 3: pici = 'P'; break;
        }
        pidchk = upde & 0x10 ? 'p' : '-';

        snprintf(bp, sizeof(buf) - i * 4,
                 "%c%c%c%s", sched, pici, pidchk, i < 7 ? ":" : "");
        bp += 4;
    }
    return buf;
}

static void
prt_show_db_entry_hdr(void)
{
    printf("%-4s %-4s %-4s %-4s "
           "%-4s %-4s "
           "%-5s %-31s\n",
           "idx", "type", "vfst", "lif",
           "idx", "qid",
           "flags", "upd vector");
}

static void
prt_show_db_entry(const int prti, prt_t *prt, const int raw)
{
    const prt_db_t *r = &prt->db;

    printf("%-4d %-4s %-4lu %-4lu %1lu:%-2lu %1lu:%-2lu %c%c%c   %-31s\n",
           prti,
           prt_type_str(r->type),
           r->vfstride,
           r->lif,
           r->idxshift, r->idxwidth,
           r->qidshift, r->qidwidth,
           r->indirect ? 'i' : '-',
           r->notify   ? 'n' : '-',
           r->qidsel   ? 'a' : 'd',
           updvec_str(r->updvec, raw));
}

static int last_hdr_displayed = -1;

static void
prt_show_entry(const int prti, prt_t *prt, const int raw)
{
    const u_int32_t prttype = prt_type(prt);

    switch (prttype) {
    case PRT_TYPE_RES:
        if (last_hdr_displayed != PRT_TYPE_RES) {
            prt_show_res_entry_hdr();
        }
        prt_show_res_entry(prti, prt);
        break;
    case PRT_TYPE_DB64:
    case PRT_TYPE_DB32:
    case PRT_TYPE_DB16:
        if (last_hdr_displayed != PRT_TYPE_DB64 &&
            last_hdr_displayed != PRT_TYPE_DB32 &&
            last_hdr_displayed != PRT_TYPE_DB16) {
            prt_show_db_entry_hdr();
        }
        prt_show_db_entry(prti, prt, raw);
        break;
    default:
        break;
    }
    last_hdr_displayed = prttype;
}

static void
prt_show(const int raw)
{
    prt_t prt;
    int prti;

    last_hdr_displayed = -1;
    for (prti = 0; prti < PRT_COUNT; prti++) {
        prt_get(prti, &prt);
        if (prt_is_valid(&prt)) {
            prt_show_entry(prti, &prt, raw);
        }
    }
}

static void
prt(int argc, char *argv[])
{
    int opt;
    int raw = 0;

    optind = 0;
    while ((opt = getopt(argc, argv, "r")) != -1) {
        switch (opt) {
        case 'r':
            raw = 1;
            break;
        default:
            return;
        }
    }
    prt_show(raw);

    pciehdev_close();
}
CMDFUNC(prt,
"pcie resource table entries",
"prt [-r]\n"
"    -r         raw format\n");
