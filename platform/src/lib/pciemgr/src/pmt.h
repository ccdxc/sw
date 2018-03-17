/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __PMT_H__
#define __PMT_H__

void pmt_init(void);
int pmt_alloc(const int n);
void pmt_free(const int pmtb, const int pmtc);
void pmt_get(const int pmti, pmt_t *p);
void pmt_set(const int pmti, const pmt_t *p);

struct pciehw_s;
typedef struct pciehw_s pciehw_t;
struct pciehwdev_s;
typedef struct pciehwdev_s pciehwdev_t;
struct pciehwbar_s;
typedef struct pciehwbar_s pciehwbar_t;

int pciehw_pmt_load_cfg(pciehwdev_t *phwdev);
void pciehw_pmt_setaddr(pciehwbar_t *phwbar, const u_int64_t addr);
void pciehw_pmt_load_bar(pciehwbar_t *phwbar);
void pciehw_pmt_enable_bar(pciehwbar_t *phwbar, const int on);
void pciehw_pmt_dbg(int argc, char *argv[]);

#endif /* __PMT_H__ */
