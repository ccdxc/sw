/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#ifndef __CFGSPACE_H__
#define __CFGSPACE_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

/*
 * Several PCI-SIG specifications from pcisig.com are used as references.
 *
 *     PCIe Base Spec, Rev 4.0v1.0
 *     PCIe Base Spec, Rev 3.1a
 *     PCI Local Bus Spec, Rev 3.0
 *     PCI-to-PCI Bridge Architecture Spec, Rev 1.2
 *     PCI Bus Power Management Spec, Rev 1.2
 *     Single Root I/O Virtualization and Sharing Spec, Rev 1.1
 *     PCI Code and ID Assignment Spec, Rev 1.8
 */

typedef struct cfgspace_s {
    u_int8_t *cur;
    u_int8_t *msk;
    u_int8_t *rst;
    u_int16_t size;
} cfgspace_t;

static inline u_int16_t
cfgspace_size(cfgspace_t *cs)
{
    return cs->size;
}

/*
 * Low-level accessors for initializing device config space data.
 */
u_int8_t  cfgspace_getb(cfgspace_t *cs, u_int16_t offset);
u_int16_t cfgspace_getw(cfgspace_t *cs, u_int16_t offset);
u_int32_t cfgspace_getd(cfgspace_t *cs, u_int16_t offset);

void cfgspace_setb(cfgspace_t *cs, u_int16_t offset, u_int8_t  val);
void cfgspace_setw(cfgspace_t *cs, u_int16_t offset, u_int16_t val);
void cfgspace_setd(cfgspace_t *cs, u_int16_t offset, u_int32_t val);

void cfgspace_setbm(cfgspace_t *cs, u_int16_t off, u_int8_t  v, u_int8_t  m);
void cfgspace_setwm(cfgspace_t *cs, u_int16_t off, u_int16_t v, u_int16_t m);
void cfgspace_setdm(cfgspace_t *cs, u_int16_t off, u_int32_t v, u_int32_t m);

/*
 * Access specific config space registers.
 */
u_int16_t cfgspace_get_status(cfgspace_t *cs);
u_int8_t cfgspace_get_cap(cfgspace_t *cs);
u_int8_t cfgspace_get_headertype(cfgspace_t *cs);
u_int8_t cfgspace_get_intpin(cfgspace_t *cs);
u_int8_t cfgspace_get_pribus(cfgspace_t *cs);
u_int8_t cfgspace_get_secbus(cfgspace_t *cs);
u_int8_t cfgspace_get_subbus(cfgspace_t *cs);

void cfgspace_set_vendorid(cfgspace_t *cs, const u_int16_t vendorid);
void cfgspace_set_deviceid(cfgspace_t *cs, const u_int16_t deviceid);
void cfgspace_set_command(cfgspace_t *cs,
                          const u_int16_t command, const u_int16_t msk);
void cfgspace_set_status(cfgspace_t *cs, const u_int16_t status);
void cfgspace_set_revid(cfgspace_t *cs, const u_int8_t revid);
void cfgspace_set_class(cfgspace_t *cs, const u_int32_t classcode);
void cfgspace_set_cachelinesz(cfgspace_t *cs, const u_int8_t sz);
void cfgspace_set_headertype(cfgspace_t *cs, const u_int8_t headertype);
void cfgspace_set_subvendorid(cfgspace_t *cs, const u_int16_t subvendorid);
void cfgspace_set_subdeviceid(cfgspace_t *cs, const u_int16_t subdeviceid);
void cfgspace_set_cap(cfgspace_t *cs, const u_int8_t cap);
void cfgspace_set_intline(cfgspace_t *cs, const u_int8_t intline);
void cfgspace_set_intpin(cfgspace_t *cs, const u_int8_t intpin);
void cfgspace_set_pribus(cfgspace_t *cs, const u_int8_t pribus);
void cfgspace_set_secbus(cfgspace_t *cs, const u_int8_t secbus);
void cfgspace_set_subbus(cfgspace_t *cs, const u_int8_t subbus);
void cfgspace_set_iobase(cfgspace_t *cs, const u_int8_t iobase);
void cfgspace_set_iolimit(cfgspace_t *cs, const u_int8_t iolimit);
void cfgspace_set_iobase_upper(cfgspace_t *cs, const u_int16_t iobaseup);
void cfgspace_set_iolimit_upper(cfgspace_t *cs, const u_int16_t iolimitup);
void cfgspace_set_membase(cfgspace_t *cs, const u_int16_t membase);
void cfgspace_set_memlimit(cfgspace_t *cs, const u_int16_t memlimit);
void cfgspace_set_prefbase(cfgspace_t *cs, const u_int16_t prefbase);
void cfgspace_set_preflimit(cfgspace_t *cs, const u_int16_t preflimit);
void cfgspace_set_prefbase_upper(cfgspace_t *cs, const u_int32_t prefbaseup);
void cfgspace_set_preflimit_upper(cfgspace_t *cs, const u_int32_t preflimup);
void cfgspace_set_bridgectrl(cfgspace_t *cs, const u_int16_t bridgectrl);

void cfgspace_update(cfgspace_t *cs,
                     const u_int8_t gen, const u_int8_t width,
                     const u_int16_t lnksta2);

/*
 * Common header registers.
 */

typedef enum cfgspace_bartype_e {
    CFGSPACE_BARTYPE_NONE,      /* invalid bar type */
    CFGSPACE_BARTYPE_MEM,       /* 32-bit memory bar */
    CFGSPACE_BARTYPE_MEM64,     /* 64-bit memory bar */
    CFGSPACE_BARTYPE_IO,        /* 32-bit I/O bar */
} cfgspace_bartype_t;

