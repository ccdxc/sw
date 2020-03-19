/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "platform/cfgspace/include/cfgspace.h"
#include "cfgspace_bars.h"

static u_int16_t
extcap_get_id(u_int32_t caphdr)
{
    return caphdr & 0x0000ffff;
}

static u_int16_t
extcap_get_next(u_int32_t caphdr)
{
    return (caphdr >> 20) & 0xffc;
}

static void
extcap_set_next(u_int32_t *caphdr, u_int16_t next)
{
    *caphdr |= ((next & 0xfff) << 20);
}

/*
 * Find extended capability header with id "capid" in the linked list of
 * extended capability headers and return the config space address of it.
 * Return 0 if "capid" is not found in the list.
 */
u_int16_t
cfgspace_findextcap(cfgspace_t *cs, const u_int16_t capid)
{
    u_int16_t cap;
    u_int32_t caphdr;
    int loops = cfgspace_size(cs) / 4; /* (config size) / (min cap size) */

    cap = 0x100;
    do {
        caphdr = cfgspace_getd(cs, cap);
        if (extcap_get_id(caphdr) == capid) {
            return cap;
        }
        cap = extcap_get_next(caphdr);
    } while (cap && --loops);

    return 0; /* not found */
}

/*
 * Link new extended capability at offset capaddr into extended
 * capability list.
 */
void
cfgspace_linkextcap(cfgspace_t *cs, const u_int16_t capaddr)
{
    u_int16_t cap;
    u_int32_t caphdr;
    int loops = cfgspace_size(cs) / 4; /* (config size) / (min cap size) */

    assert(capaddr >= 0x100);
    assert((capaddr & 0x3) == 0);

    if (capaddr == 0x100) {
        /* Linking first extended cap, so nothing to do. */
        return;
    }

    cap = 0x100;
    caphdr = cfgspace_getd(cs, cap);
    while (extcap_get_next(caphdr) && --loops) {
        cap = extcap_get_next(caphdr);
        caphdr = cfgspace_getd(cs, cap);
    }

    if (loops != 0) {
        extcap_set_next(&caphdr, capaddr);
        cfgspace_setd(cs, cap, caphdr);
    }
}

