/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHW_H__
#define __PCIEHW_H__

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;
struct cfgspace_s;
typedef struct cfgspace_s cfgspace_t;

typedef struct pciehw_params_s {
    u_int32_t inithw:1;         /* initialize hw */
    u_int32_t fake_bios_scan:1; /* assign bus #'s on finalize */
} pciehw_params_t;

int pciehw_open(pciehw_params_t *hwparams);
void pciehw_close(void);
void pciehw_initialize_topology(void);
void pciehw_finalize_topology(pciehdev_t *proot);

int pciehw_cfgrd(const u_int16_t bdf,
                 const u_int16_t offset, const u_int8_t size, u_int32_t *valp);
int pciehw_cfgwr(const u_int16_t bdf,
                 const u_int16_t offset, const u_int8_t size, u_int32_t val);

int pciehw_memrd(const u_int64_t addr, const u_int8_t size, u_int64_t *valp);
int pciehw_memwr(const u_int64_t addr, const u_int8_t size, u_int64_t val);

int pciehw_iord(const u_int32_t addr, const u_int8_t size, u_int32_t *valp);
int pciehw_iowr(const u_int32_t addr, const u_int8_t size, u_int32_t val);

#endif /* __PCIEHW_H__ */
