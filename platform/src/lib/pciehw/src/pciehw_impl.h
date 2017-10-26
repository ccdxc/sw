/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHW_IMPL_H__
#define __PCIEHW_IMPL_H__

struct pciehw_s;
typedef struct pciehw_s pciehw_t;

#define PCIEHW_NBUS     16
#define PCIEHW_NDEVS    100
#define PCIEHW_CFGSZ    1024

typedef u_int32_t pciehwdevh_t;

typedef struct pciehwdev_s {
    pciehdev_t *pdev;                   /* associated pciehdev */
    u_int16_t bdf;                      /* bdf of this dev */
    pciehwdevh_t parenth;               /* handle to parent */
    pciehwdevh_t childh;                /* handle to child */
    pciehwdevh_t peerh;                 /* handle to peer */
} pciehwdev_t;

#define PCIEHW_MAGIC    0x706d656d      /* 'pmem' */
#define PCIEHW_VERSION  0x1

typedef struct pciehw_mem_s {
    u_int32_t magic;                    /* PCIEHW_MAGIC when initialized */
    u_int32_t version;
    u_int32_t allocdev;
    pciehwdevh_t rooth;
    pciehwdev_t dev[PCIEHW_NDEVS];
    u_int8_t cfgcur[PCIEHW_NDEVS][PCIEHW_CFGSZ];
    u_int8_t cfgrst[PCIEHW_NDEVS][PCIEHW_CFGSZ];
    u_int8_t cfgmsk[PCIEHW_NDEVS][PCIEHW_CFGSZ];
} pciehw_mem_t;

typedef struct pciehw_s {
    u_int16_t flags;
    u_int16_t clients;
    pciehw_params_t hwparams;
    pciehw_mem_t *pciehwmem;
} pciehw_t;
#define PCIEHWF_OPEN            0x0001  /* hw is open */

int pciehw_openmem(pciehw_t *phw, const int inithw);
void pciehw_closemem(pciehw_t *phw);

#endif /* __PCIEHW_IMPL_H__ */
