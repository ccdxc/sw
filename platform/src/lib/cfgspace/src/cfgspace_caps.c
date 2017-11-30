/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "cfgspace.h"

/*
 * Find capability header with id "capid" in the linked list of
 * capability headers and return the config space address of it.
 * Return 0 if "capid" is not found in the list.
 */
u_int8_t
cfgspace_findcap(cfgspace_t *cs, const u_int8_t capid)
{
    u_int16_t status = cfgspace_get_status(cs);

    /* check Capability List bit in status reg */
    if (status & (1 << 4)) {
        int loops = 256 / 4; /* max-capspace / min-cap-size */
        u_int8_t capaddr;

        for (capaddr = cfgspace_get_cap(cs) & ~0x3;
             loops && capaddr != 0;
             capaddr = cfgspace_getb(cs, capaddr + 0x1) & ~0x3, loops--) {
            const u_int8_t id = cfgspace_getb(cs, capaddr);
            if (id == capid) {
                return capaddr; /* found capid at capaddr */
            }
        }
    }
    return 0; /* not found */
}

/*
 * Link new capability at offset capaddr into capability list.
 * Set Capability List bit in Status register if this is the
 * first capability in the list.
 */
void
cfgspace_linkcap(cfgspace_t *cs, const u_int8_t capaddr)
{
    u_int8_t cap, lastcap;
    int loops = 256 / 4; /* max-capspace / min-cap-size */

    assert(capaddr >= 0x40);
    assert((capaddr & 0x3) == 0);

    cap = cfgspace_get_cap(cs) & ~0x3;
    if (cap == 0) { // first cap to be added?
        u_int16_t status = cfgspace_get_status(cs);
        status |= (1 << 4); // set Capability List
        cfgspace_set_status(cs, status);
        cfgspace_set_cap(cs, capaddr); // cs->cap = capaddr
        return;
    }
    // Find end of cap list.
    do {
        lastcap = cap;
        cap = cfgspace_getb(cs, cap + 1) & ~0x3; // cap = cap->next
    } while (cap && --loops);
    assert(loops > 0);
    // Link in new cap at capaddr to last cap in the list.
    cfgspace_setb(cs, lastcap + 1, capaddr); // lastcap->next = capaddr
}

/*
 * Reference: PCIe Base Spec, Rev 4.0v0.7.
 * Reference: PCIe Base Spec, Rev 3.1a.
 * Reference: PCI Local Bus Spec, Rev 3.0.
 */
static int
cfgspace_setcap_msi(cfgspace_t *cs,
                    const cfgspace_capparams_t *cp,
                    const u_int8_t capaddr)
{
    const u_int8_t caplen = 0xe; /* 64-bit address, no per-vector mask */
    u_int16_t nintrs = cp->nintrs;
    u_int16_t nlog2;
    u_int32_t v, m;

    /* if no intrs then no msi capability */
    if (nintrs == 0) return 0;

    assert(capaddr + caplen < 0x100);
    /* msi intr count limited to 32 max */
    if (nintrs > 32) {
        nintrs = 32;
    }
    /* compute log2(nintrs) */
    for (nlog2 = 0; (1 << nlog2) < nintrs; nlog2++) {
        continue;
    }

    cfgspace_setb(cs, capaddr, 0x5);            /* msi cap id */
    cfgspace_setb(cs, capaddr + 0x1, 0);        /* cap->next = NULL */

    v = ((nlog2 << 1) |                         /* multiple msg capable */
         (1 << 7));                             /* 64-bit address capable */
    m = ((1 << 0) |                             /* msi enable */
         (0x7 << 4));                           /* multiple msg enable */
    cfgspace_setwm(cs, capaddr + 0x2, v, m);    /* msg control */

    cfgspace_setdm(cs, capaddr + 0x4, 0, 0xfffffffc); /* msg address */
    cfgspace_setdm(cs, capaddr + 0x8, 0, 0xffffffff); /* msg upper address */
    cfgspace_setwm(cs, capaddr + 0xc, 0, 0xffff);     /* msg data */

    cfgspace_linkcap(cs, capaddr);
    return caplen;
}

