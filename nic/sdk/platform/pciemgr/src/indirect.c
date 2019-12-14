/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pcietlp/include/pcietlp.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "pciehw_impl.h"
#include "indirect.h"

#define IND_INFO_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_TGT_IND_INFO_BYTE_OFFSET)
#define IND_INFO_NWORDS 1
#define IND_INFO_STRIDE 4

static u_int64_t
ind_info_addr(const int port)
{
    return IND_INFO_BASE + (port * IND_INFO_STRIDE);
}

static u_int64_t
indirect_int_addr(void)
{
    return (CAP_ADDR_BASE_PXB_PXB_OFFSET +
            CAP_PXB_CSR_CFG_TGT_REQ_INDIRECT_INT_BYTE_OFFSET);
}

static void
indirect_int_get(u_int64_t *addrp, u_int32_t *datap)
{
    req_int_get(indirect_int_addr(), addrp, datap);
}

static const char *
indirect_reason_str(const int reason)
{
    static struct {
        const char *name;
    } reason_tab[] = {
#define PCIEIND_REASON_DEF(NAME, VAL) \
        { .name = #NAME },
#include "indirect_reason.h"
    };
    const int nreasons = sizeof(reason_tab) / sizeof(reason_tab[0]);

    if (reason < nreasons) return reason_tab[reason].name;
    return "unknown";
}

/*****************************************************************
 * aximst rams
 */
#define AXIMST_BASE     \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_AXIMST0_BYTE_ADDRESS)
#define AXIMST_STRIDE   \
    (CAP_PXB_CSR_DHS_TGT_AXIMST1_BYTE_ADDRESS - \
     CAP_PXB_CSR_DHS_TGT_AXIMST0_BYTE_ADDRESS)
#define AXIMST_NWORDS           4
#define AXIMST_ENTRY_STRIDE     32
#define AXIMST_ENTRIES_PER_PORT 16
#define AXIMST_PORTS_PER_ROW    8
#define AXIMST_PORT_STRIDE      (AXIMST_ENTRY_STRIDE * AXIMST_ENTRIES_PER_PORT)

static u_int64_t
aximst_addr(const unsigned int port,
            const unsigned int idx,
            const unsigned int entry)
{
    assert(port < AXIMST_PORTS_PER_ROW);
    assert(idx < 5);
    assert(entry < AXIMST_ENTRIES_PER_PORT);

    return (AXIMST_BASE +
            ((u_int64_t)idx * AXIMST_STRIDE) +
            ((u_int64_t)port * AXIMST_PORT_STRIDE) +
            ((u_int64_t)entry * AXIMST_ENTRY_STRIDE));
}

static void
read_aximst(const unsigned int port,
            const unsigned int idx,
            const unsigned int entry,
            u_int32_t *buf)
{
    const u_int64_t pa = aximst_addr(port, idx, entry);

    pal_reg_rd32w(pa, buf, AXIMST_NWORDS);
}

/*
 * Indirect info tlp format is reversed in srams:
 *
 *    15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 * --------------------------------------------------
 * 0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 1: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 2: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <= tlp[16]
 * 3: 00 00 00 00 60 02 00 3d 0f 00 00 3a 01 00 00 05 <= tlp[0]
 * 4: 24 48 00 00 00 04 c0 bc 05 78 02 00 40 0e 41 c4 <= indirect info
 */
static void
decode_indirect_info(u_int8_t *info, indirect_entry_t *ientry)
{
    u_int8_t *p;
    int i;

    /* copy the raw tlp data */
    p = (u_int8_t *)&ientry->rtlp;
    for (i = 0; i < sizeof(ientry->rtlp); i++) {
        p[i] = info[63 - i];
    }

    /* copy the tlp aux info */
    p = (u_int8_t *)&ientry->info;
    for (i = 0; i < sizeof(ientry->info); i++) {
        p[i] = info[64 + i];
    }
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

/*
 * Fill the buffer with the raw indirect info data
 * from the aximst srams.
 */
static void
read_indirect_info(const unsigned int port,
                   const unsigned int entry,
                   u_int8_t *buf)
{
    u_int8_t *bp;
    int i;

    for (bp = buf, i = 0; i < 5; i++, bp += 16) {
        read_aximst(port, i, entry, (u_int32_t *)bp);
    }
}

static void
read_indirect_entry(const unsigned int port,
                    const unsigned int entry,
                    indirect_entry_t *ientry)
{
    u_int8_t buf[80];

    read_indirect_info(port, entry, buf);
    decode_indirect_info(buf, ientry);
    ientry->port = port;
}

static int
read_pending_indirect_entry(const unsigned int port,
                            indirect_entry_t *ientry)
{
    int entry, pending;

    read_ind_info(port, &entry, &pending);
    read_indirect_entry(port, entry, ientry);
    return pending;
}

void
pciehw_indirect_complete(indirect_entry_t *ientry)
{
#define IND_RSP_ADDR    \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + \
     CAP_PXB_CSR_DHS_TGT_IND_RSP_ENTRY_BYTE_ADDRESS)
#define IND_RSP_NWORDS  5
    union {
        struct {
            u_int32_t data0;
            u_int32_t data1;
            u_int32_t data2;
            u_int32_t data3;
            u_int32_t cpl_stat:3;
            u_int32_t port_id:3;
            u_int32_t axi_id:7;
            u_int32_t fetch_rsp:1;
        } __attribute__((packed));
        u_int32_t w[IND_RSP_NWORDS];
    } ind_rsp;
    const u_int64_t pa = ientry->info.direct_addr;
    const size_t sz = ientry->info.direct_size;

    /*
     * This indirect transaction was handled by software.
     * We might have written some memory that will be read
     * by subsequent direct transactions handled in hw.
     * Insert barrier here to be sure all memory writes have
     * landed so hw will always see the data we wrote.
     */
    PAL_barrier();

    if (sz < 4 && (pa & 0x3)) {
        /*
         * If sub-dword read, shift return data to the correct
         * byte lanes expected for this transaction.
         *
         *     data0 = data0 << (address-dword-offset * 8);
         */
        ind_rsp.data0 = ientry->data[0] << ((pa & 0x3) << 3);
    } else {
        ind_rsp.data0 = ientry->data[0];
        ind_rsp.data1 = ientry->data[1];
        ind_rsp.data2 = ientry->data[2];
        ind_rsp.data3 = ientry->data[3];
    }
    ind_rsp.cpl_stat = ientry->cpl;
    ind_rsp.port_id = ientry->port;
    ind_rsp.axi_id = ientry->info.context_id;
    ind_rsp.fetch_rsp = 0;

    pal_reg_wr32w(IND_RSP_ADDR, ind_rsp.w, IND_RSP_NWORDS);
}

static void
handle_indirect(indirect_entry_t *ientry)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const u_int32_t pmti = ientry->info.pmti;
    pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    pciehw_port_t *p = &pshmem->port[ientry->port];
    pcie_stlp_t stlpbuf, *stlp = &stlpbuf;

    pcietlp_decode(stlp, ientry->rtlp, sizeof(ientry->rtlp));

    switch (stlp->type) {
    case PCIE_STLP_CFGRD:
    case PCIE_STLP_CFGRD1:
        pciehw_cfgrd_indirect(ientry, stlp);
        spmt->swrd++;
        p->stats.ind_cfgrd++;
        break;
    case PCIE_STLP_CFGWR:
    case PCIE_STLP_CFGWR1:
        pciehw_cfgwr_indirect(ientry, stlp);
        spmt->swwr++;
        p->stats.ind_cfgwr++;
        break;
    case PCIE_STLP_MEMRD:
    case PCIE_STLP_MEMRD64:
        pciehw_barrd_indirect(ientry, stlp);
        spmt->swrd++;
        p->stats.ind_memrd++;
        break;
    case PCIE_STLP_MEMWR:
    case PCIE_STLP_MEMWR64:
        pciehw_barwr_indirect(ientry, stlp);
        spmt->swwr++;
        p->stats.ind_memwr++;
        break;
    case PCIE_STLP_IORD:
        pciehw_barrd_indirect(ientry, stlp);
        spmt->swrd++;
        p->stats.ind_iord++;
        break;
    case PCIE_STLP_IOWR:
        pciehw_barwr_indirect(ientry, stlp);
        spmt->swwr++;
        p->stats.ind_iowr++;
        break;
    default:
        ientry->cpl = PCIECPL_UR;
        pciehw_indirect_complete(ientry);
        p->stats.ind_unknown++;
        break;
    }
}

