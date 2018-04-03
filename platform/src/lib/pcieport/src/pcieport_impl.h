/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_IMPL_H__
#define __PCIEPORT_IMPL_H__

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#include "events.h"

typedef enum pcieportst_e {
    PCIEPORTST_OFF,
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

    PCIEPORTEV_MAX
} pcieportev_t;

struct pcieport_s {
    int port;
    int cap_gen;
    int cap_width;
    int cur_gen;
    int cur_width;
    u_int16_t lanemask;
    u_int16_t subvendorid;
    u_int16_t subdeviceid;
    u_int32_t open:1;
    u_int32_t host:1;
    u_int32_t config:1;
    u_int32_t crs:1;
    pcieportst_t state;
    pcieportev_t event;
    char fault_reason[80];
    char last_fault_reason[80];
    u_int64_t hostup;
    u_int64_t phypolllast;
    u_int64_t phypollmax;
    u_int64_t phypollperstn;
    u_int64_t phypollfail;
    u_int64_t gatepolllast;
    u_int64_t gatepollmax;
    u_int64_t markerpolllast;
    u_int64_t markerpollmax;
    u_int64_t axipendpolllast;
    u_int64_t axipendpollmax;
    u_int64_t faults;
};
typedef struct pcieport_s pcieport_t;

struct pcieport_info_s {
    u_int32_t init:1;
    pcieport_t pcieport[PCIEPORT_NPORTS];
};
typedef struct pcieport_info_s pcieport_info_t;

extern pcieport_info_t pcieport_info;

static inline pcieport_info_t *
pcieport_info_get(void)
{
    extern pcieport_info_t pcieport_info;
    return &pcieport_info;
}

int pcieport_config(pcieport_t *p);
void pcieport_fsm(pcieport_t *p, pcieportev_t ev);
int pcieport_tgt_marker_rx_wait(pcieport_t *p);
int pcieport_tgt_axi_pending_wait(pcieport_t *p);
int pcieport_gate_open(pcieport_t *p);
void pcieport_set_crs(pcieport_t *p, const int on);
void pcieport_set_serdes_reset(pcieport_t *p, const int on);
void pcieport_set_pcs_reset(pcieport_t *p, const int on);
void pcieport_set_mac_reset(pcieport_t *p, const int on);
void pcieport_set_ltssm_en(pcieport_t *p, const int on);
void pcieport_set_clock_freq(pcieport_t *p, const u_int32_t freq);
void pcieport_rx_credit_bfr(const int port, const int base, const int limit);
u_int16_t pcieport_get_phystatus(pcieport_t *p);
u_int32_t pcieport_get_sta_rst(pcieport_t *p);
int pcieport_get_perstn(pcieport_t *p);
int pcieport_get_ltssm_st_cnt(pcieport_t *p);
void pcieport_set_ltssm_st_cnt(pcieport_t *p, const int cnt);

void pcieport_fault(pcieport_t *p, const char *fmt, ...)
    __attribute__((format (printf, 2, 3)));

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

/* sta_rst flags */
#define STA_RSTF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_RST_ ##REG## _FIELD_MASK)

/* sta_mac flags */
#define STA_MACF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* cfg_mac flags */
#define CFG_MACF_(REG) \
    (CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_ ##REG## _FIELD_MASK)

#endif /* __PCIEPORT_IMPL_H__ */
