/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#ifndef __PCIEHW_IMPL_H__
#define __PCIEHW_IMPL_H__

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"

#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"

#include "pmt.h"
#include "prt.h"
#include "vpd.h"
#include "notify.h"
#include "indirect.h"
#include "req_int.h"
#include "event.h"

struct pciehw_s;
typedef struct pciehw_s pciehw_t;

typedef struct pciehw_mem_s {
    u_int8_t notify_area[PCIEHW_NPORTS][PCIEHW_NOTIFYSZ]
                                     __attribute__((aligned(PCIEHW_NOTIFYSZ)));
    /* page of zeros to back cfgspace */
    u_int8_t zeros[0x1000] __attribute__((aligned(4096)));
    u_int8_t cfgcur[PCIEHW_NDEVS][PCIEHW_CFGSZ] __attribute__((aligned(4096)));
    u_int32_t notify_intr_dest[PCIEHW_NPORTS];   /* notify   intr dest */
    u_int32_t indirect_intr_dest[PCIEHW_NPORTS]; /* indirect intr dest */
    u_int32_t magic;                    /* PCIEHW_MAGIC when initialized */
    u_int32_t version;                  /* PCIEHW_VERSION when initialized */
} pciehw_mem_t;

typedef struct pciehw_s {
    u_int32_t open:1;                   /* hw is in use */
    u_int16_t clients;                  /* number of clients using us */
    pciemgr_params_t params;
    pciehw_mem_t *pciehwmem;
    pciehw_shmem_t *pcieshmem;
} pciehw_t;

struct pcie_stlp_s;
typedef struct pcie_stlp_s pcie_stlp_t;

pciehw_mem_t *pciehw_get_hwmem(void);
pciemgr_params_t *pciehw_get_params(void);
pciehwdev_t *pciehwdev_get(pciehwdevh_t hwdevh);
pciehwdevh_t pciehwdev_geth(const pciehwdev_t *phwdev);
pciehwdev_t *pciehwdev_getvf(pciehwdev_t *phwdev, const int vfidx);
void pciehwdev_get_cfgspace(const pciehwdev_t *phwdev, cfgspace_t *cs);
const char *pciehwdev_get_name(const pciehwdev_t *phwdev);
u_int16_t pciehwdev_get_hostbdf(const pciehwdev_t *phwdev);
pciehwdev_t *pciehwdev_find_by_name(const char *name);

u_int16_t pciehw_hostbdf(const int port, const u_int16_t lbdf);

#include "hdrt.h"
#include "portmap.h"
#include "intr.h"
#include "reset.h"

int pciehw_cfg_init(void);
int pciehw_cfg_finalize(pciehdev_t *pdev);
int pciehw_cfg_finalize_done(pciehwdev_t *phwroot);
void pciehw_cfg_reset(pciehwdev_t *phwdev, const pciehdev_rsttype_t rsttype);

int pciehwdev_cfgrd(pciehwdev_t *phwdev,
                    const u_int16_t offset,
                    const u_int8_t size,
                    u_int32_t *valp);
int pciehwdev_cfgwr(pciehwdev_t *phwdev,
                    const u_int16_t offset,
                    const u_int8_t size,
                    const u_int32_t val);

void pciehw_cfgrd_notify(pciehwdev_t *phwdev,
                         const pcie_stlp_t *stlp,
                         const tlpauxinfo_t *info,
                         const pciehw_spmt_t *spmt);
void pciehw_cfgwr_notify(pciehwdev_t *phwdev,
                         const pcie_stlp_t *stlp,
                         const tlpauxinfo_t *info,
                         const pciehw_spmt_t *spmt);

struct indirect_entry_s;
typedef struct indirect_entry_s indirect_entry_t;
void pciehw_cfgrd_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp);
void pciehw_cfgwr_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp);

int pciehw_bar_init(void);
int pciehw_bars_finalize(pciehdev_t *pdev);
void pciehw_bar_setaddr(pciehwbar_t *phwbar, const u_int64_t addr);
void pciehw_bar_load(pciehwbar_t *phwbar);
void pciehw_bar_enable(pciehwbar_t *phwbar, const int on);
u_int64_t pciehw_bar_getsize(pciehwbar_t *phwbar);
void pciehw_barrd_notify(pciehwdev_t *phwdev,
                         const pcie_stlp_t *stlp,
                         const tlpauxinfo_t *info,
                         const pciehw_spmt_t *spmt);
void pciehw_barwr_notify(pciehwdev_t *phwdev,
                         const pcie_stlp_t *stlp,
                         const tlpauxinfo_t *info,
                         const pciehw_spmt_t *spmt);
void pciehw_barrd_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp);
void pciehw_barwr_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp);
void pciehw_bar_dbg(int argc, char *argv[]);

#define ROMSK_RDONLY 1

void pciehw_romsk_init(void);
int pciehw_romsk_load(pciehwdev_t *phwdev);
void pciehw_romsk_unload(pciehwdev_t *phwdev);
void pciehw_romsk_dbg(int argc, char *argv[]);

#define VFSTRIDE_IDX_DEVCFG     0x0
#define VFSTRIDE_IDX_4K         0x1

void pciehw_vfstride_init(void);

void pciehw_tgt_port_init(void);
void pciehw_tgt_port_skip_notify(const int port, const int on);
void pciehw_tgt_port_single_pnd(const int port, const int on);

void pciehw_itr_port_init(void);

void *pciehw_memset(void *s, int c, size_t n);
void *pciehw_memcpy(void *dst, const void *src, size_t n);

#endif /* __PCIEHW_IMPL_H__ */
