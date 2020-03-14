//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PDSA_UDS_HDLR_H__
#define __VPP_FLOW_PDSA_UDS_HDLR_H__

#ifdef __cplusplus
extern "C" {
#endif

// function prototypes

// pdsa_uds_hdlr.cc
void pds_flow_dump_init(void);

// pdsa_vpp_uds_hdlr.c
void * pds_flow_get_table4(void);
void * pds_flow_get_table6_or_l2(void);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_FLOW_PDSA_UDS_HDLR_H__
