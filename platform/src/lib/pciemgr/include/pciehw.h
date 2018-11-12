/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __PCIEHW_H__
#define __PCIEHW_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "pmt.h"
#include "prt.h"

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;
struct cfgspace_s;
typedef struct cfgspace_s cfgspace_t;
struct pciehdev_params_s;
typedef struct pciehdev_params_s pciehdev_params_t;

int pciehw_open(pciehdev_params_t *params);
void pciehw_close(void);
void pciehw_initialize_topology(const u_int8_t port);
void pciehw_finalize_topology(pciehdev_t *proot);
void pciehw_dev_show(int argc, char *argv[]);
void pciehw_pmt_show(int argc, char *argv[]);
void pciehw_bar_show(void);

#define AXIMSTF_TLP     0x1
#define AXIMSTF_IND     0x2
#define AXIMSTF_RAW     0x4

void pciehw_aximst_show(const unsigned int port,
                        const unsigned int entry,
                        const int flags);

void pciehw_dbg(int argc, char *argv[]);

int pciehw_cfgrd(const u_int8_t port, const u_int16_t bdf,
                 const u_int16_t offset, const u_int8_t size, u_int32_t *valp);
int pciehw_cfgwr(const u_int8_t port, const u_int16_t bdf,
                 const u_int16_t offset, const u_int8_t size, u_int32_t val);

int pciehw_memrd(const u_int8_t port,
                 const u_int64_t addr, const u_int8_t size, u_int64_t *valp);
int pciehw_memwr(const u_int8_t port,
                 const u_int64_t addr, const u_int8_t size, u_int64_t val);

int pciehw_iord(const u_int8_t port,
                const u_int32_t addr, const u_int8_t size, u_int32_t *valp);
int pciehw_iowr(const u_int8_t port,
                const u_int32_t addr, const u_int8_t size, u_int32_t val);

unsigned long long pciehw_barsz(const u_int8_t port,
                                const u_int16_t bdf, const int i);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHW_H__ */
