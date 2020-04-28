//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __IMPL_STUB_IMPL_VNIC_H__
#define __IMPL_STUB_IMPL_VNIC_H__

#include <nic/vpp/infra/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

always_inline void
pds_vnic_add_tx_hdrs (vlib_buffer_t *b, u16 vnic_nh_hw_id)
{
}

always_inline int
pds_vnic_l2_rewrite_info_get (u16 vnic_id, u8 **src_mac, u8 **dst_mac,
                              u8 *vnic_not_found, u8 *subnet_not_found)
{
    *vnic_not_found = *subnet_not_found = 1;
    return -1;
}

always_inline int
pds_vnic_subnet_get (u16 vnic_id, u16 *subnet_hw_id)
{
    return -1;
}

always_inline int
pds_vnic_flow_log_en_get (u16 vnic_id, int *enable)
{
    return -1;
}

always_inline int
pds_vnic_active_sessions_decrement (uint16_t vnic_id) {
    return -1;
}

#ifdef __cplusplus
}
#endif
#endif    // __IMPL_STUB_IMPL_VNIC_H__
