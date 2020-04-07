//
//  {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
//

#ifndef __VPP_FLOW_LOG_H__
#define __VPP_FLOW_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

int flow_log_notice(const char *fmt, ...);
int flow_log_error(const char *fmt, ...);
int pds_vpp_flow_log_init(void);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_FLOW_LOG_H__