/*
 * Each port can have only 1 outstanding indirect transaction.
 * XXX move to pciehw_port_t?
 */
static indirect_entry_t indirect_entry[PCIEHW_NPORTS];

int
pciehw_indirect_intr(const int port)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_port_t *p = &pshmem->port[port];
    indirect_entry_t *ientry = &indirect_entry[port];
    const int pending = read_pending_indirect_entry(port, ientry);

    p->stats.ind_intr++;
    if (!pending) {
        p->stats.ind_spurious++;
        return -1;
    }

    ientry->cpl = PCIECPL_SC; /* assume success */
    handle_indirect(ientry);
    return 0;
}

int
pciehw_indirect_intr_init(const int port,
                          const u_int64_t msgaddr, const u_int32_t msgdata)
{
    return req_int_init(indirect_int_addr(), "indirect_intr", port,
                        msgaddr, msgdata | 0x80000000);
}

/******************************************************************
 * apis
 */

int
pciehw_indirect_init(void)
{
    return 0;
}

int
pciehw_indirect_poll(const int port)
{
    int pending;

    read_ind_info(port, NULL, &pending);
    if (pending) {
        pciehw_indirect_intr(port);
    }
    return 0;
}

/*
 * Arrange to have the notify interrupt written to memory,
 * then we can poll memory locations to see if there is work to do.
 */