/*
 * Reference: PCIe Base Spec, Rev 4.0v0.7.
 * Reference: PCIe Base Spec, Rev 3.1a.
 * Reference: PCI Local Bus Spec, Rev 3.0.
 */
static int
cfgspace_setcap_msix(cfgspace_t *cs,
                     const cfgspace_capparams_t *cp,
                     const u_int8_t capaddr)
{
    const u_int8_t caplen = 0xc;
    u_int16_t nintrs = cp->nintrs;
    u_int8_t tblbir = cp->msix_tblbir;
    u_int8_t pbabir = cp->msix_pbabir;
    u_int32_t tbloff = cp->msix_tbloff;
    u_int32_t pbaoff = cp->msix_pbaoff;
    u_int32_t v, m;

    /* if no intrs then no msix capability */
    if (nintrs == 0) {
        return 0;
    }
    assert(capaddr + caplen < 0x100);
    cfgspace_setb(cs, capaddr, 0x11);         /* msi-x cap id */
    cfgspace_setb(cs, capaddr + 0x1, 0);      /* cap->next = NULL */

    nintrs = MIN(nintrs, 2048); /* max out at 2048 */
    v = nintrs - 1;
    m = ((1 << 14) |            /* function mask */
         (1 << 15));            /* msi-x enable */
    cfgspace_setwm(cs, capaddr + 0x2, v, m);  /* msg control */
    cfgspace_setd(cs, capaddr + 0x4, tbloff | tblbir);
    cfgspace_setd(cs, capaddr + 0x8, pbaoff | pbabir);
    cfgspace_linkcap(cs, capaddr);
    return caplen;
}

/*
 * Reference: PCIe Base Spec, Rev 4.0v0.7.
 * Reference: PCIe Base Spec, Rev 3.1a.
 */
