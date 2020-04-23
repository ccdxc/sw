//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include "impl_db.h"
#include "pdsa_impl_db_hdlr.h"
#include "mapping.h"

bool
pds_session_active_on_vnic_get (uint16_t vnic_id, uint32_t *sess_count)
{
    pds_impl_db_vnic_entry_t *vnic_info = NULL;

    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if (vnic_info == NULL) {
        return false;
    }

    *sess_count = vnic_info->active_ses_count;

    return true;
}

int
pds_dst_vnic_info_get (uint16_t lkp_id, uint32_t addr, uint16_t *vnic_id,
                       uint16_t *vnic_nh_hw_id)
{
    pds_impl_db_vnic_entry_t *vnic;
    pds_impl_db_subnet_entry_t *subnet;
    int ret;

    subnet = pds_impl_db_subnet_get(lkp_id);
    if (subnet == NULL) {
        return -1;
    }

    ret = pds_local_mapping_vnic_id_get(subnet->vpc_id, addr, vnic_id);
    if (ret != 0) {
        return ret;
    }

    vnic = pds_impl_db_vnic_get(*vnic_id);
    if (vnic == NULL) {
        return -1;
    }

    *vnic_nh_hw_id = vnic->nh_hw_id;
    return 0;
}

int
pds_src_vnic_info_get (uint16_t lkp_id, uint32_t addr, mac_addr_t mac_addr,
                       uint16_t *host_lif_hw_id)
{
    pds_impl_db_vnic_entry_t *vnic;
    pds_impl_db_subnet_entry_t *subnet;
    uint16_t vnic_id;
    int ret;

    subnet = pds_impl_db_subnet_get(lkp_id);
    if (subnet == NULL) {
        return -1;
    }

    ret = pds_local_mapping_vnic_id_get(subnet->vpc_id, addr, &vnic_id);
    if (ret != 0) {
        return ret;
    }

    vnic = pds_impl_db_vnic_get(vnic_id);
    if (vnic == NULL) {
        return -1;
    }

    memcpy(mac_addr, vnic->mac, ETH_ADDR_LEN);
    *host_lif_hw_id = vnic->host_lif_hw_id;
    return 0;
}


