/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#include "tlpauxinfo.h"

#define NOTIFY_TLPSZ            48

typedef struct notify_entry_s {
    u_int8_t rtlp[NOTIFY_TLPSZ];
    tlpauxinfo_t info;
} notify_entry_t;

#define NOTIFY_ENTRYSZ          sizeof(notify_entry_t)
#define NOTIFY_NENTRIES         0x4000  /* power of 2 */
#define NOTIFYSZ                (NOTIFY_NENTRIES * NOTIFY_ENTRYSZ)

struct pciehw_s;
typedef struct pciehw_s pciehw_t;

int pciehw_notify_init(pciehw_t *phw);
int pciehw_notify_poll(pciehw_t *phw);
void pciehw_notify_dbg(int argc, char *argv[]);

#endif /* __NOTIFY_H__ */
