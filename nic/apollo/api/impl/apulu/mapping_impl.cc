//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of mapping
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"
#include "nic/apollo/api/impl/apulu/mapping_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"

using sdk::table::sdk_table_api_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL - mapping entry datapath implementation
/// \ingroup PDS_MAPPING
/// @{

mapping_impl *
mapping_impl::factory(pds_mapping_spec_t *spec) {
    mapping_impl    *impl;

    impl = mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) mapping_impl();
    return impl;
}

void
mapping_impl::soft_delete(mapping_impl *impl) {
    impl->~mapping_impl();
    mapping_impl_db()->free(impl);
}

void
mapping_impl::destroy(mapping_impl *impl) {
    mapping_impl::soft_delete(impl);
}

impl_base *
mapping_impl::clone(void) {
    mapping_impl *cloned_impl;

    cloned_impl = mapping_impl_db()->alloc();
    new (cloned_impl) mapping_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
mapping_impl::free(mapping_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

mapping_impl *
mapping_impl::build(pds_mapping_key_t *key, mapping_entry *mapping) {
    sdk_ret_t ret;
    bool is_local;
    vpc_entry *vpc;
    mapping_impl *impl;
    ip_addr_t public_ip;
    subnet_entry *subnet;
    device_entry *device;
    pds_vpc_key_t vpc_key;
    p4pd_error_t p4pd_ret;
    nat_actiondata_t nat_data;
    mapping_swkey_t mapping_key;
    bool public_ip_valid = false;
    sdk_table_api_params_t tparams;
    mapping_appdata_t mapping_data;
    local_mapping_swkey_t local_mapping_key;
    local_mapping_appdata_t local_mapping_data;
    local_mapping_appdata_t pub_ip_local_mapping_data;

    device = device_find();
    if (key->type == PDS_MAPPING_TYPE_L2) {
        subnet = subnet_find(&key->subnet);
        if (unlikely(subnet == NULL)) {
            return NULL;
        }
        vpc_key = subnet->vpc();
        vpc = vpc_find(&vpc_key);
        if (unlikely(vpc == NULL)) {
            return NULL;
        }
    } else {
        vpc = vpc_find(&key->vpc);
        if (unlikely(vpc == NULL)) {
            return NULL;
        }
    }

    // read mapping table and infer if this is also a local mapping
    if (key->type == PDS_MAPPING_TYPE_L2) {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key,
                                       ((subnet_impl *)subnet->impl())->hw_id(),
                                       key->mac_addr);
    } else {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key,
                                       vpc->hw_id(), &key->ip_addr);
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, &mapping_data,
                                   0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return NULL;
    }

    is_local = (mapping_data.nexthop_type == NEXTHOP_TYPE_NEXTHOP);

    // read local mapping table, this also tells us if there are public
    // IP mappings
    if (is_local) {
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                             &key->ip_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                       &local_mapping_data, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_mapping_tbl()->get(&tparams);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        public_ip_valid = (local_mapping_data.xlate_id !=
                               PDS_IMPL_RSVD_NAT_HW_ID);
    }

    // if public mapping exists, NAT table provides the public IP
    if (public_ip_valid) {
        p4pd_ret = p4pd_global_entry_read(P4TBL_ID_NAT,
                                          local_mapping_data.xlate_id,
                                          NULL, NULL, &nat_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            return NULL;
        }

        // read public ip from NAT data
        P4_IPADDR_TO_IPADDR(nat_data.nat_action.ip, public_ip, key->ip_addr.af);

        // read local mapping for pub ip to get it's xlate index
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                             &public_ip);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                       &pub_ip_local_mapping_data, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_mapping_tbl()->get(&tparams);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
    }

    // all required tables are read, allocate impl object and build it
    impl = mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) mapping_impl();

    mapping->set_local(is_local);
    if (is_local) {
        impl->vnic_hw_id_ = local_mapping_data.vnic_id;
    } else {
        impl->vnic_hw_id_ = PDS_IMPL_RSVD_VNIC_HW_ID;
    }
    impl->vpc_hw_id_ = vpc->hw_id();
    impl->subnet_hw_id_ = mapping_data.egress_bd_id;
    if (public_ip_valid) {
        mapping->set_public_ip(&public_ip);
        impl->to_public_ip_nat_idx_ = local_mapping_data.xlate_id;
        impl->to_overlay_ip_nat_idx_ = pub_ip_local_mapping_data.xlate_id;
    }
    return impl;
}

