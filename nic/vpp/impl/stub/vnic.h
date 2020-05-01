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
pds_nh_add_tx_hdrs (vlib_buffer_t *b, u16 vnic_nh_hw_id)
{
}

always_inline u8*
pds_vnic_l2_rewrite_info_get (u16 vnic_id, u16 *nh_id)
{
    *nh_id = 0;
    return NULL;
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

static inline int
pds_dst_vnic_info_get (uint16_t lkp_id, uint32_t addr, uint16_t *vnic_id,
                       uint16_t *vnic_nh_hw_id)
{
    return 0;
}

static inline int
pds_src_vnic_info_get (uint16_t lkp_id, uint32_t addr, uint8_t **rewrite,
                       uint16_t *host_lif_hw_id)
{
    return 0;
}

#ifdef __cplusplus
}
#endif
#endif    // __IMPL_STUB_IMPL_VNIC_H__
