/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEHW_PMT_H__
#define __PCIEHW_PMT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#define PMT_COUNT       1024

struct pmt_s; typedef struct pmt_s pmt_t;

void pmt_get(const int pmti, pmt_t *pmt);
void pmt_set(const int pmti, const pmt_t *pmt);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHW_PMT_H__ */