static int
cfgspace_setcap_pcie(cfgspace_t *cs,
                     const cfgspace_capparams_t *cp,
                     const u_int8_t capaddr)
{
    u_int8_t caplen = 0x24; /* at least 0x24, more if DevCap2 needed */
    u_int8_t type, vers, flr, exttag, exttagen;
    u_int32_t v, m;

    cfgspace_setb(cs, capaddr, 0x10);         /* PCIe cap id */
    cfgspace_setb(cs, capaddr + 0x1, 0);      /* cap->next = NULL */

    /* flr supported in Gen2, only for endpoints */
    flr = (cp->cap_gen >= 2 &&
           cfgspace_is_endpoint(cp) &&
           cp->flr);
    exttag = cp->exttag;
    exttagen = cp->exttag_en;

    /*****************
     * PCIe Capabilities
     */
    if (cp->cap_gen == 1) {
        vers = 0x1;             /* PCIe Base Spec 1.1 */
    } else {
        vers = 0x2;             /* PCIe Base Spec 2,3,4 */
    }
    if (cfgspace_is_bridgeup(cp)) {
        type = 0x5;             /* Upstream Port of PCIe Switch */
    } else if (cfgspace_is_bridgedn(cp)) {
        type = 0x6;             /* Downstream Port of PCIe Switch */
    } else {
        type = 0;               /* PCIe Endpoint */
    }
    v = (vers |                 /* vers: PCIe Base Spec, Rev 4.0v0.7 */
         (type << 4));          /* device/port type */
    cfgspace_setw(cs, capaddr + 0x2, v);

    /*****************
     * Device Capabilities
     */
    if (cfgspace_is_bridge(cp)) {
        /* Device Capabilities (bridges) */
        v = ((0x1 << 0) |       /* MaxPayload 256 */
             (exttag << 5) |    /* Extended Tags */
             (1 << 15));        /* Error Reporting */
    } else {
        /* Device Capabilities (endpoints) */
        v = ((0x1 << 0) |       /* MaxPayload 256 */
             (exttag << 5) |    /* Extended Tags */
             (0x7 << 6) |       /* L0s Latency: No Limit */
             (0x7 << 9) |       /* L1 Latency: No Limit */
             (flr << 28) |      /* flr */
             (1 << 15));        /* Error Reporting */
    }
    cfgspace_setd(cs, capaddr + 0x4, v);

    /*****************
     * Device Control
     */
    v = ((1 << 4) |             /* Relaxed Ordering En */
         (0 << 5) |             /* Max Payload Size 128 */
         (exttagen << 8) |      /* Extended Tag Enable */
         (1 << 11) |            /* No Snoop Enable */
         (0x2 << 12));          /* Max Read Request Size 512 */
    m = ((1 << 0) |             /* Correctable Error Reporting Enable */
         (1 << 1) |             /* Non-Fatal Error Reporting Enable */
         (1 << 2) |             /* Fatal Error Reporting Enable */
         (1 << 3) |             /* Unsupported Request Rep En */
         (1 << 4) |             /* Relaxed Ordering En */
         (0x7 << 5) |           /* Max Payload Size */
         (1 << 8) |             /* Extended Tag Enable */
         (1 << 9) |             /* Phantom Functions Enable */
         (1 << 11) |            /* No Snoop Enable */
         (0x7 << 12) |          /* Max Read Request Size */
         (flr << 15));          /* endpoint: Function Level Reset */
    cfgspace_setwm(cs, capaddr + 0x8, v, m);

    /*****************
     * Device Status
     */
    cfgspace_setw(cs, capaddr + 0xa, 0);

    /*****************
     * Link Capabilities
     */
    v = ((cp->cap_gen << 0) |       /* link speed */
         (cp->cap_width << 4) |     /* link width */
         (0x0 << 10) |                  /* ASPM: not supported */
         (0x7 << 12) |                  /* L0s Exit Latency: unlimited */
         (0x7 << 15));                  /* L1 Exit Latency: unlimited */
    if (cp->cap_gen >= 3) {
        // required for gen3 and later devices
        v |= (1 << 22);                 /* ASPM Optionality Compliance */
    }
    cfgspace_setd(cs, capaddr + 0xc, v);

    /*****************
     * Link Control
     */
    v = 0;
    m = ((0x3 << 0) |                   /* ASPM Control */
         (1 << 6) |                     /* Common Clock Config */
         (1 << 7));                     /* Extended Synch */
    if (cfgspace_is_endpoint(cp)) {
        m |= (1 << 3);                  /* Rd Cpl Boundary (endpoints) */
    }
    if (cfgspace_is_bridgedn(cp)) {
        m |= ((1 << 4) |                /* Link Disable */
              (0 << 5));                /* Retrain always returns 0 on read */
    }
    cfgspace_setwm(cs, capaddr + 0x10, v, m);

    /*****************
     * Link Status
     */
    /*
     * Link Status contains negotiated link cp.
     * For now, initialize with capability cp, when the
     * physical link comes up we could update these to reflect
     * physical link negotiated values.
     */
    v = ((cp->cap_gen << 0) |       /* Init with cap speed */
         (cp->cap_width << 4));     /* Init with cap width */
    cfgspace_setw(cs, capaddr + 0x12, v);

    /*****************
     * Slot Capabilities, Slot Control, Slot Status
     */
    cfgspace_setd(cs, capaddr + 0x14, 0);
    cfgspace_setw(cs, capaddr + 0x18, 0);
    cfgspace_setw(cs, capaddr + 0x1a, 0);

    /* The following stuff was added in Gen2 */
    if (cp->cap_gen >= 2) {
        /* caplen grows to 0x3c for Gen2 and later */
        caplen = 0x3c;

        /*****************
         * Device Capabilities 2
         */
        v = 0;
        if (cfgspace_is_bridgedn(cp)) {
            v |= (1 << 5);              /* ARI Forwarding Supported */
        }
        if (cfgspace_is_bridge(cp)) {
            v |= (1 << 6);              /* AtomicOp Routing Supported */
        }
        if (cfgspace_is_endpoint(cp)) {
            v |= ((1 << 4) |            /* Completion Timeout Disable Sup */
                  (1 << 7) |            /* 32-bit AtomicOp Completer Sup */
                  (1 << 8) |            /* 64-bit AtomicOp Completer Sup */
                  (1 << 9) |            /* 128-bit CAS Completer Supported */
                  (0x3 << 12));         /* TPH Completer Supported */
        }
        if (cp->cap_gen >= 4) {
            /* let 10-bit extended tags follow normal extended tags setting */
            v |= ((exttag << 16) |      /* 10-bit Completer Tag Sup */
                  (exttag << 17));      /* 10-bit Requester Tag Sup */
        }
        cfgspace_setd(cs, capaddr + 0x24, v);

        /*****************
         * Device Control 2
         */
        v = m = 0;                      /* nothing on by default */
        if (cfgspace_is_bridgedn(cp)) {
            m |= (1 << 5);              /* ARI Forwarding Enabled */
        }
        if (cfgspace_is_bridge(cp)) {
            m |= (1 << 7);              /* AtomicOp Egress Blocking */
        }
        if (cfgspace_is_endpoint(cp)) {
            m |= ((1 << 4) |            /* Completion Timeout Disable */
                  (1 << 6));            /* AtomicOp Requester Enable */
        }
        if (cp->cap_gen >= 4) {
            m |= (exttag << 12);        /* 10-bit Requester Tags Enabled */
        }
        cfgspace_setwm(cs, capaddr + 0x28, v, m);

        /*****************
         * Device Status 2
         * (nothing spec'd here - this page intentionally left blank :-)
         */
        cfgspace_setw(cs, capaddr + 0x2a, 0);

        /*****************
         * Link Capabilities 2
         *
         * Crosslink Support activates more support in
         * Secondary PCIe extended capability.  We'll claim
         * no Crosslink Support to avoid the extra complexity.
         */
        v = 0;
        /* Link Cap 2 fields added in Gen3 and later */
        if (cp->cap_gen >= 3) {
            v |= ((((1 << cp->cap_gen) - 1) << 1) | /* Link Speed Vec */
                  (0 << 8));            /* NOT: Crosslink Supported */
            /* Gen4 added these */
            if (cp->cap_gen >= 4) {
                v |= ((1 << 23) |       /* Retimer Presence Detect Supported */
                      (1 << 24));       /* Two Retimers Presence Detect Sup */
            }
        }
        cfgspace_setd(cs, capaddr + 0x2c, v);

        /*****************
         * Link Control 2
         */
        v = m = 0;
        /* only function 0 reports these link params */
        if (!cp->fnn) {
            v |= cp->cap_gen << 0;      /* Target Link Speed */
            m |= ((0xf << 0) |          /* Target Link Speed */
                  (1 << 4) |            /* Enter Compliance */
                  (0x7 << 7) |          /* Transmit Margin */
                  (1 << 10) |           /* Enter Modified Compliance */
                  (1 << 11));           /* Compliance SOS */
            if (cp->cap_gen >= 3) {
                m |= (0xf << 12);       /* Compliance Preset/De-emphasis */
            }
        }
        cfgspace_setwm(cs, capaddr + 0x30, v, m);

        /*****************
         * Link Status 2
         */
        v = 0;
        if (cp->cap_gen >= 4) {
            v |= (0x1 << 8);            /* Crosslink Resolution: Upstream */
        }
        cfgspace_setw(cs, capaddr + 0x32, v);

        /*****************
         * Slot Capabilities 2, Slot Control 2, Slot Status 2
         */
        cfgspace_setd(cs, capaddr + 0x34, 0);
        cfgspace_setw(cs, capaddr + 0x38, 0);
        cfgspace_setw(cs, capaddr + 0x3a, 0);
    }

    assert(capaddr + caplen < 0x100);
    cfgspace_linkcap(cs, capaddr);
    return caplen;
}

