/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_H__
#define __PCIEPORT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "pcieport_events.h"

#define PCIEPORT_NPORTS         8

int pcieport_open(const int port);
void pcieport_close(const int port);

struct pciehdev_params_s;
typedef struct pciehdev_params_s pciehdev_params_t;

int pcieport_hostconfig(const int port, const pciehdev_params_t *params);
int pcieport_crs_off(const int port);

int pcieport_poll(const int port);
void pcieport_dbg(int argc, char *argv[]);

/*
 * Register convenience macros.
 */
#define PP_(REG) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_ ##REG## _BYTE_ADDRESS)

#define PXC_(REG, pn) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + \
     ((pn) * CAP_PXC_CSR_BYTE_SIZE) + \
     CAP_PP_CSR_PORT_C_ ##REG## _BYTE_ADDRESS)

#define PXP_(REG, pn) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + \
     ((pn) * CAP_PXP_CSR_BYTE_SIZE) + \
     CAP_PP_CSR_PORT_P_ ##REG## _BYTE_ADDRESS)

/* sta_rst flags */
#define STA_RSTF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_RST_ ##REG## _FIELD_MASK)

/* sta_mac flags */
#define STA_MACF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* cfg_mac flags */
#define CFG_MACF_(REG) \
    (CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_ ##REG## _FIELD_MASK)

#ifdef __cplusplus
}
#endif

#endif /* __PCIEPORT_H__ */
