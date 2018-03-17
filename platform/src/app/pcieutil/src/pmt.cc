/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/time.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"
#include "bdf.h"
#include "pciemgrutils.h"
#include "pciehw.h"
#include "pciehw_dev.h"
#include "pal.h"
#include "cmd.h"

#define PMTF_BAR        0x01
#define PMTF_CFG        0x02
#define PMTF_RAW        0x04

static int last_hdr_displayed;

/*
idx  id typ rw p:00:00.0 0xreg_
                                vfid port 00:00.0 vfst romsk 0xaddress VINTA
 */
static void
pmt_show_cfg_entry_hdr(void)
{
    printf("%-4s %-2s %-3s %-2s %-9s %-6s "
           "%-4s %-9s %-4s %-5s %-11s %-5s\n",
           "idx", "id", "typ", "rw", "p:bdf", "cfgreg",
           "vfid", "p:bdflim", "vfst", "romsk",
           "address", "flags");
}

static void
pmt_show_cfg_entry(const int pmti, const pmt_t *pmt, const pmt_datamask_t *dm)
{
    const pmt_cfg_format_t *d = &dm->data.cfg;
    const pmt_cfg_format_t *m = &dm->mask.cfg;
    const pmr_cfg_entry_t *r = &pmt->pmre.cfg;
    const int rw = d->rw;
    const int rw_m = m->rw;

    if (last_hdr_displayed != PMTF_CFG) {
        pmt_show_cfg_entry_hdr();
        last_hdr_displayed = PMTF_CFG;
    }

    printf("%-4d %2ld %-3s %c%c %1ld:%-7s 0x%04x "
           "%4ld %ld:%-7s %4ld %5ld 0x%09" PRIx64 " %c%c%c%c%c\n",
           pmti, d->tblid,
           d->type == r->type ? pmt_type_str(d->type) : "BAD",
           ((!rw && rw_m) || !rw_m) ? 'r' : ' ',
           (( rw && rw_m) || !rw_m) ? 'w' : ' ',
           d->port, bdf_to_str(d->bdf), d->addrdw << 2,
           r->vfbase,
           r->plimit,
           bdf_to_str(bdf_make(r->blimit, r->dlimit, r->flimit)),
           r->vfstridesel,
           r->romsksel,
           (u_int64_t)r->addrdw << 2,
           d->valid && r->valid ? 'v' : '-',
           r->indirect ? 'i' : '-',
           r->notify ? 'n' : '-',
           r->td ? 't' : '-',
           r->aspace ? 'h' : '-');
}

/*
 * 6                               3
 * 3                               1                              0
 * ................................................................
 * bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbVVVVLLLPPPPP......TTT...
 * ........................................VVVVVVLLLLLPPPP...TTT...
 * bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbppppppp.................
 *
 *
 * idx id typ rw p:bb:dd.f baraddr     vf  lif   prt   pid  qtype
 *   0  0 mem rw 0:00:00.0 0x90600000     18:15 15:12 17:12 05:03
 *     prt 15 res  0x13e000000 4k -n--
 *     prt 16 db64 idx 0:0 qid 0:0  --d -pp:spp:spp:spp:spp:-pp:-pp:---
 *
 *
 * idx  tid typ rw p-bb:dd.f baraddr prtsize pgsz
 *
 * idx  tid typ rw p-bb:dd.f baraddr    vfid prts      prtsize vfrange   vflim bdf     pgsz qty:m qid
 *  32    0 mem rw 0 0x00000000    0    1 +8        4K   15 +16      1 02:00.0 0      0:0  0-1
 *  33  0 mem rw 0 0x00000000    0    9 +1       16K   14 +15      1 02:00.0 0      3:7  0-1

 *
 */
static void
pmt_show_bar_entry_hdr(void)
{
    printf("%-4s %-2s %-3s %-2s %-9s %-10s "
           "%-5s %-5s %-5s %-5s %-5s "
           "%4s:%-4s\n",
           "idx", "id", "typ", "rw", "p:bb:dd.f", "baraddr",
           " vf  ", " lif ", " prt ", " pid ", "qtype",
           "prtb", "prtc");
}

