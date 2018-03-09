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

#include "misc.h"
#include "bdf.h"
#include "pal.h"
#include "pciehsys.h"
#include "pciehost.h"
#include "pcietlp.h"
#include "pciehw.h"
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
indirect_int_set(const u_int64_t addr, const u_int32_t data)
{
    req_int_set(indirect_int_addr(), addr, data);
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

static void
indirect_init(pciehw_t *phw)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    u_int64_t pa;

    pa = pal_mem_vtop(&phwmem->indirect_intr_dest);
    indirect_int_set(pa, 1);
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
            (idx * AXIMST_STRIDE) +
            (port * AXIMST_PORT_STRIDE) +
            (entry * AXIMST_ENTRY_STRIDE));
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

    pal_reg_wr32w(IND_RSP_ADDR, ind_rsp.w, IND_RSP_NWORDS);
}

static void
handle_indirect(indirect_entry_t *ientry)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehw_port_t *p = &phwmem->port[ientry->port];
    pcie_stlp_t stlpbuf, *stlp = &stlpbuf;

    pcietlp_decode(stlp, ientry->rtlp, sizeof(ientry->rtlp));

    switch (stlp->type) {
    case PCIE_STLP_CFGRD:
        pciehw_cfgrd_indirect(ientry, stlp);
        p->indcfgrd++;
        break;
    case PCIE_STLP_CFGWR:
        pciehw_cfgwr_indirect(ientry, stlp);
        p->indcfgwr++;
        break;
    case PCIE_STLP_MEMRD:
    case PCIE_STLP_MEMRD64:
        pciehw_barrd_indirect(ientry, stlp);
        p->indmemrd++;
        break;
    case PCIE_STLP_MEMWR:
    case PCIE_STLP_MEMWR64:
        pciehw_barwr_indirect(ientry, stlp);
        p->indmemwr++;
        break;
    case PCIE_STLP_IORD:
        pciehw_barrd_indirect(ientry, stlp);
        p->indiord++;
        break;
    case PCIE_STLP_IOWR:
        pciehw_barwr_indirect(ientry, stlp);
        p->indiowr++;
        break;
    default:
        ientry->cpl = PCIECPL_UR;
        pciehw_indirect_complete(ientry);
        p->indunknown++;
        break;
    }
}

/*
 * Each port can have only 1 outstanding indirect transaction.
 * XXX move to pciehw_port_t?
 */
static indirect_entry_t indirect_entry[PCIEHW_NPORTS];

int
pciehw_indirect_intr(pciehw_port_t *p, const int port)
{
    indirect_entry_t *ientry = &indirect_entry[port];
    const int pending = read_pending_indirect_entry(port, ientry);

    if (!pending) return -1;

    p->indirect_cnt++;

    ientry->cpl = PCIECPL_SC; /* assume success */
    handle_indirect(ientry);
    return 0;
}

/******************************************************************
 * apis
 */

int
pciehw_indirect_init(pciehw_t *phw)
{
    indirect_init(phw);
    return 0;
}

int
pciehw_indirect_poll(pciehw_t *phw)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    int port;

    if (phwmem->indirect_intr_dest == 0) return -1;
    phwmem->indirect_intr_dest = 0;

    for (port = 0; port < phw->nports; port++) {
        pciehw_port_t *p = &phwmem->port[port];
        if (pciehw_port_is_enabled(port)) {
            pciehw_indirect_intr(p, port);
        }
    }
    return 0;
}

/******************************************************************
 * debug
 */

static void
indirect_show(void)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    u_int64_t addr;
    u_int32_t data;
    const int w = 20;
    int i;

    indirect_int_get(&addr, &data);
    pciehsys_log("%-*s : 0x%08"PRIx64"\n", w, "indirect_int_addr", addr);
    pciehsys_log("%-*s : 0x%08x\n", w, "indirect_int_data", data);
    pciehsys_log("%-*s : 0x%08x\n", w,
                 "indirect_intr_dest", phwmem->indirect_intr_dest);

    pciehsys_log("%-4s %4s %s\n",
                 "port", "cnt", "last_data");
    for (i = 0; i < phw->nports; i++) {
        pciehw_port_t *p = &phwmem->port[i];
        indirect_entry_t *ientry = &indirect_entry[i];

        pciehsys_log("%-4d %4"PRId64" %08x %08x %08x %08x\n",
                     i, p->indirect_cnt,
                     ientry->data[0], ientry->data[1],
                     ientry->data[2], ientry->data[3]);
    }
}