typedef struct cfgspace_bar_s {
    cfgspace_bartype_t type;    /* bar type */
    u_int64_t size;             /* bar size */
    u_int8_t cfgidx;            /* bar index in cfg space bars region */
    u_int32_t prefetch:1;       /* prefetch enabled */
} cfgspace_bar_t;

typedef struct cfgspace_header_params_s {
    u_int16_t vendorid;         /* default vendorid */
    u_int16_t deviceid;         /* device id */
    u_int16_t subvendorid;      /* default subvendorid */
    u_int16_t subdeviceid;      /* default subdeviceid */
    u_int32_t classcode;        /* device classcode */
    u_int32_t vf;               /* SRIOV VF */
    u_int8_t revid;             /* device revision id */
    u_int8_t intpin;            /* legacy int pin IntA|B|C|D = 1|2|3|4 */
    u_int8_t nbars;             /* number of valid entries in bars[] */
    cfgspace_bar_t bars[6];     /* cfg space bar properties */
    cfgspace_bar_t rombar;      /* option rom bar */
} cfgspace_header_params_t;

void cfgspace_sethdr_type0(cfgspace_t *cs, const cfgspace_header_params_t *p);
void cfgspace_sethdr_type1(cfgspace_t *cs, const cfgspace_header_params_t *p);

/*
 * Capabilities.  All capabilities use this common param list.
 */
typedef struct cfgspace_capparams_s {
    u_int32_t bridgeup:1;       /* upstream port bridge */
    u_int32_t bridgedn:1;       /* downstream port bridge */
    u_int32_t flr:1;            /* Function Level Reset support */
    u_int32_t exttag:1;         /* extended tag capable */
    u_int32_t exttag_en:1;      /* extended tag enabled by default */
    u_int32_t fnn:1;            /* multi-function device, not function 0 */
    u_int32_t totalvfs;         /* PF sriov provides totalvfs */
    u_int16_t vfdeviceid;       /* VF device id */
    u_int16_t subvendorid;      /* default subvendorid */
    u_int16_t subdeviceid;      /* default subdeviceid */
    u_int16_t nintrs;           /* number of MSI/-X interrupts */
    u_int8_t cap_gen;           /* PCIe GenX (1,2,3,4) */
    u_int8_t cap_width;         /* lane width xX (1,2,4,8,16,32) */
    u_int8_t msix_tblbir;       /* msix bar index 0-5 */
    u_int8_t msix_pbabir;       /* msix bar index 0-5 */
    u_int32_t msix_tbloff;      /* msix table offset */
    u_int32_t msix_pbaoff;      /* msix pending bit array offset */
    u_int64_t dsn;              /* device serial number */
    u_int8_t nvfbars;           /* number of valid bars in vfbars[] */
    cfgspace_bar_t vfbars[6];   /* sriov vf bar properties */
} cfgspace_capparams_t;

static inline int
cfgspace_is_bridgeup(const cfgspace_capparams_t *cp)
{
    return cp->bridgeup;
}

static inline int
cfgspace_is_bridgedn(const cfgspace_capparams_t *cp)
{
    return cp->bridgedn;
}

static inline int
cfgspace_is_bridge(const cfgspace_capparams_t *cp)
{
    return cfgspace_is_bridgeup(cp) || cfgspace_is_bridgedn(cp);
}

static inline int
cfgspace_is_endpoint(const cfgspace_capparams_t *cp)
{
    return !cfgspace_is_bridge(cp);
}

/*
 * Capabilities.
 */
u_int8_t cfgspace_findcap(cfgspace_t *cs, const u_int8_t capid);
void cfgspace_linkcap(cfgspace_t *cs, const u_int8_t capaddr);
int cfgspace_setcap(cfgspace_t *cs,
                    const char *capname,
                    const cfgspace_capparams_t *cp,
                    const u_int8_t capaddr);

/*
 * Extended Capabilities.
 */
u_int16_t cfgspace_findextcap(cfgspace_t *cs, const u_int16_t capid);
void cfgspace_linkextcap(cfgspace_t *cs, const u_int16_t capaddr);
int cfgspace_setextcap(cfgspace_t *cs,
                       const char *capname,
                       const cfgspace_capparams_t *cp,
                       const u_int16_t capaddr);

/*
 * Config space operational accessors.
 *
 * Reads return current values, writes apply the write-mask to
 * implement read-only fields.
 */
u_int8_t  cfgspace_readb(cfgspace_t *cs, const u_int16_t offset);
u_int16_t cfgspace_readw(cfgspace_t *cs, const u_int16_t offset);
u_int32_t cfgspace_readd(cfgspace_t *cs, const u_int16_t offset);
int cfgspace_read(cfgspace_t *cs,
                  const u_int16_t offset,
                  const u_int8_t size,
                  u_int32_t *valp);

void cfgspace_writeb(cfgspace_t *cs,
                     const u_int16_t offset, const u_int8_t val);
void cfgspace_writew(cfgspace_t *cs,
                     const u_int16_t offset, const u_int16_t val);
void cfgspace_writed(cfgspace_t *cs,
                     const u_int16_t offset, const u_int32_t val);
int cfgspace_write(cfgspace_t *cs,
                   const u_int16_t offset,
                   const u_int8_t size,
                   const u_int32_t val);

#ifdef __cplusplus
}
#endif

#endif /* __CFGSPACE_H__ */
