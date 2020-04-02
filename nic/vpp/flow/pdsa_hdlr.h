//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PDSA_HDLR_H__
#define __VPP_FLOW_PDSA_HDLR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PDS_FLOW_PROTO_START,
    PDS_FLOW_PROTO_TCP = PDS_FLOW_PROTO_START,
    PDS_FLOW_PROTO_UDP,
    PDS_FLOW_PROTO_ICMP,
    PDS_FLOW_PROTO_OTHER,
    PDS_FLOW_PROTO_END,
} pds_flow_protocol;

// function prototypes

// pdsa_hdlr.cc
void pdsa_flow_hdlr_init(void);

// pdsa_vpp_hdlr.c
void pds_flow_cfg_set(uint8_t con_track_en,
                      uint32_t tcp_syn_timeout,
                      uint32_t tcp_half_close_timeout,
                      uint32_t tcp_close_timeout,
                      const uint32_t *flow_idle_timeout,
                      const uint32_t *flow_drop_timeout);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_FLOW_PDSA_HDLR_H__
