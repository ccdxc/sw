//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_VNIC_H__
#define __VPP_IMPL_APULU_VNIC_H__

#include <nic/vpp/infra/utils.h>
#include <nic/apollo/packet/apulu/p4_cpu_hdr.h>

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

// get arp packet header offset
static inline int
pds_arp_pkt_offset_get (void *hdr)
{
    u16 vnic_id;
    pds_impl_db_vnic_entry_t *vnic;

    vnic_id = ((p4_rx_cpu_hdr_t *)hdr)->vnic_id;
    vnic = pds_impl_db_vnic_get(vnic_id);
    return (VPP_P4_TO_ARM_HDR_SZ + vnic->l2_encap_len);
}

static inline bool
is_vnic_present (u16 vnic_id)
{
    pds_impl_db_vnic_entry_t *vnic;

    vnic = pds_impl_db_vnic_get(vnic_id);
    if (vnic) {
        return TRUE;
    }
    return FALSE;
}

#ifdef __cplusplus
}
#endif
#endif    // __VPP_IMPL_APULU_VNIC_H__
