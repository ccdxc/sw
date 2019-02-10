/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_IMPL_H__
#define __PCIEPORT_IMPL_H__

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#include "events.h"

int pcieport_onetime_init(pcieport_info_t *pi, pciemgr_initmode_t initmode);
int pcieport_onetime_portinit(pcieport_t *p);
void pcieport_intr_init(pcieport_t *p);
int pcieport_config(pcieport_t *p);
void pcieport_fsm(pcieport_t *p, pcieportev_t ev);
void pcieport_fsm_init(pcieport_t *p, pcieportst_t st);
int pcieport_tgt_marker_rx_wait(pcieport_t *p);
int pcieport_tgt_axi_pending_wait(pcieport_t *p);
int pcieport_gate_open(pcieport_t *p);
void pcieport_set_crs(pcieport_t *p, const int on);
void pcieport_set_serdes_reset(pcieport_t *p, const int on);
void pcieport_set_pcs_reset(pcieport_t *p, const int on);
void pcieport_set_mac_reset(pcieport_t *p, const int on);
int pcieport_get_ltssm_en(pcieport_t *p);
void pcieport_set_ltssm_en(pcieport_t *p, const int on);
void pcieport_set_aer_common_en(pcieport_t *p, const int on);
void pcieport_set_clock_freq(pcieport_t *p, const u_int32_t freq);
void pcieport_rx_credit_bfr(const int port, const int base, const int limit);
void pcieport_pcsd_control_sris(const int sris_en);
u_int16_t pcieport_get_phystatus(pcieport_t *p);
u_int32_t pcieport_get_sta_rst(pcieport_t *p);
int pcieport_get_perstn(pcieport_t *p);
int pcieport_get_ltssm_st_cnt(pcieport_t *p);
void pcieport_set_ltssm_st_cnt(pcieport_t *p, const int cnt);
int pcieport_serdes_init(void);

void pcieport_fault(pcieport_t *p, const char *fmt, ...)
    __attribute__((format (printf, 2, 3)));

void pcieport_fsm_dbg(int argc, char *argv[]);

#endif /* __PCIEPORT_IMPL_H__ */