sdk_ret_t
mapping_impl::reserve_local_mapping_resources_(mapping_entry *mapping,
                                               vpc_entry *vpc,
                                               pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vnic_entry *vnic;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;

    vnic = vnic_find(&spec->vnic);
    // reserve an entry in LOCAL_MAPPING table for overlay IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                         &spec->key.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->reserve(&tparams);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_MAPPING table "
                      "for mapping (vpc %u, ip %s), vnic %u, err %u",
                      spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                      vnic->key().id, ret);
        return ret;
    }
    local_mapping_overlay_ip_hdl_ = tparams.handle;

    // reserve an entry in MAPPING table for overlay IP
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                   &spec->key.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for "
                      "mapping (vpc %u, ip %s), vnic %u, err %u",
                      spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                      vnic->key().id, ret);
        goto error;
    }
    mapping_hdl_ = tparams.handle;

    PDS_TRACE_DEBUG("Rsvd LOCAL_MAPPING handle 0x%lx, MAPPING handle 0x%lx",
                    local_mapping_overlay_ip_hdl_, mapping_hdl_);

    // check if this mapping has public IP
    if (!spec->public_ip_valid) {
        return SDK_RET_OK;
    }

    // reserve an entry in LOCAL_MAPPING table for public IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                         &spec->public_ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_MAPPING table "
                      "for public IP %s of mapping (vpc %u, ip %s), "
                      "vnic %u, err %u", ipaddr2str(&spec->public_ip),
                      spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                      vnic->key().id, ret);
        goto error;
    }
    local_mapping_public_ip_hdl_ = tparams.handle;

    // reserve an entry in MAPING table for public IP
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                   &spec->public_ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for public IP "
                      "%s of mapping (vpc %u, ip %s), vnic %u, err %u",
                      ipaddr2str(&spec->public_ip),
                      spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                      vnic->key().id, ret);
        goto error;
    }
    mapping_public_ip_hdl_ = tparams.handle;

    PDS_TRACE_DEBUG("Rsvd public IP hdl 0x%lx in LOCAL_MAPPING table, "
                    "public IP handle in 0x%lx in MAPPING table",
                    local_mapping_public_ip_hdl_, mapping_public_ip_hdl_);

    // reserve an entry for overlay IP to public IP xlation in NAT table
    ret = apulu_impl_db()->nat_idxr()->alloc(&to_public_ip_nat_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry for public IP %s in NAT table "
                      "for mapping (vpc %u, ip %s), vnic %u, err %u",
                      ipaddr2str(&spec->public_ip),
                      spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                      vnic->key().id, ret);
        goto error;
    }

    // reserve an entry for public IP to overlay IP xlation in NAT table
    ret = apulu_impl_db()->nat_idxr()->alloc(&to_overlay_ip_nat_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry for overlay IP in NAT table for "
                      "mapping (vpc %u, ip %s), vnic %u, err %u",
                      spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                      vnic->key().id, ret);
        goto error;
    }

    PDS_TRACE_DEBUG("Rsvd public IP NAT handle 0x%lx, overlay IP NAT "
                    "handle 0x%lx", to_public_ip_nat_idx_,
                    to_overlay_ip_nat_idx_);
    return SDK_RET_OK;

error:

    // TODO: release all allocated resources
    return ret;
}

sdk_ret_t
mapping_impl::reserve_remote_mapping_resources_(mapping_entry *mapping,
                                                vpc_entry *vpc,
                                                subnet_entry *subnet,
                                                pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;

    // reserve an entry in MAPPING table
    if (spec->key.type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                       &spec->key.ip_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    } else if (spec->key.type == PDS_MAPPING_TYPE_L2) {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key,
                                       ((subnet_impl *)subnet->impl())->hw_id(),
                                       spec->key.mac_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for "
                      "mapping %s, err %u", mapping->key2str().c_str(), ret);
        return ret;
    }
    mapping_hdl_ = tparams.handle;
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::reserve_resources(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    vpc_entry *vpc;
    subnet_entry *subnet;
    pds_mapping_spec_t *spec;
    mapping_entry *mapping = (mapping_entry *)api_obj;

    spec = &obj_ctxt->api_params->mapping_spec;
    if (spec->is_local) {
        PDS_TRACE_DEBUG("Reserving resources for mapping %s, local %u, "
                        "subnet %u, vnic %u, pub_ip_valid %u, pub_ip %s",
                        mapping->key2str().c_str(), spec->is_local,
                        spec->subnet.id,
                        spec->vnic.id, spec->public_ip_valid,
                        ipaddr2str(&spec->public_ip));
        // reserve all local IP mapping resources
        vpc = vpc_find(&spec->key.vpc);
        return reserve_local_mapping_resources_(mapping, vpc, spec);
    }

    // reserve all remote MAC or IP mapping resources
    PDS_TRACE_DEBUG("Reserving resources for mapping %s local %u, subnet %u, "
                    "nh type %s-%u", mapping->key2str().c_str(), spec->is_local,
                    spec->subnet.id,
                    (spec->nh_type == PDS_NH_TYPE_OVERLAY) ? "tep" : "nh group",
                    (spec->nh_type == PDS_NH_TYPE_OVERLAY) ?
                        spec->tep.id : spec->nh_group.id);
    if (spec->key.type == PDS_MAPPING_TYPE_L2) {
        vpc = NULL;
        subnet = subnet_find(&spec->key.subnet);
    } else {
        vpc = vpc_find(&spec->key.vpc);
        subnet = NULL;
    }
    return reserve_remote_mapping_resources_(mapping, vpc, subnet, spec);
}

