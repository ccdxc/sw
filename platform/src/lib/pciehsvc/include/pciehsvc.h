/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHSVC_H__
#define __PCIEHSVC_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef struct pciehsvc_params_s {
    u_int16_t unused;
} pciehsvc_params_t;

int pciehsvc_open(pciehsvc_params_t *svcparams);
void pciehsvc_close(void);

int pciehsvc_cfgrd(const u_int16_t bdf,
                   const u_int16_t offset,
                   const u_int8_t size,
                   u_int32_t *valp);
int pciehsvc_cfgwr(const u_int16_t bdf,
                   const u_int16_t offset,
                   const u_int8_t size,
                   u_int32_t val);

int pciehsvc_memrd(const u_int64_t addr,
                   const u_int8_t size, 
                   u_int64_t *valp);
int pciehsvc_memwr(const u_int64_t addr,
                   const u_int8_t size,
                   u_int64_t val);

int pciehsvc_iord(const u_int32_t addr,
                  const u_int8_t size,
                  u_int32_t *valp);
int pciehsvc_iowr(const u_int32_t addr,
                  const u_int8_t size,
                  u_int32_t val);

pciehsvc_params_t *pciehsvc_get_params(void);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHSVC_H__ */
