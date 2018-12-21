/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"

#include "nic/sdk/platform/pal/include/pal.h"
#include "platform/src/lib/pciemgr/include/pciehw.h"
#include "platform/src/lib/pciemgr/include/pciehw_dev.h"

#include "cmd.h"

#define IND_INFO_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_TGT_IND_INFO_BYTE_OFFSET)
#define IND_INFO_NWORDS 1
#define IND_INFO_STRIDE 4

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

static void
aximst(int argc, char *argv[])
{
    static int port;
    int opt, entry, first_entry, flags;

    flags = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "ip:rt")) != -1) {
        switch (opt) {
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

#define ENTRY_MASK(e)   ((e) & 0xf)
#define ENTRY_INC(e)    ENTRY_MASK((e) + 1)

    if (optind < argc) {
        entry = ENTRY_MASK(strtoul(argv[optind], NULL, 0));
        pciehw_aximst_show(port, entry, flags);
    } else {
        read_ind_info(port, &entry, NULL);
        first_entry = entry;
        do {
            printf("---------------- "
                   "port %d entry %d "
                   "----------------\n", port, entry);
            pciehw_aximst_show(port, entry, flags);
            entry = ENTRY_INC(entry);
        } while (entry != first_entry);
    }

    pciehdev_close();
}
CMDFUNC(aximst, "aximst [-irt][-p port][entry]");