sdk_ret_t
mapping_impl::nuke_resources(api_base *api_obj) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;
    mapping_entry *mapping = (mapping_entry *)api_obj;

    // remove mapping entry for overlay IP or MAC
    if (mapping->key().type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc_hw_id_,
                                       &mapping->key().ip_addr);
    } else {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key, subnet_hw_id_,
                                       mapping->key().mac_addr)
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->remove(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to remove entry in MAPPING table for %s, err %u",
                      mapping->key2str().c_str(), ret);
        return ret;
    }

    if (mapping->is_local() == false) {
        // nothing else to do if there is no local mapping
        return ret;
    }

    // delete local mapping entry for overlay ip
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc_hw_id_,
                                         &mapping->key().ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->remove(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to remove entry in LOCAL_MAPPING table %s, "
                      "err %u", mapping->key2str().c_str(), ret);
        return ret;
    }

    // nothing else to do if public ip is not configured
    if (!mapping->is_public_ip_valid()) {
        return SDK_RET_OK;
    }

    // remove public ip mapping
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc_hw_id_,
                                   &mapping->public_ip());
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL,
                                   0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->remove(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to remove entry in MAPPING table for %s, err %u",
                      mapping->key2str().c_str(), ret);
        return ret;
    }

    // remove public ip local mapping
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc_hw_id_,
                                         &mapping->public_ip());
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                   NULL, 0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->remove(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to remove entry in LOCAL_MAPPING table for %s, "
                      "err %u", mapping->key2str().c_str(), ret);
        return ret;
    }

    // free NAT entries
    if (to_public_ip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
        apulu_impl_db()->nat_idxr()->free(to_public_ip_nat_idx_);
    }

    if (to_overlay_ip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
        apulu_impl_db()->nat_idxr()->free(to_overlay_ip_nat_idx_);
    }

    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_local_mapping_resources_(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };

    if (local_mapping_overlay_ip_hdl_.valid()) {
        tparams.handle = local_mapping_overlay_ip_hdl_;
        mapping_impl_db()->local_mapping_tbl()->release(&tparams);
    }
    if (mapping_hdl_.valid()) {
        tparams.handle = mapping_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&tparams);
    }
    if (local_mapping_public_ip_hdl_.valid()) {
        tparams.handle = local_mapping_public_ip_hdl_;
        mapping_impl_db()->local_mapping_tbl()->release(&tparams);
    }
    if (mapping_public_ip_hdl_.valid()) {
        tparams.handle = mapping_public_ip_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&tparams);
    }
    if (to_public_ip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
        apulu_impl_db()->nat_idxr()->free(to_public_ip_nat_idx_);
    }
    if (to_overlay_ip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
        apulu_impl_db()->nat_idxr()->free(to_overlay_ip_nat_idx_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_remote_mapping_resources_(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };

    if (mapping_hdl_.valid()) {
        tparams.handle = mapping_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&tparams);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_resources(api_base *api_obj) {
    mapping_entry *mapping = (mapping_entry *)api_obj;
    if (mapping->is_local()) {
        return release_local_mapping_resources_(api_obj);
    }
    return release_remote_mapping_resources_(api_obj);
}

sdk_ret_t
mapping_impl::add_nat_entries_(mapping_entry *mapping,
                               pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    nat_actiondata_t nat_data = { 0 };

    // add private to public IP xlation NAT entry
    PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->public_ip, 0);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NAT, to_public_ip_nat_idx_,
                                       NULL, NULL, &nat_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NAT table entry %u for overlay IP "
                      "to public IP xlation for mapping %s)",
                      to_public_ip_nat_idx_, mapping->key2str().c_str());
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // add public to private/overlay IP xlation NAT entry
    PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->key.ip_addr, 0);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NAT, to_overlay_ip_nat_idx_,
                                       NULL, NULL, &nat_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NAT table entry %u for public IP "
                      "to overlay IP xlation for mapping (vpc %u, ip %s)",
                      to_overlay_ip_nat_idx_, mapping->key2str().c_str());
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    pds_mapping_spec_t *spec;

    spec = &obj_ctxt->api_params->mapping_spec;
    if (spec->is_local && spec->public_ip_valid) {
        return add_nat_entries_((mapping_entry *)api_obj, spec);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                        api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::add_remote_mapping_entries_(vpc_entry *vpc, subnet_entry *subnet,
                                          pds_mapping_spec_t *spec) {
    tep_impl *tep;
    mapping_swkey_t mapping_key;
    nexthop_group_impl *nh_group;
    mapping_appdata_t mapping_data;
    sdk_table_api_params_t tparams;

    // add entry to MAPPING table for overlay IP or MAC
    if (spec->key.type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                       &spec->key.ip_addr);
    } else {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key,
                                       ((subnet_impl *)subnet->impl())->hw_id(),
                                       spec->key.mac_addr);
    }
    memset(&mapping_data, 0, sizeof(mapping_data));
    mapping_data.nexthop_valid = TRUE;
    switch (spec->nh_type) {
        case PDS_NH_TYPE_OVERLAY:
            tep = (tep_impl *)tep_find(&spec->tep)->impl();
            mapping_data.nexthop_type = NEXTHOP_TYPE_TUNNEL;
            mapping_data.nexthop_id = tep->hw_id();
            break;

        case PDS_NH_TYPE_OVERLAY_ECMP:
            nh_group =
                (nexthop_group_impl *)nexthop_group_find(&spec->nh_group)->impl();
            mapping_data.nexthop_type = NEXTHOP_TYPE_ECMP;
            mapping_data.nexthop_id = nh_group->hw_id();
            break;

        default:
            return SDK_RET_INVALID_ARG;
            break;
    }
    mapping_data.egress_bd_id = ((subnet_impl *)subnet->impl())->hw_id();
    sdk::lib::memrev(mapping_data.dmaci, spec->overlay_mac, ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key,
                                   NULL, &mapping_data,
                                   MAPPING_MAPPING_INFO_ID, mapping_hdl_);
    return mapping_impl_db()->mapping_tbl()->insert(&tparams);
}