static int
cfgspace_setextcap_acs(cfgspace_t *cs,
                       const cfgspace_capparams_t *cp,
                       const u_int16_t capaddr)
{
    const u_int16_t caplen = 0xc;
    u_int32_t v, m;

    assert(capaddr + caplen < cfgspace_size(cs));

    /*****************
     * PCIe Extended Capability Header
     */
    v = (0xd | (0x1 << 16));    /* acs cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    /*****************
     * ACS Capability
     */
    v = ((1 << 0) |             /* V */
         (1 << 1) |             /* B */
         (1 << 2) |             /* R */
         (1 << 3) |             /* C */
         (1 << 4) |             /* U */
                                /* no E */
         (1 << 6));             /* T */
    cfgspace_setw(cs, capaddr + 0x4, v);

    /*****************
     * ACS Control
     */
    m = ((1 << 0) |             /* V */
         (1 << 1) |             /* B */
         (1 << 2) |             /* R */
         (1 << 3) |             /* C */
         (1 << 4) |             /* U */
                                /* no E */
         (1 << 6));             /* T */
    cfgspace_setwm(cs, capaddr + 0x6, 0, m);

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

static int
cfgspace_setextcap_aer(cfgspace_t *cs,
                       const cfgspace_capparams_t *cp,
                       const u_int16_t capaddr)
{
    const u_int16_t caplen = 0x48;
    u_int32_t v, m;

    assert(capaddr + caplen < cfgspace_size(cs));

    /*****************
     * PCIe Extended Capability Header
     */
    v = 0x1;                    /* aer cap id */
    if (cp->cap_gen <= 1) {
        v |= (0x1 << 16);       /* version 1 */
    } else {
        v |= (0x2 << 16);       /* version 2 */
    }
    cfgspace_setd(cs, capaddr, v);

    /*****************
     * Uncorrectable Error Status
     */
    cfgspace_setd(cs, capaddr + 0x4, 0);

    /*****************
     * Uncorrectable Error Mask
     */
    v = 0;
    m = ((1 << 4) |             /* Data Link Protocol */
         (1 << 12) |            /* Poisoned TLP */
         (1 << 16) |            /* Unexpected Completion */
         (1 << 18) |            /* Malformed TLP */
         (1 << 20));            /* Unsupport Request */
    cfgspace_setdm(cs, capaddr + 0x8, v, m);

    /*****************
     * Uncorrectable Error Severity
     */
    v = ((1 << 4) |             /* Data Link Protocol */
         (1 << 18));            /* Malformed TLP */
    m = ((1 << 4) |             /* Data Link Protocol */
         (1 << 12) |            /* Poisoned TLP */
         (1 << 16) |            /* Unexpected Completion */
         (1 << 18) |            /* Malformed TLP */
         (1 << 20));            /* Unsupport Request */
    cfgspace_setdm(cs, capaddr + 0xc, v, m);

    /*****************
     * Correctable Error Status
     */
    cfgspace_setd(cs, capaddr + 0x10, 0);

    /*****************
     * Correctable Error Mask
     */
    v = (1 << 13);              /* Advisory Non-fatal */
    m = ((1 << 0) |             /* Receiver Error */
         (1 << 6) |             /* Bad TLP */
         (1 << 7) |             /* Bad DLLP */
         (1 << 12));            /* Replay Timer Timeout */
    cfgspace_setdm(cs, capaddr + 0x14, v, m);

    /*****************
     * Advanced Error Capabilities
     */
    v = ((1 << 5) |             /* ECRC Generation capable */
         (1 << 7));             /* ECRC Check capable */
    m = ((1 << 6) |             /* ECRC Generation enable */
         (1 << 8));             /* ECRC Check enable */
    cfgspace_setdm(cs, capaddr + 0x18, v, m);

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

static int
cfgspace_setextcap_ari(cfgspace_t *cs,
                       const cfgspace_capparams_t *cp,
                       const u_int16_t capaddr)
{
    const u_int16_t caplen = 0x8;
    const u_int8_t ari_nextfunc = 0;
    u_int32_t v;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0xe | (0x1 << 16));                  /* ari cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    v = (ari_nextfunc << 8);
    cfgspace_setw(cs, capaddr + 0x4, v);      /* ari capability */
    cfgspace_setw(cs, capaddr + 0x6, 0);      /* ari control */

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

static int
cfgspace_setextcap_datalink(cfgspace_t *cs,
                            const cfgspace_capparams_t *cp,
                            const u_int16_t capaddr)
{
    const u_int16_t caplen = 0xc;
    u_int32_t v;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0x25 | (0x1 << 16));    /* data link feature cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    /*****************
     * Data Link Feature Capabilities
     */
    /* Scaled Flow Control MUST be set for Gen4 */
    v = ((1 << 0) |             /* Scaled Flow Control Supported */
         (1 << 31));            /* Data Link Feature Exchange Enable */
    cfgspace_setd(cs, capaddr + 0x4, v);

    /*****************
     * Data Link Feature Status
     */
    v = 0;
    cfgspace_setd(cs, capaddr + 0x8, v);

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

static int
cfgspace_setextcap_dsn(cfgspace_t *cs,
                       const cfgspace_capparams_t *cp,
                       const u_int16_t capaddr)
{
    const u_int16_t caplen = 0xc;
    const u_int64_t dsn = cp->dsn;
    u_int32_t v;
    int i;

    /* if no dsn then no dsn capability */
    if (dsn == 0) return 0;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0x3 | (0x1 << 16));    /* dsn cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    for (i = 0; i < 8; i++) {
        cfgspace_setb(cs, capaddr + 0x4 + i, dsn >> (i * 8));
    }

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

/*
 * Lane Margining at the Receiver
 */
static int
cfgspace_setextcap_lanemargin(cfgspace_t *cs,
                              const cfgspace_capparams_t *cp,
                              const u_int16_t capaddr)
{
    const u_int16_t caplen = 0x88;
    u_int16_t ctrlv, ctrlm, statv, statm;
    u_int32_t v;
    int i;

    /* required only for Gen4 speeds, and only function 0 */
    if (cp->cap_gen < 4 || cp->fnn) return 0;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0x27 | (0x1 << 16));    /* lane margin cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    /*****************
     * Margining Port Capabilities
     */
    v = 0;
    cfgspace_setw(cs, capaddr + 0x4, v);

    /*****************
     * Margining Port Status
     */
    v = 0x1;                    /* margining ready */
    cfgspace_setw(cs, capaddr + 0x6, v);

    /*****************
     * Margining Lane Control/Status
     */
    ctrlv = ((0x7 << 3) |       /* margin type */
             (0x9c << 8));      /* margin payload */
    ctrlm = ((0x7 << 0) |       /* receiver number */
             (0x7 << 3) |       /* margin type */
             (0x1 << 6) |       /* usage model */
             (0xff << 8));      /* margin payload */
    statv = 0;
    statm = 0;
    for (i = 0; i < cp->cap_width; i++) {
        cfgspace_setwm(cs, capaddr + 0x8 + (i * 2), ctrlv, ctrlm);
        cfgspace_setwm(cs, capaddr + 0xa + (i * 2), statv, statm);
    }

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

static int
cfgspace_setextcap_pasid(cfgspace_t *cs,
                         const cfgspace_capparams_t *cp,
                         const u_int16_t capaddr)
{
    const u_int16_t caplen = 0x8;
    u_int32_t v, m;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0x1b | (0x1 << 16));   /* pasid cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    v = (20 << 8);              /* Max PASID Width */
    cfgspace_setw(cs, capaddr + 0x4, v);

    v = 0;
    m = (1 << 0);               /* PASID enable */
    cfgspace_setwm(cs, capaddr + 0x6, v, m);

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

/*
 * Physical Layer (not Phy Slayer :-).
 */
static int
cfgspace_setextcap_physlayer(cfgspace_t *cs,
                             const cfgspace_capparams_t *cp,
                             const u_int16_t capaddr)
{
    const u_int16_t caplen = 0xa0;
    u_int32_t v, m;
    int i;

    /* required only for Gen4 speeds, and only function 0 */
    if (cp->cap_gen < 4 || cp->fnn) return 0;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0x26 | (0x1 << 16));   /* physical layer 16 GT/s cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    /*****************
     * 16 GT/s Lane Equalization Control
     */
    v = 0;
    if (cfgspace_is_bridgedn(cp)) {
        v |= (0xf << 0);        /* downstream port 16 GT/s tx preset */
    }
    v |= (0xf << 4);            /* upstream port 16 GT/s tx preset */
    m = 0;
    for (i = 0; i < cp->cap_width; i++) {
        cfgspace_setbm(cs, capaddr + 0x20 + i, v, m);
    }

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

/*
 * SRIOV
 */
static int
cfgspace_setextcap_sriov(cfgspace_t *cs,
                         const cfgspace_capparams_t *cp,
                         const u_int16_t capaddr)
{
    const u_int16_t caplen = 0x40;
    u_int32_t v, m;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0x10 | (0x1 << 16));   /* sriov cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    /*****************
     * SR-IOV Capabilities
     */
    v = 0;
    if (cp->cap_gen >= 4 && cp->exttag) {
        v |= (0x1 << 2);        /* VF 10-bit Requester Tag Sup */
    }
    cfgspace_setd(cs, capaddr + 0x4, v);

    /*****************
     * SR-IOV Control
     */
    v = 0;
    m = 0;
    m |= ((1 << 0) |            /* VF Enable */
          (1 << 3));            /* VF Mem Space Enable */
    /* only function 0 */
    if (!cp->fnn) {
        m |= (1 << 4);          /* ARI Capable Heirarchy */
    }
    if (cp->cap_gen >= 4 && cp->exttag) {
        m |= (1 << 5);          /* VF 10-bit Requester Tag Enable */
    }
    cfgspace_setwm(cs, capaddr + 0x8, v, m);

    /*****************
     * SR-IOV Status
     */
    v = 0;
    m = 0;
    cfgspace_setwm(cs, capaddr + 0xa, v, m);

    /*****************
     * InitialVFs
     */
    /* for SRIOV, InitialVFs should match TotalVFs */
    v = cp->totalvfs;
    m = 0;
    cfgspace_setwm(cs, capaddr + 0xc, v, m);

    /*****************
     * TotalVFs
     */
    v = cp->totalvfs;
    m = 0;
    cfgspace_setwm(cs, capaddr + 0xe, v, m);

    /*****************
     * NumVFs
     */
    v = 0;
    m = 0xffff;
    cfgspace_setwm(cs, capaddr + 0x10, v, m);

    /*****************
     * Function Dependency Link
     */
    v = 0; /* XXX should be cp->fnc, "usually" 0 */
    cfgspace_setb(cs, capaddr + 0x12, v);

    /*****************
     * First VF Offset
     */
    /* place VFs at next device number on our bus */
    v = 1;
    cfgspace_setw(cs, capaddr + 0x14, v);

    /*****************
     * VF Stride
     */
    /* subsequence VFs increment by 1 */
    v = 1;
    cfgspace_setw(cs, capaddr + 0x16, v);

    /*****************
     * VF Device ID
     */
    v = cp->vfdeviceid;
    cfgspace_setw(cs, capaddr + 0x1a, v);

    /*****************
     * Support Page Sizes
     */
    /* "required" page sizes */
    v = ((1 << 0) |             /* 4k */
         (1 << 1) |             /* 8k */
         (1 << 4) |             /* 64k */
         (1 << 6) |             /* 256k */
         (1 << 8) |             /* 1m */
         (1 << 10));            /* 4m */
    cfgspace_setd(cs, capaddr + 0x1c, v);

    /*****************
     * System Page Size
     */
    v = (1 << 0);               /* 4k */
    m = 0xffffffff;
    cfgspace_setdm(cs, capaddr + 0x20, v, m);

    /*****************
     * VF BARs
     */
    cfgspace_set_sriov_bars(cs, capaddr + 0x24, cp->vfbars, cp->nvfbars);

    /*****************
     * VF Migration State Array Offset
     */
    v = 0;
    cfgspace_setd(cs, capaddr + 0x3c, v);

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

/*
 * Secondary PCIe.
 */
static int
cfgspace_setextcap_spcie(cfgspace_t *cs,
                         const cfgspace_capparams_t *cp,
                         const u_int16_t capaddr)
{
    const u_int16_t caplen = 0xc + (cp->cap_width * 2);
    u_int32_t v, m;
    int i;

    /* pcie cap is only for function 0 */
    if (cp->fnn) return 0;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0x19 | (0x1 << 16));   /* secondary pcie cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    /*****************
     * Link Control 3
     */
    v = 0;
    m = 0;
    if (cp->cap_gen >= 3 && cfgspace_is_bridgeup(cp)) {
        m |= ((1 << 0) |        /* perform equalization */
              (1 << 1));        /* link equalization request interrupt en */
    }
    cfgspace_setdm(cs, capaddr + 0x4, v, m);

    /*****************
     * Lane Error Status
     */
    v = 0;
    m = 0;
    cfgspace_setdm(cs, capaddr + 0x8, v, m);

    /*****************
     * Lane Equalization Control
     */
    v = ((0xf << 0) |           /* downstream port transmitter preset */
         (0x7 << 4) |           /* downstream port receiver preset */
         (0xf << 8) |           /* upstream port transmitter preset */
         (0x7 << 12));          /* upstream port receiver preset */
    m = 0;
    if (cp->cap_gen >= 3 && !cfgspace_is_bridgeup(cp)) {
        for (i = 0; i < cp->cap_width; i++) {
            cfgspace_setwm(cs, capaddr + 0xc + (i * 2), v, m);
        }
    }

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

static int
cfgspace_setextcap_tph(cfgspace_t *cs,
                       const cfgspace_capparams_t *cp,
                       const u_int16_t capaddr)
{
    const u_int16_t caplen = 0xc;
    u_int32_t v, m;

    assert(capaddr + caplen < cfgspace_size(cs));

    v = (0x17 | (0x1 << 16));   /* tph cap id, version 1 */
    cfgspace_setd(cs, capaddr, v);

    v = ((1 << 0) |             /* No ST Mode Sup */
         (1 << 8));             /* Extended TPH Requester */
    cfgspace_setd(cs, capaddr + 0x4, v);

    v = 0;
    m = (0x3 << 8);             /* TPH Requester enable */
    cfgspace_setdm(cs, capaddr + 0x8, v, m);

    cfgspace_linkextcap(cs, capaddr);
    return caplen;
}

/******************************************************************/

typedef struct extcapent_s {
    const char *capname;
    int (*setextcapf)(cfgspace_t *cs,
                      const cfgspace_capparams_t *cp,
                      const u_int16_t capaddr);
} extcapent_t;

static extcapent_t extcaptab[] = {
#define EXTCAPENT(name) \
    { #name, cfgspace_setextcap_##name }
    EXTCAPENT(acs),
    EXTCAPENT(aer),
    EXTCAPENT(ari),
    EXTCAPENT(datalink),
    EXTCAPENT(dsn),
    EXTCAPENT(lanemargin),
    EXTCAPENT(pasid),
    EXTCAPENT(physlayer),
    EXTCAPENT(spcie),
    EXTCAPENT(sriov),
    EXTCAPENT(tph),
    { NULL, NULL }
};

int
cfgspace_setextcap(cfgspace_t *cs,
                   const char *capname,
                   const cfgspace_capparams_t *cp,
                   const u_int16_t capaddr)
{
    extcapent_t *e;

    for (e = extcaptab; e->capname; e++) {
        if (strcmp(e->capname, capname) == 0) {
            return e->setextcapf(cs, cp, capaddr);
        }
    }
    assert(0);
    return 0;
}
