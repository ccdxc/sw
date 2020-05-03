/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>
#include <signal.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgr/include/pciehw.h"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"

#include "cmd.h"
#include "utils.hpp"

#define IND_INFO_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_TGT_IND_INFO_BYTE_OFFSET)
#define IND_INFO_NWORDS 1
#define IND_INFO_STRIDE 4

#define ENTRY_MASK(e)   ((e) & 0xf)
#define ENTRY_INC(e)    ENTRY_MASK((e) + 1)

static u_int64_t
ind_info_addr(const int port)
{
    return IND_INFO_BASE + (port * IND_INFO_STRIDE);
}

static void
read_ind_info(const unsigned int port, int *entryp, int *pendingp)
{
    union {
        struct {
            u_int32_t pending:1;
            u_int32_t entry:4;
            u_int32_t port:3;
        } __attribute__((packed));
        u_int32_t w[IND_INFO_NWORDS];
    } ind_info;

    pal_reg_rd32w(ind_info_addr(port), ind_info.w, IND_INFO_NWORDS);

    /* assert(ind_info.port == port); */

    if (entryp) *entryp = ind_info.entry;
    if (pendingp) *pendingp = ind_info.pending;
}

static int
aximst_get_entry(const unsigned int port)
{
    int entry;

    read_ind_info(port, &entry, NULL);
    return entry;
}

/*
 * Wait for entry to advance indicating an update to the slot at entry.
 */
static void
aximst_wait_for_entry(const unsigned int port, const int entry)
{
    int cur_entry;

    do {
        cur_entry = aximst_get_entry(port);
    } while (cur_entry == entry);
}

static void
aximst_show_entry(const unsigned int port,
                  const int entry, const int flags, u_int64_t tm)
{
    printf("---------------- "
           "port %d entry %d "
           "----------------\n", port, entry);
    pciehw_aximst_show(port, entry, flags, tm);
}

static void
sighand(int s)
{
    exit(0);
}

static void
aximst(int argc, char *argv[])
{
    int port;
    int opt, entry, first_entry, flags, follow;
    u_int64_t otm, ntm;

    port = default_pcieport();
    flags = 0;
    follow = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "fip:rt")) != -1) {
        switch (opt) {
        case 'f':
            follow = 1;
            break;
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        case 'i':
            flags |= AXIMSTF_IND;
            break;
        case 'r':
            flags |= AXIMSTF_RAW;
            break;
        case 't':
            flags |= AXIMSTF_TLP;
            break;
        default:
            printf("Usage: %s [-irt][-p port][entry]\n", argv[0]);
            return;
        }
    }

    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    if (flags == 0) {
        flags = AXIMSTF_TLP | AXIMSTF_IND;
    }

    if (optind < argc) {
        entry = ENTRY_MASK(strtoul(argv[optind], NULL, 0));
        aximst_show_entry(port, entry, flags, 0);
    } else {
        entry = aximst_get_entry(port);
        first_entry = entry;
        do {
            aximst_show_entry(port, entry, flags, 0);
            entry = ENTRY_INC(entry);
        } while (entry != first_entry);

        if (follow) {
            /*
             * follow might be printing to a file,
             * arrange to flush stdout on signal.
             */
            signal(SIGINT, sighand);
            signal(SIGTERM, sighand);

            flags |= AXIMSTF_TS;
            otm = gettimestamp();
            while (1) {
                aximst_wait_for_entry(port, entry);

                ntm = gettimestamp();
                aximst_show_entry(port, entry, flags, ntm - otm);

                entry = ENTRY_INC(entry);
                otm = ntm;
            }
        }
    }

    pciehdev_close();
}
CMDFUNC(aximst,
"show aximst host transaction ring in LRU order",
"aximst [-firt][-p port][entry]\n"
"    -f         follow and wait for more entries to arrive\n"
"    -i         show indirect entry info (default)\n"
"    -r         display entries in raw format\n"
"    -t         show tlp info (default)\n"
"    -p <port>  show ring for pcie port <port>\n"
"    <entry>    show ring entry number <entry> [0-15]\n");
