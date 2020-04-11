//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena vnic implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "gen/p4gen/athena/include/p4pd.h"
#include "gen/p4gen/p4/include/ftl.h"

using namespace sdk;

extern "C" {

pds_ret_t
pds_vlan_to_vnic_map_create (pds_vlan_to_vnic_map_spec_t *spec)
{
    p4pd_error_t         p4pd_ret = P4PD_SUCCESS;
    uint16_t             vlan_id = PDS_VLAN_ID_MAX;
    vlan_to_vnic_entry_t entry = { 0 };
    
    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }
    vlan_id = spec->key.vlan_id;
    if (vlan_id >= PDS_VLAN_ID_MAX) {
        PDS_TRACE_ERR("vlan id %u is beyond range", vlan_id);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    entry.set_vnic_id(spec->data.vnic_id);
    entry.set_vnic_type(spec->data.vnic_type);

    p4pd_ret = entry.write(vlan_id);

    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write vlan to vnic table at index %u",
                      vlan_id);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}

pds_ret_t
pds_vlan_to_vnic_map_read (pds_vlan_to_vnic_map_key_t *key,
                           pds_vlan_to_vnic_map_info_t *info)
{
    p4pd_error_t         p4pd_ret = P4PD_SUCCESS;
    uint16_t             vlan_id = PDS_VLAN_ID_MAX;
    vlan_to_vnic_entry_t entry = { 0 };
 
    if (!key || !info) {
        PDS_TRACE_ERR("key/info is null");
        return PDS_RET_INVALID_ARG;
    }
    vlan_id = key->vlan_id;
    if (vlan_id >= PDS_VLAN_ID_MAX) {
        PDS_TRACE_ERR("vlan id %u is beyond range", vlan_id);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    p4pd_ret = entry.read(vlan_id);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read vlan to vnic table at index %u",
                      vlan_id);
        return PDS_RET_HW_READ_ERR;
    }
    info->spec.data.vnic_id = entry.get_vnic_id();
    info->spec.data.vnic_type = (pds_vnic_type_t) entry.get_vnic_type();

    return PDS_RET_OK;
}

pds_ret_t
pds_vlan_to_vnic_map_update (pds_vlan_to_vnic_map_spec_t *spec)
{
    return pds_vlan_to_vnic_map_create(spec);
}

pds_ret_t
pds_vlan_to_vnic_map_delete (pds_vlan_to_vnic_map_key_t *key)
{
    p4pd_error_t         p4pd_ret = P4PD_SUCCESS;
    uint16_t             vlan_id = PDS_VLAN_ID_MAX;
    vlan_to_vnic_entry_t entry = { 0 };
 
    if (!key) {
        PDS_TRACE_ERR("key is null");
        return PDS_RET_INVALID_ARG;
    }
    vlan_id = key->vlan_id;
    if (vlan_id >= PDS_VLAN_ID_MAX) {
        PDS_TRACE_ERR("vlan id %u is beyond range", vlan_id);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    p4pd_ret = entry.write(vlan_id);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to clear vlan to vnic table at index %u",
                      vlan_id);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}

uint64_t
static inline pds_mpls_label_to_hw_id(uint32_t mpls_label)
{
    mpls_label_to_vnic_swkey_t  swkey;
    uint64_t    hw_idx;
    swkey.control_metadata_mpls_label_b20_b4 = (mpls_label >> 4);
    swkey.control_metadata_mpls_label_b3_b0 = (mpls_label & 0x000f);
    hw_idx = p4pd_index_to_hwindex_map(P4TBL_ID_MPLS_LABEL_TO_VNIC, (void*) &swkey);

    PDS_TRACE_ERR("mpls_label: %x: hwidx: %x\n", mpls_label, (uint32_t)hw_idx);
    return hw_idx;
}

pds_ret_t
pds_mpls_label_to_vnic_map_create (pds_mpls_label_to_vnic_map_spec_t *spec)
{
    p4pd_error_t               p4pd_ret = P4PD_SUCCESS;
    uint32_t                   mpls_label = PDS_MPLS_LABEL_MAX;
    mpls_label_to_vnic_entry_t entry = { 0 };
    uint64_t                   hw_idx = 0;
    
    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }

    mpls_label = spec->key.mpls_label;
    if (mpls_label >= PDS_MPLS_LABEL_MAX) {
        PDS_TRACE_ERR("mpls label %u is beyond range", mpls_label);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    entry.set_vnic_type(spec->data.vnic_type);
    entry.set_vnic_id(spec->data.vnic_id);

    hw_idx = pds_mpls_label_to_hw_id(mpls_label);
    p4pd_ret = entry.write(hw_idx);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write mpls label to vnic table at index %u",
                      mpls_label);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}

pds_ret_t
pds_mpls_label_to_vnic_map_read (pds_mpls_label_to_vnic_map_key_t *key,
                                 pds_mpls_label_to_vnic_map_info_t *info)
{
    p4pd_error_t               p4pd_ret = P4PD_SUCCESS;
    uint32_t                   mpls_label = PDS_MPLS_LABEL_MAX;
    mpls_label_to_vnic_entry_t entry = { 0 };
    uint64_t                   hw_idx = 0;
 
    if (!key || !info) {
        PDS_TRACE_ERR("key/info is null");
        return PDS_RET_INVALID_ARG;
    }
    mpls_label = key->mpls_label;
    if (mpls_label >= PDS_MPLS_LABEL_MAX) {
        PDS_TRACE_ERR("mpls label %u is beyond range", mpls_label);
        return PDS_RET_INVALID_ARG;
    }
    hw_idx = pds_mpls_label_to_hw_id(mpls_label);

    entry.clear();
    p4pd_ret = entry.read(hw_idx);

    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read mpls label to vnic table at index %u",
                      mpls_label);
        return PDS_RET_HW_READ_ERR;
    }
    info->spec.data.vnic_id = entry.get_vnic_id();
    info->spec.data.vnic_type = (pds_vnic_type_t) entry.get_vnic_type();

    return PDS_RET_OK;
}

pds_ret_t
pds_mpls_label_to_vnic_map_update (pds_mpls_label_to_vnic_map_spec_t *spec)
{
    return pds_mpls_label_to_vnic_map_create(spec);
}

pds_ret_t
pds_mpls_label_to_vnic_map_delete (pds_mpls_label_to_vnic_map_key_t *key)
{
    p4pd_error_t                p4pd_ret = P4PD_SUCCESS;
    uint32_t                    mpls_label = PDS_MPLS_LABEL_MAX;
    mpls_label_to_vnic_entry_t  entry = { 0 };
    uint64_t                    hw_idx = 0;
 
    if (!key) {
        PDS_TRACE_ERR("key is null");
        return PDS_RET_INVALID_ARG;
    }
    mpls_label = key->mpls_label;
    if (mpls_label >= PDS_MPLS_LABEL_MAX) {
        PDS_TRACE_ERR("mpls label %u is beyond range", mpls_label);
        return PDS_RET_INVALID_ARG;
    }

    hw_idx = pds_mpls_label_to_hw_id(mpls_label);
     
    entry.clear();
    p4pd_ret = entry.write(hw_idx);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to clear mpls label to vnic table at index %u",
                      mpls_label);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}

}