static void
show_ientry(const indirect_entry_t *ientry)
{
    const tlpauxinfo_t *info = &ientry->info;

    pciehsys_log("pmti %d reason %s %c%c%c%c%c%c%c%c%c%c "
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

#define AXIMSTF_TLP     0x1
#define AXIMSTF_IND     0x2
#define AXIMSTF_RAW     0x4

static void
show_aximst(const unsigned int port, const unsigned int entry, const int flags)
{
    u_int8_t buf[80], *bp;
    indirect_entry_t ientry_buf, *ientry = &ientry_buf;
    pcie_stlp_t stlpbuf, *stlp = &stlpbuf;
    int i;

    read_indirect_info(port, entry, buf);
    decode_indirect_info(buf, ientry);
    pcietlp_decode(stlp, ientry->rtlp, sizeof(ientry->rtlp));

    if (flags & AXIMSTF_TLP) {
        pciehsys_log("%s\n", pcietlp_str(stlp));
    }
    if (flags & AXIMSTF_IND) {
        show_ientry(ientry);
    }
    if (flags & AXIMSTF_RAW) {
        char line[80];

        for (bp = buf, i = 0; i < 5; i++, bp += 16) {
            hex_format(line, sizeof(line), bp, 16);
            pciehsys_log("aximst%d: %s\n", i, line);
        }
    }
}

static void
cmd_aximst(int argc, char *argv[])
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
            pciehsys_error("Usage: %s [-irt][-p port][entry]\n", argv[0]);
            return;
        }
    }
    
    if (flags == 0) {
        flags = AXIMSTF_TLP | AXIMSTF_IND;
    }

#define ENTRY_MASK(e)   ((e) & 0xf)
#define ENTRY_INC(e)    ENTRY_MASK((e) + 1)

    if (optind < argc) {
        entry = ENTRY_MASK(strtoul(argv[optind], NULL, 0));
        show_aximst(port, entry, flags);
    } else {
        read_ind_info(port, &entry, NULL);
        first_entry = entry;
        do {
            pciehsys_log("---------------- "
                         "port %d entry %d "
                         "----------------\n", port, entry);
            show_aximst(port, entry, flags);
            entry = ENTRY_INC(entry);
        } while (entry != first_entry);
    }
}

static void
cmd_stats(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehw_port_t *p;
    int port = 0;
    const int w = 20;
    int opt;

    optind = 0;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            pciehsys_error("Usage: %s [-p port]\n", argv[0]);
            return;
        }
    }

    p = &phwmem->port[port];
    pciehsys_log("port %d:\n", port);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "indirect_cnt", p->indirect_cnt);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "indcfgrd", p->indcfgrd);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "indcfgwr", p->indcfgwr);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "indmemrd", p->indmemrd);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "indmemwr", p->indmemwr);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "indiord", p->indiord);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "indiowr", p->indiowr);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "indunknown", p->indunknown);

    /* XXX need notify stats */
    pciehsys_log("%-*s : %"PRIu64"\n", w, "notcfgrd", p->notcfgrd);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "notcfgwr", p->notcfgwr);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "notmemrd", p->notmemrd);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "notmemwr", p->notmemwr);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "notiord", p->notiord);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "notiowr", p->notiowr);
    pciehsys_log("%-*s : %"PRIu64"\n", w, "notunknown", p->notunknown);
}

typedef struct cmd_s {
    const char *name;
    void (*f)(int argc, char *argv[]);
    const char *desc;
    const char *helpstr;
} cmd_t;

static cmd_t cmdtab[] = {
#define CMDENT(name, desc, helpstr) \
    { #name, cmd_##name, desc, helpstr }
    CMDENT(aximst, "aximst", ""),
    CMDENT(stats, "stats", ""),
    { NULL, NULL }
};

static cmd_t *
cmd_lookup(cmd_t *cmdtab, const char *name)
{
    cmd_t *c;

    for (c = cmdtab; c->name; c++) {
        if (strcmp(c->name, name) == 0) {
            return c;
        }
    }
    return NULL;
}

void
pciehw_indirect_dbg(int argc, char *argv[])
{
    cmd_t *c;

    if (argc <= 1) {
        indirect_show();
        return;
    }

    c = cmd_lookup(cmdtab, argv[1]);
    if (c == NULL) {
        pciehsys_log("%s: debug command not found\n", argv[1]);
        return;
    }
    c->f(argc - 1, argv + 1);
}