static void
pmt_show_bar_entry(const int pmti, const pmt_t *pmt, const pmt_datamask_t *dm)
{
    const pmt_bar_format_t *d = &dm->data.bar;
    const pmt_bar_format_t *m = &dm->mask.bar;
    const pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    const u_int32_t pagesize = pmr_pagesize_dec(pmr->pagesize);
    const int rw = d->rw;
    const int rw_m = m->rw;
    int pid_s, pid_e, qtype_s, qtype_e;
    int vf_s, vf_e, lif_s, lif_e, prt_s, prt_e;

    if (last_hdr_displayed != PMTF_BAR) {
        pmt_show_bar_entry_hdr();
        last_hdr_displayed = PMTF_BAR;
    }

    vf_s = pmr->vfstart;
    vf_e = pmr->vfend - 1;

    lif_s = lif_e = 0;
    prt_s = prt_e = 0;
    pid_s = pid_e = 0;
    qtype_s = qtype_e = 0;

    if (pmr->qtypemask) {
        /* 64b db pmt entry */
        lif_s = pmr->prtsize;
        lif_e = pmr->vfstart;

        prt_s = pmr->prtsize; /* no real prt selection for 64b db */
        prt_e = pmr->prtsize;

        pid_s = ffs(pagesize) - 1;
        pid_e = pmr->prtsize - 1;

        qtype_s = pmr->qtypestart;
        qtype_e = (pmr->qtypemask ?
                   qtype_s + ffs(pmr->qtypemask + 1) - 2 : qtype_s);

    } else if (pmr->qidend) {
        /* 32b/16b db pmt entry */
        lif_s = lif_e = -1;
        qtype_s = qtype_e = -1;

    } else {
        /* resource pmt entry */
        prt_s = pmr->prtsize;
        prt_e = pmr->prtsize + (ffs(pmr->prtsize) - 1);
    }

    printf("%-4d %2ld %-3s %c%c %1ld:%-7s 0x%08" PRIx64 " "
           "%2d:%-2d %2d:%-2d %2d:%-2d %2d:%-2d %2d:%-2d "
           "%4ld:%-4ld\n",
           pmti, d->tblid,
           d->type == pmr->type ? pmt_type_str(d->type) : "BAD",
           ((!rw && rw_m) || !rw_m) ? 'r' : ' ',
           (( rw && rw_m) || !rw_m) ? 'w' : ' ',
           d->port,
           bdf_to_str(pmr->bdf),
           (u_int64_t)d->addrdw << 2,
           vf_e, vf_s,
           lif_e, lif_s,
           prt_e, prt_s,
           pid_e, pid_s,
           qtype_e, qtype_s,
           pmr->prtb, pmr->prtc);
}

static void
pmt_show_raw_entry_hdr(void)
{
    printf("%-4s %-16s %-16s %-3s\n",
           "idx", "x", "y", "pmr");
}

static void
pmt_show_raw_entry(const int pmti,
                   const pmt_t *pmt,
                   const pmt_datamask_t *dm)
{
    const pmt_tcam_t *tcam = &pmt->pmte.tcam;
    const u_int32_t *w = pmt->pmre.w;

    if (last_hdr_displayed != PMTF_RAW) {
        pmt_show_raw_entry_hdr();
        last_hdr_displayed = PMTF_RAW;
    }

    printf("%-4d %016" PRIx64 " %016" PRIx64 " %08x %08x %08x %08x\n",
           pmti, tcam->x, tcam->y,
           w[0], w[1], w[2], w[3]);
}

static void
pmt_show_entry(const int pmti, const pmt_t *pmt, const int flags)
{
    pmt_datamask_t dm;
    const pmt_cmn_format_t *cmn;

    pmt_entry_dec(&pmt->pmte, &dm);
    cmn = &dm.data.cmn;

    switch (cmn->type) {
    case PMT_TYPE_CFG:
        if (flags & PMTF_CFG) {
            if (flags & PMTF_RAW) {
                pmt_show_raw_entry(pmti, pmt, &dm);
            } else {
                pmt_show_cfg_entry(pmti, pmt, &dm);
            }
        }
        break;
    case PMT_TYPE_MEM:
    case PMT_TYPE_IO:
        if (flags & PMTF_BAR) {
            if (flags & PMTF_RAW) {
                pmt_show_raw_entry(pmti, pmt, &dm);
            } else {
                pmt_show_bar_entry(pmti, pmt, &dm);
            }
        }
        break;
    case PMT_TYPE_RC:
        break;
    default:
        break;
    }
}

static void
pmt_show(const int flags)
{
    pmt_t pmt;
    int i;

    last_hdr_displayed = -1;
    for (i = 0; i < PMT_COUNT; i++) {
        pmt_get(i, &pmt);
        if (pmt_is_valid(&pmt)) {
            pmt_show_entry(i, &pmt, flags);
        }
    }
}

static void
pmt(int argc, char *argv[])
{
    int opt, flags;

    flags = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "bcr")) != -1) {
        switch (opt) {
        case 'b':
            flags |= PMTF_BAR;
            break;
        case 'c':
            flags |= PMTF_CFG;
            break;
        case 'r':
            flags |= PMTF_RAW;
            break;
        default:
            return;
        }
    }
    if ((flags & (PMTF_BAR | PMTF_CFG)) == 0) {
        flags |= PMTF_BAR | PMTF_CFG;
    }

    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    pmt_show(flags);

    pciehdev_close();
}
CMDFUNC(pmt, "pmt [-bcr]");