/*
 * Reference: PCI Bus Power Management Spec, Rev 1.2.
 * Reference: PCIe Base Spec, Rev 4.0v0.7.
 * Reference: PCIe Base Spec, Rev 3.1a.
 */
static int
cfgspace_setcap_pm(cfgspace_t *cs,
                   const cfgspace_capparams_t *cp,
                   const u_int8_t capaddr)
{
    const u_int8_t caplen = 8;
    u_int16_t pmc, pmcsr_mask;

    assert(capaddr + caplen < cfgspace_size(cs));
    cfgspace_setb(cs, capaddr, 0x1); /* power managmenet cap id */
    cfgspace_setb(cs, capaddr + 0x1, 0); /* cap->next = NULL */

    /*****************
     * Power Management Capabilities (PMC)
     */
    pmc = 0x0003;               /* Version 1.2 */
    if (cfgspace_is_bridge(cp)) {
        /* spec: a bridge must implement at least these */
        pmc |= ((1 << 11) |     /* PME D0 */
                (1 << 14) |     /* PME D3 hot */
                (1 << 15));     /* PME D3 cold */
    }
    cfgspace_setw(cs, capaddr + 0x2, pmc);

    /*****************
     * Power Management Control/Status (PMCSR)
     */
    pmcsr_mask = (0x3 |         /* PowerState */
                  (1 << 8));    /* PME_En */
    cfgspace_setwm(cs, capaddr + 0x4, 0, pmcsr_mask);

    cfgspace_setb(cs, capaddr + 0x6, 0); /* PMCSR_BSE */
    cfgspace_setb(cs, capaddr + 0x7, 0); /* Data */
    cfgspace_linkcap(cs, capaddr);
    return caplen;
}

