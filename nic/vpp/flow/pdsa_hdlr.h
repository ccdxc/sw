//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PDSA_HDLR_H__
#define __VPP_FLOW_PDSA_HDLR_H__

#ifdef __cplusplus
extern "C" {
#endif

// function prototypes

// pdsa_hdlr.cc
void pdsa_flow_hdlr_init(void);

// pdsa_vpp_hdlr.c
void pds_flow_idle_timeout_get(uint32_t *flow_idle_timeout, size_t sz);
void pds_flow_idle_timeout_set(const uint32_t *flow_idle_timeout, size_t sz);
void pds_flow_cfg_init(void);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_FLOW_PDSA_HDLR_H__
