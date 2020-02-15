//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __UDS_INTERNAL_H__
#define __UDS_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

void udswrap_process_input(int fd, char *buf, int n);

#ifdef __cplusplus
}
#endif

typedef enum vpp_uds_op_s {
    VPP_UDS_FLOW_DUMP,
    VPP_UDS_NAT_PB_DUMP,
    VPP_UDS_MAX
} vpp_uds_op_t;

// callback function prototype
typedef void (*vpp_uds_cb)(int fd);

void vpp_uds_register_cb(vpp_uds_op_t op, vpp_uds_cb cb);

#endif    // __UDS_INTERNAL_H__