sdk_ret_t
mapping_impl::add_local_mapping_entries_(vpc_entry *vpc,
                                         pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vnic_entry *vnic;
    subnet_entry *subnet;
    vnic_impl *vnic_impl_obj;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;
    local_mapping_appdata_t local_mapping_data;

    subnet = subnet_find(&spec->subnet);
    vnic = vnic_find(&spec->vnic);
    vnic_impl_obj = (vnic_impl *)vnic->impl();
    // add entry to LOCAL_MAPPING table for overlay IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                         &spec->key.ip_addr);
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_mapping_data,
                                           vnic_impl_obj->hw_id(),
                                           to_public_ip_nat_idx_);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                   &local_mapping_data,
                                   LOCAL_MAPPING_LOCAL_MAPPING_INFO_ID,
                                   local_mapping_overlay_ip_hdl_);
    ret = mapping_impl_db()->local_mapping_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    // add entry to MAPPING table for overlay IP
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                   &spec->key.ip_addr);
    memset(&mapping_data, 0, sizeof(mapping_data));
    mapping_data.nexthop_valid = TRUE;
    mapping_data.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    mapping_data.nexthop_id = vnic_impl_obj->nh_idx();
    mapping_data.egress_bd_id = ((subnet_impl *)subnet->impl())->hw_id();
    sdk::lib::memrev(mapping_data.dmaci, vnic->mac(), ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key,
                                   NULL, &mapping_data,
                                   MAPPING_MAPPING_INFO_ID, mapping_hdl_);
    ret = mapping_impl_db()->mapping_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    if (spec->public_ip_valid) {
        // add entry to LOCAL_MAPPING table for public IP
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                             &spec->public_ip);
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_mapping_data,
                                               vnic_impl_obj->hw_id(),
                                               to_overlay_ip_nat_idx_);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams,
                                       &local_mapping_key,
                                       NULL, &local_mapping_data,
                                       LOCAL_MAPPING_LOCAL_MAPPING_INFO_ID,
                                       local_mapping_public_ip_hdl_);
        ret = mapping_impl_db()->local_mapping_tbl()->insert(&tparams);
        if (ret != SDK_RET_OK) {
            goto error;
        }

        // add entry to MAPPING table for public IP
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                       &spec->public_ip);
        // reuse the mapping data filled above as there is no change
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key,
                                       NULL, &mapping_data,
                                       MAPPING_MAPPING_INFO_ID,
                                       mapping_public_ip_hdl_);
        ret = mapping_impl_db()->mapping_tbl()->insert(&tparams);
        if (ret != SDK_RET_OK) {
            goto error;
        }

    }
    return SDK_RET_OK;

