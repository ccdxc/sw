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

// return FALSE if VNIC is not present,
// else set vnic_nh_hw_id and return TRUE
static inline bool
pds_vnic_data_fill (void *hdr, u16 *vnic_nh_hw_id, u32 *offset)
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

#ifdef __cplusplus
}
#endif
#endif    // __VPP_IMPL_APULU_VNIC_H__
