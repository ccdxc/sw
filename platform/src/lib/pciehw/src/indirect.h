/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __INDIRECT_H__
#define __INDIRECT_H__

#include "tlpauxinfo.h"

typedef enum {
#define PCIEIND_REASON_DEF(NAME, VAL)               \
    PCIEIND_REASON_##NAME = VAL,
#include "indirect_reason.h"
    PCIEIND_REASON_MAX
} pcieind_reason_t;

/*
 * Completion Status field values
 * PCIe 4.0, Table 2-34.
 */
typedef enum {
    PCIECPL_SC          = 0x0,
    PCIECPL_UR          = 0x1,
    PCIECPL_CRS         = 0x2,
    PCIECPL_CA          = 0x4,
} pciecpl_t;

#define INDIRECT_TLPSZ          64

typedef struct indirect_entry_s {
    u_int32_t port;
    pciecpl_t cpl;
    u_int32_t data[4];
    u_int8_t rtlp[INDIRECT_TLPSZ];
    tlpauxinfo_t info;
} indirect_entry_t;

struct pciehw_s;
typedef struct pciehw_s pciehw_t;

int pciehw_indirect_init(pciehw_t *phw);
int pciehw_indirect_poll(pciehw_t *phw);
void pciehw_indirect_dbg(int argc, char *argv[]);
void pciehw_indirect_complete(indirect_entry_t *ientry);

#endif /* __INDIRECT_H__ */