int
pciehw_indirect_poll_init(const int port)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    const u_int64_t msgaddr = pal_mem_vtop(&phwmem->indirect_intr_dest[port]);
    const u_int32_t msgdata = 1;

    return req_int_init(indirect_int_addr(), "indirect_intr",
                        port, msgaddr, msgdata);
}

/*
 * Disable any indirect handling.  The indirect requests are coming
 * from the PMT/PRT table entries.  We could clean those so they
 * don't generate an interrupt but for now we leave them in place
 * so indirect will generate an entry in the ring and attempt to
 * generate an interrupt.
 */
void
pciehw_indirect_disable(const int port)
{
}

void
pciehw_indirect_disable_all_ports(void)
{
    int port;

    for (port = 0; port < PCIEHW_NPORTS; port++) {
        pciehw_indirect_disable(port);
    }
}

/******************************************************************
 * debug
 */

static void
show_ientry(const indirect_entry_t *ientry)
{
    const tlpauxinfo_t *info = &ientry->info;

    pciesys_loginfo("pmti %d reason %s %c%c%c%c%c%c%c%c%c%c "
                    "addr 0x%08"PRIx64" sz %d\n",
                    info->pmti,
                    indirect_reason_str(info->indirect_reason),
                    info->pmt_hit     ? 'p' : '-',
                    info->is_direct   ? 'd' : '-',
                    info->is_indirect ? 'i' : '-',
                    info->is_notify   ? 'n' : '-',
                    info->is_ur       ? 'u' : '-',
                    info->is_ca       ? 'a' : '-',
                    info->is_host     ? 'h' : '-',
                    info->aspace      ? 'H' : '-',
                    info->sop         ? 's' : '-',
                    info->eop         ? 'e' : '-',
                    (u_int64_t)info->direct_addr,
                    (u_int32_t)info->direct_size);
}

void
pciehw_aximst_show(const unsigned int port,
                   const unsigned int entry,
                   const int flags,
                   const u_int64_t tm)
{
    u_int8_t buf[80], *bp;
    indirect_entry_t ientry_buf, *ientry = &ientry_buf;
    pcie_stlp_t stlpbuf, *stlp = &stlpbuf;
    int i;

    read_indirect_info(port, entry, buf);
    decode_indirect_info(buf, ientry);
    pcietlp_decode(stlp, ientry->rtlp, sizeof(ientry->rtlp));

    if (flags & AXIMSTF_TLP) {
        if (flags & AXIMSTF_TS) {
            pciesys_loginfo("[+%010.6lf] %s\n",
                            tm / 1000000.0, pcietlp_str(stlp));
        } else {
            pciesys_loginfo("%s\n", pcietlp_str(stlp));
        }
    }
    if (flags & AXIMSTF_IND) {
        show_ientry(ientry);
    }
    if (flags & AXIMSTF_RAW) {
        char line[80];

        for (bp = buf, i = 0; i < 5; i++, bp += 16) {
            hex_format(line, sizeof(line), bp, 16);
            pciesys_loginfo("aximst%d: %s\n", i, line);
        }
    }

    /* reference for safe keeping */
    if (0) indirect_int_get(NULL, NULL);
}
