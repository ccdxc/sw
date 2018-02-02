/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_IMPL_H__
#define __PCIEPORT_IMPL_H__

#include "cap_top_csr_defines.h"
#include "cap_pp_c_hdr.h"

typedef enum pcieportst_e {
    PCIEPORTST_DOWN,
    PCIEPORTST_MACUP,
    PCIEPORTST_LINKUP,
    PCIEPORTST_UP,
    PCIEPORTST_FAULT,

    PCIEPORTST_MAX
} pcieportst_t;

typedef enum pcieportev_e {
    PCIEPORTEV_MACDN,
    PCIEPORTEV_MACUP,
    PCIEPORTEV_LINKDN,
    PCIEPORTEV_LINKUP,
    PCIEPORTEV_BUSCHG,
    PCIEPORTEV_CONFIG,

    PCIEPORTEV_MAX
} pcieportev_t;

struct pcieport_s {
    int port;
    u_int32_t open:1;
    u_int32_t host:1;
    u_int32_t config:1;
    u_int32_t crs:1;
    pcieportst_t state;
    pcieportev_t event;
};
typedef struct pcieport_s pcieport_t;

struct pcieport_info_s {
    u_int32_t init:1;
    pcieport_t pcieport[PCIEPORT_NPORTS];
};
typedef struct pcieport_info_s pcieport_info_t;

extern pcieport_info_t pcieport_info;

void pcieport_config(pcieport_t *p);
void pcieport_fsm(pcieport_t *p, pcieportev_t ev);
void pcieport_gate_open(pcieport_t *p);
void pcieport_set_crs(pcieport_t *p, const int on);
void pcieport_set_serdes_reset(pcieport_t *p, const int on);
void pcieport_set_pcs_reset(pcieport_t *p, const int on);
void pcieport_set_mac_reset(pcieport_t *p, const int on);
void pcieport_set_ltssm_en(pcieport_t *p, const int on);

void pcieport_fsm_dbg(int argc, char *argv[]);

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

#endif /* __PCIEPORT_IMPL_H__ */
