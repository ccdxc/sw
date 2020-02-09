//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_VNIC_H__
#define __VPP_IMPL_APULU_VNIC_H__

#include <nic/vpp/infra/utils.h>
#include <nic/apollo/packet/apulu/p4_cpu_hdr.h>
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
        return FALSE;
    }
    *vnic_nh_hw_id = vnic->nh_hw_id;
    *offset = VPP_P4_TO_ARM_HDR_SZ + vnic->l2_encap_len;
    return TRUE;
}

#ifdef __cplusplus
}
#endif
#endif    // __VPP_IMPL_APULU_VNIC_H__