error:

    // TODO: handle cleanup in case of failure
    return ret;
}

sdk_ret_t
mapping_impl::activate_create_(pds_epoch_t epoch, mapping_entry *mapping,
                               api_obj_ctxt_t *obj_ctxt,
                               pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    subnet_entry *subnet;

    vpc = vpc_find(&spec->key.vpc);
    subnet = subnet_find(&spec->subnet);
    PDS_TRACE_DEBUG("Activating %s %s, subnet %u, tep %u, "
                    "overlay mac %s, fabric encap type %u "
                    "fabric encap value %u, vnic %u",
                    spec->is_local ? "local" : "remote",
                    mapping->key2str().c_str(), spec->subnet.id, spec->tep.id,
                    macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, spec->vnic.id);
    if (spec->is_local) {
        ret = add_local_mapping_entries_(vpc, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    } else {
        ret = add_remote_mapping_entries_(vpc, subnet, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    // TODO: take care of MAC entries also while printing !!
    PDS_TRACE_ERR("Failed to program %s, err %u",
                  mapping->key2str().c_str(), ret);
    return ret;
}

sdk_ret_t
mapping_impl::deactivate_l2_mapping_entry_(pds_subnet_key_t subnet,
                                           mac_addr_t mac_addr) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    mapping_appdata_t mapping_data = { 0 };

    PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key, subnet_hw_id_, mac_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, &mapping_data,
                                   0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to deactivate mapping (subnet %u, mac %s)",
                      subnet.id, macaddr2str(mac_addr));
    }
    return ret;
}

sdk_ret_t
mapping_impl::deactivate_ip_mapping_entry_(pds_vpc_key_t vpc, ip_addr_t *ip) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    mapping_appdata_t mapping_data = { 0 };

    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc_hw_id_, ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, &mapping_data,
                                   0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to deactivate mapping (vpc %u, ip %s)",
                      vpc.id, ipaddr2str(ip));
    }
    return ret;
}

sdk_ret_t
mapping_impl::deactivate_ip_local_mapping_entry_(pds_vpc_key_t vpc,
                                                 ip_addr_t *ip) {
    sdk_ret_t ret;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;
    mapping_appdata_t local_mapping_data = { 0 };

    // deactivate mapping entry
    ret = deactivate_ip_mapping_entry_(vpc, ip);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // deactivate local mapping entry
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc_hw_id_, ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                   &local_mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update mapping (vpc %u, ip %s)",
                      vpc.id, ipaddr2str(ip));
    }
    return ret;
}

sdk_ret_t
mapping_impl::activate_delete_(pds_epoch_t epoch, mapping_entry *mapping) {
    sdk_ret_t ret;

    if (!mapping->is_local()) {
        PDS_TRACE_DEBUG("Deleting remote mapping %s",
                        mapping->key2str().c_str());
        if (mapping->key().type == PDS_MAPPING_TYPE_L3) {
            // delete the L3 mapping
            return deactivate_ip_mapping_entry_(mapping->key().vpc,
                                                &mapping->key().ip_addr);
        }
        // must be L2 mapping
        return deactivate_l2_mapping_entry_(mapping->key().subnet,
                                            mapping->key().mac_addr);
    }
    PDS_TRACE_DEBUG("Deleting local mapping %s", mapping->key2str().c_str());
    ret = deactivate_ip_local_mapping_entry_(mapping->key().vpc,
                                             &mapping->key().ip_addr);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    if (mapping->is_public_ip_valid()) {
        ret = deactivate_ip_local_mapping_entry_(mapping->key().vpc,
                                                 &mapping->public_ip());
    }
    return ret;
}

