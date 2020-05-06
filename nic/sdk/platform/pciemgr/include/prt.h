/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEHW_PRT_H__
#define __PCIEHW_PRT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#define PRT_COUNT       4096

union prt_u; typedef union prt_u prt_t;

void prt_get(const int prti, prt_t *prt);
void prt_set(const int prti, const prt_t *prt);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHW_PRT_H__ */