/*
 * Reference: PCI-to-PCI Bridge Architecture Spec, Rev 1.2
 */
static int
cfgspace_setcap_subsys(cfgspace_t *cs,
                       const cfgspace_capparams_t *cp,
                       const u_int8_t capaddr)
{
    const u_int8_t caplen = 8;

    assert(capaddr + caplen < cfgspace_size(cs));
    cfgspace_setb(cs, capaddr, 0xd);          /* subsystem ids cap id */
    cfgspace_setb(cs, capaddr + 0x1, 0);      /* cap->next = NULL */
    cfgspace_setw(cs, capaddr + 0x4, cp->subvendorid);
    cfgspace_setw(cs, capaddr + 0x6, cp->subdeviceid);
    cfgspace_linkcap(cs, capaddr);
    return caplen;
}

/******************************************************************/

typedef struct capent_s {
    const char *capname;
    int (*setcapf)(cfgspace_t *cs,
                   const cfgspace_capparams_t *cp,
                   const u_int8_t capaddr);
} capent_t;

static capent_t captab[] = {
#define CAPENT(name) \
    { #name, cfgspace_setcap_##name }
    CAPENT(msi),
    CAPENT(msix),
    CAPENT(pcie),
    CAPENT(pm),
    CAPENT(subsys),
    { NULL, NULL }
};

int
cfgspace_setcap(cfgspace_t *cs,
                const char *capname,
                const cfgspace_capparams_t *cp,
                const u_int8_t capaddr)
{
    capent_t *e;

    for (e = captab; e->capname; e++) {
        if (strcmp(e->capname, capname) == 0) {
            return e->setcapf(cs, cp, capaddr);
        }
    }
    assert(0);
    return 0;
}