sdk_ret_t
mapping_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                          pds_epoch_t epoch, api_op_t api_op,
                          api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_mapping_key_t *key;
    pds_mapping_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->mapping_spec;
        ret = activate_create_(epoch, (mapping_entry *)api_obj,
                               obj_ctxt, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (mapping_entry *)api_obj);
        break;

    case API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
mapping_impl::read_remote_mapping_(vpc_entry *vpc, subnet_entry *subnet,
                                   pds_mapping_info_t *info) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data;
    mapping_swkey_t mapping_key;
    nexthop_group_impl *nh_group;
    mapping_appdata_t mapping_data;
    sdk_table_api_params_t tparams;
    pds_mapping_spec_t *spec = &info->spec;
    pds_mapping_status_t *status = &info->status;

    if (info->spec.key.type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                       &spec->key.ip_addr);
    } else {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key,
                                       ((subnet_impl *)subnet->impl())->hw_id(),
                                       spec->key.mac_addr);
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL,
                                   &mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    switch (mapping_data.nexthop_type) {
    case NEXTHOP_TYPE_TUNNEL:
        spec->nh_type = PDS_NH_TYPE_OVERLAY;
        break;
    case NEXTHOP_TYPE_ECMP:
        spec->nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
        break;
    case NEXTHOP_TYPE_NEXTHOP:
        spec->nh_type = PDS_NH_TYPE_UNDERLAY;
        break;
    case NEXTHOP_TYPE_VPC:
        spec->nh_type = PDS_NH_TYPE_PEER_VPC;
        break;
    default:
        return SDK_RET_ERR;
    }
    sdk::lib::memrev(spec->overlay_mac, mapping_data.dmaci, ETH_ADDR_LEN);

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_BD, mapping_data.egress_bd_id,
                                      NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_READ_ERR;
    }
    spec->fabric_encap.val.vnid = bd_data.bd_info.vni;
    spec->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    // TODO: tag support
    spec->num_tags = 0;
    status->subnet_hw_id = mapping_data.egress_bd_id;

    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::read_local_mapping_(vpc_entry *vpc, pds_mapping_info_t *info) {
    sdk_ret_t               ret;
    local_mapping_swkey_t   local_mapping_key;
    local_mapping_appdata_t local_mapping_data;
    sdk_table_api_params_t  tparams;
    nat_actiondata_t        nat_data;
    pds_mapping_spec_t      *spec = &info->spec;
    pds_mapping_status_t    *status = &info->status;
    p4pd_error_t            p4pd_ret;

    // first read the remote mapping, it can provide all the info except vnic id
    ret = read_remote_mapping_(vpc, NULL, info);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // read the local mapping
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                         &spec->key.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                   &local_mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    status->vnic_hw_id = local_mapping_data.vnic_id;

    if (local_mapping_data.xlate_id != PDS_IMPL_RSVD_NAT_HW_ID) {
        p4pd_ret = p4pd_global_entry_read(P4TBL_ID_NAT,
                                          local_mapping_data.xlate_id, NULL,
                                          NULL, &nat_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            return sdk::SDK_RET_HW_READ_ERR;
        }
        spec->public_ip_valid = true;
        P4_IPADDR_TO_IPADDR(nat_data.nat_action.ip, spec->public_ip,
                            spec->key.ip_addr.af);
    }
    // TODO: tag support
    spec->num_tags = 0;

    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    subnet_entry *subnet;
    pds_vpc_key_t vpc_key;
    nat_actiondata_t nat_data = { 0 };
    pds_mapping_key_t *mkey = (pds_mapping_key_t *)key;
    pds_mapping_info_t *minfo = (pds_mapping_info_t *)info;
    mapping_entry *mapping = (mapping_entry *)api_obj;

    if (mkey->type == PDS_MAPPING_TYPE_L3) {
        vpc = vpc_find(&mkey->vpc);
        subnet = NULL;
    } else {
        subnet = subnet_find(&mkey->subnet);
        vpc_key = subnet->vpc();
        vpc = vpc_find(&vpc_key);
    }
    memcpy(&minfo->spec.key, mkey, sizeof(pds_mapping_key_t));
    if (mapping->is_local()) {
        ret = read_local_mapping_(vpc, minfo);
    } else {
        ret = read_remote_mapping_(vpc, subnet, minfo);
    }
    return ret;
}

/// \@}

}    // namespace impl
}    // namespace api
