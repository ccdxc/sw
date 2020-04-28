//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_VNIC_H__
#define __VPP_IMPL_APULU_VNIC_H__

#include <nic/vpp/infra/utils.h>
#include <nic/apollo/packet/apulu/p4_cpu_hdr.h>
#include <nic/vpp/impl/apulu/p4_cpu_hdr_utils.h>
#include <nic/apollo/p4/include/apulu_defines.h>
#include <impl_db.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int
pds_ingress_bd_id_get (void *hdr)
{
    return (((p4_rx_cpu_hdr_t *)hdr)->ingress_bd_id);
}

static inline int
pds_vnic_id_get (void *hdr)
{
    return (((p4_rx_cpu_hdr_t *)hdr)->vnic_id);
}

// return -1 if VNIC is not present,
// else set vnic_nh_hw_id and return 0
static inline int
pds_vnic_nexthop_get (void *hdr, u16 *vnic_nh_hw_id, u32 *offset)
{
    u16 vnic_id;
    pds_impl_db_vnic_entry_t *vnic;

    vnic_id = pds_vnic_id_get(hdr);
    vnic = pds_impl_db_vnic_get(vnic_id);
    if (PREDICT_FALSE(vnic == NULL)) {
        return -1;
    }
    *vnic_nh_hw_id = vnic->nh_hw_id;
    *offset = VPP_P4_TO_ARM_HDR_SZ + vnic->l2_encap_len;
    return 0;
}

static inline int
pds_vnic_subnet_get (u16 vnic_id, u16 *subnet_hw_id)
{
    pds_impl_db_vnic_entry_t *vnic_info = NULL;

    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if (PREDICT_FALSE(vnic_info == NULL)) {
        return -1;
    }
    *subnet_hw_id = vnic_info->subnet_hw_id;
    return 0;
}

always_inline int
pds_vnic_flow_log_en_get (u16 vnic_id, int *enable)
{
    pds_impl_db_vnic_entry_t *vnic_info = NULL;

    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if (PREDICT_FALSE(vnic_info == NULL)) {
        return -1;
    }
    *enable = vnic_info->flow_log_en;
    return 0;
}

always_inline int
pds_vnic_active_sessions_decrement (uint16_t vnic_id) {
    pds_impl_db_vnic_entry_t *vnic_info = NULL;

    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if (PREDICT_FALSE(vnic_info == NULL)) {
        return -1;
    }
    vnic_info->active_ses_count--;
    return 0;
}

always_inline void
pds_vnic_add_tx_hdrs (vlib_buffer_t *b, u16 vnic_nh_hw_id)
{
    p4_tx_cpu_hdr_t *tx;

    tx = vlib_buffer_get_current(b);
    tx->lif_flags = 0;
    tx->nexthop_valid = 1;
    tx->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    tx->nexthop_id = clib_host_to_net_u16(vnic_nh_hw_id);
    tx->lif_flags = clib_host_to_net_u16(tx->lif_flags);
}

always_inline int
pds_vnic_l2_rewrite_info_get (u16 vnic_id, u8 **src_mac, u8 **dst_mac,
                              u8 *vnic_not_found, u8 *subnet_not_found)
{
    pds_impl_db_vnic_entry_t *vnic_info = NULL;
    pds_impl_db_subnet_entry_t *subnet_info = NULL;

    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if(vnic_info == NULL) {
        *vnic_not_found = 1;
        return -1;
    }
    subnet_info = pds_impl_db_subnet_get(vnic_info->subnet_hw_id);
    if(subnet_info == NULL) {
        *subnet_not_found = 1;
        return -1;
    }
    *src_mac = subnet_info->mac;
    *dst_mac = vnic_info->mac;
    return 0;
}
#ifdef __cplusplus
}
#endif
#endif    // __VPP_IMPL_APULU_VNIC_H__
