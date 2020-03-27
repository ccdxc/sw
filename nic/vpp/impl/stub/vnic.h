//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __IMPL_STUB_IMPL_H__
#define __IMPL_STUB_IMPL_H__

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

#ifdef __cplusplus
}
#endif
#endif    // __IMPL_STUB_IMPL_H__
