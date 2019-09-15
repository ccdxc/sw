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
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"
#include "nic/apollo/api/impl/apulu/mapping_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"

#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(key, vpc_hw_id, ip)             \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->vnic_metadata_vpc_id = vpc_hw_id;                                 \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        sdk::lib::memrev((key)->key_metadata_mapping_ip,                     \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        memcpy((key)->key_metadata_mapping_ip,                               \
               &(ip)->addr.v4_addr, IP4_ADDR8_LEN);                          \
    }                                                                        \
}

#define PDS_IMPL_FILL_MAPPING_SWKEY(key, vpc_hw_id, ip)                      \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->rx_to_tx_hdr_vpc_id = vpc_hw_id;                                  \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        sdk::lib::memrev((key)->rx_to_tx_hdr_remote_ip,                      \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        memcpy((key)->rx_to_tx_hdr_remote_ip,                                \
               &(ip)->addr.v4_addr, IP4_ADDR8_LEN);                          \
    }                                                                        \
}

using sdk::table::sdk_table_api_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL - mapping entry datapath implementation
/// \ingroup PDS_MAPPING
/// @{

mapping_impl *
mapping_impl::factory(pds_mapping_spec_t *spec) {
    mapping_impl    *impl;
    device_entry    *device;

    impl = mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) mapping_impl();
    device = device_db()->find();
    if (spec->is_local) {
        impl->is_local_ = true;
        spec->tep.ip_addr = device->ip_addr();
    } else {
        impl->is_local_ = false;
    }
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

mapping_impl *
mapping_impl::build(pds_mapping_key_t *key) {
    sdk_ret_t                 ret;
    device_entry              *device;
    vpc_entry                 *vpc;
    mapping_impl              *impl;
    uint16_t                  vnic_hw_id;
    bool                      local_mapping = false;
    sdk_table_api_params_t    api_params = { 0 };
    nexthop_actiondata_t      nh_data;

    device = device_db()->find();
    vpc = vpc_db()->find(&key->vpc);
    if (unlikely(vpc == NULL)) {
        return NULL;
    }

    impl = mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) mapping_impl();
    // TODO:

    return impl;

error:
    if (impl) {
        impl->~mapping_impl();
        //SDK_FREE(SDK_MEM_ALLOC_PDS_MAPPING_IMPL, impl);
        mapping_impl_db()->free(impl);
    }
    return NULL;
}

sdk_ret_t
mapping_impl::reserve_local_mapping_resources_(api_base *api_obj,
                                               vpc_entry *vpc,
                                               pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    local_mapping_swkey_t local_mapping_key;
    sdk_table_api_params_t tparams;

#if 0
    // reserve an entry in LOCAL_MAPPING table for overlay IP
    PDS_IMPL_FILL_LOCAL_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                      &spec->key.ip_addr, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_MAPPING table "
                      "for mapping %s, err %u", api_obj->key2str().c_str(),
                      ret);
        return ret;
    }
    local_mapping_overlay_ip_hdl_ = tparams.handle;

    // reserve an entry in MAPPING table for overlay IP
    PDS_IMPL_FILL_MAPPING_SWKEY(&remote_mapping_key, vpc->hw_id(),
                                &spec->key.ip_addr, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for "
                      "mapping %s, err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }
    mapping_overlay_ip_hdl_ = tparams.handle;

    PDS_TRACE_DEBUG("Rsvd LOCAL_MAPPING handle %lx, MAPPING handle 0x%lx",
                    local_mapping_hdl_, mapping_hdl_);

    // check if this mapping has public IP
    if (!spec->public_ip_valid) {
        return SDK_RET_OK;
    }

    // reserve an entry in LOCAL_MAPPING table for public IP
    PDS_IMPL_FILL_LOCAL_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                      &spec->public_ip, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_MAPPING table "
                      "for public IP of mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    local_mapping_public_ip_hdl_ = tparams.handle;

    // reserve an entry in MAPING table for public IP
    PDS_IMPL_FILL_MAPPING_SWKEY(&remote_mapping_key, vpc->hw_id(),
                                &spec->public_ip, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for public IP "
                      "of mapping %s, err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }
    mapping_public_ip_hdl_ = tparams.handle;

    PDS_TRACE_DEBUG("Rsvd public IP hdl 0x%lx in LOCAL_MAPPING table, "
                    "public IP handle in 0x%lx in MAPPING table",
                    local_mapping_public_ip_hdl_, mapping_public_ip_hdl_);

    // reserve an entry for overlay IP to public IP xlation in NAT table
    ret = mapping_impl_db()->nat_tbl()->reserve(&to_public_ip_nat_hdl_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry for public IP in NAT table for "
                      "mapping %s, err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }

    // reserve an entry for public IP to overlay IP xlation in NAT table
    ret = mapping_impl_db()->nat_tbl()->reserve(&to_overlay_ip_nat_hdl_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry for overlay IP in NAT table for "
                      "mapping %s, err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }

    PDS_TRACE_DEBUG("Rsvd public IP NAT handle 0x%lx, overlay IP NAT "
                    "handle 0x%lx", to_public_ip_nat_hdl_,
                    to_overlay_ip_nat_hdl_);
    return SDK_RET_OK;

error:

    // TODO: release all allocated resources
    return ret;
#endif
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::reserve_remote_mapping_resources_(api_base *api_obj,
                                                vpc_entry *vpc,
                                                pds_mapping_spec_t *spec) {
#if 0
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;

    // reserve an entry in REMOTE_VNIC_MAPPING_TX table
    PDS_IMPL_FILL_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                &spec->key.ip_addr, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for mapping "
                      "%s, err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }
    mapping_handle_ = tparams.handle;
    return SDK_RET_OK;

error:

    return ret;
#endif
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
#if 0
    pds_mapping_spec_t    *spec;
    vpc_entry             *vpc;

    spec = &obj_ctxt->api_params->mapping_spec;
    vpc = vpc_db()->find(&spec->key.vpc);
    PDS_TRACE_DEBUG("Reserving resources for mapping (vpc %u, ip %s), "
                    "local %u, subnet %u, tep %s, vnic %u, "
                    "pub_ip_valid %u, pub_ip %s, "
                    "prov_ip_valid %u, prov_ip %s",
                    spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr), is_local_,
                    spec->subnet.id, ipaddr2str(&spec->tep.ip_addr),
                    spec->vnic.id, spec->public_ip_valid,
                    ipaddr2str(&spec->public_ip),
                    spec->provider_ip_valid, ipaddr2str(&spec->provider_ip));
    if (is_local_) {
        return reserve_local_ip_mapping_resources_(orig_obj, vpc, spec);
    }
    return reserve_remote_ip_mapping_resources_(orig_obj, vpc, spec);
#endif
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::nuke_resources(api_base *api_obj) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::release_local_mapping_resources_(api_base *api_obj) {
#if 0
    sdk_table_api_params_t    api_params = { 0 };

    if (local_mapping_overlay_ip_hdl_.valid()) {
        api_params.handle = local_mapping_overlay_ip_hdl_;
        mapping_impl_db()->local_mapping_tbl()->release(&api_params);
    }
    if (local_mapping_public_ip_hdl_.valid()) {
        api_params.handle = local_mapping_public_ip_hdl_;
        mapping_impl_db()->local_mapping_tbl()->release(&api_params);
    }
    if (mapping_overlay_ip_hdl_.valid()) {
        api_params.handle = mapping_overlay_ip_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&api_params);
    }
    if (mapping_public_ip_hdl_.valid()) {
        api_params.handle = mapping_public_ip_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&api_params);
    }
    if (to_public_ip_nat_hdl_ != 0xFFFF) {
        mapping_impl_db()->nat_tbl()->release(to_public_ip_nat_hdl_);
    }
    if (to_overlay_ip_nat_hdl_) {
        mapping_impl_db()->nat_tbl()->release(to_overlay_ip_nat_hdl_);
    }
    return SDK_RET_OK;
#endif
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::release_remote_mapping_resources_(api_base *api_obj) {
#if 0
    sdk_table_api_params_t    api_params = { 0 };

    if (mapping_overlay_ip_hdl_.valid()) {
        api_params.handle = mapping_overlay_ip_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&api_params);
    }
    return SDK_RET_OK;
#endif
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::release_resources(api_base *api_obj) {
    if (is_local_) {
        return release_local_mapping_resources_(api_obj);
    }
    return release_remote_mapping_resources_(api_obj);
}

// TODO: take care of both local and remote mappings here
sdk_ret_t
mapping_impl::add_remote_mapping_entries_(vpc_entry *vpc,
                                          pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    sdk_table_api_params_t api_params;
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::add_nat_entries_(pds_mapping_spec_t *spec) {
    sdk_ret_t           ret;
#if 0
    nat_actiondata_t    nat_data = { 0 };

    // add NAT table entries
    if (spec->public_ip_valid) {
        // add private to public IP xlation NAT entry
        PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->public_ip);
        ret = mapping_impl_db()->nat_tbl()->insert_atid(&nat_data,
                                                        to_public_ip_nat_hdl_);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        // add public to private IP xlation NAT entry
        PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->key.ip_addr);
        ret = mapping_impl_db()->nat_tbl()->insert_atid(&nat_data,
                                                        to_overlay_ip_nat_hdl_);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:
    // TODO: handle cleanup in case of failure
    return ret;
#endif
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::add_local_mapping_entries_(vpc_entry *vpc,
                                         pds_mapping_spec_t *spec) {
#if 0
    sdk_ret_t ret;
    local_mapping_swkey_t local_mapping_key;
    local_mapping_appdata_t local_mapping_data;
    mapping_swkey_t mapping_tx_key;
    mapping_appdata_t mapping_tx_data;
    sdk_table_api_params_t api_params;

    // add entry to LOCAL_MAPPING table for overlay IP
    PDS_IMPL_FILL_LOCAL_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                      &spec->key.ip_addr);
    PDS_IMPL_FILL_LOCAL_MAPPING_APPDATA(&local_mapping_data, vnic_impl->hw_id(),
                                        to_public_ip_nat_hdl_);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_mapping_key,
                                   NULL, &local_mapping_data,
                                   LOCAL_MAPPING_LOCAL_MAPPING_INFO_ID,
                                   local_mapping_overlay_ip_hdl_);
    ret = mapping_impl_db()->local_mapping_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    // add entry to LOCAL_MAPPING table for public IP
    if (spec->public_ip_valid) {
        PDS_IMPL_FILL_LOCAL_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                          &spec->public_ip);
        PDS_IMPL_FILL_LOCAL_MAPPING_APPDATA(&local_mapping_data,
                                            vnic_impl->hw_id(),
                                            to_overlay_ip_nat_hdl_);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params,
                                       &local_mapping_key,
                                       NULL, &local_mapping_data,
                                       LOCAL_MAPPING_LOCAL_MAPPING_INFO_ID,
                                       local_mapping_public_ip_hdl_);
        ret = mapping_impl_db()->local_mapping_tbl()->insert(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }

    // TODO: add entries to MAPPING table for overlay & public IPs

error:

    // TODO: handle cleanup in case of failure
    return ret;
#endif
    return SDK_RET_INVALID_OP;
}

// TODO: as we are not able to reserve() ahead of time currently, entries are
//       programmed here, ideally we just use pre-allocated indices
sdk_ret_t
mapping_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t             ret;
    pds_mapping_spec_t    *spec;
    vpc_entry             *vpc;
    subnet_entry          *subnet;

    spec = &obj_ctxt->api_params->mapping_spec;
    vpc = vpc_db()->find(&spec->key.vpc);
    subnet = subnet_db()->find(&spec->subnet);
    PDS_TRACE_DEBUG("Programming mapping (vpc %u, ip %s), subnet %u, tep %s, "
                    "overlay mac %s, fabric encap type %u "
                    "fabric encap value %u, vnic %u",
                    spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                    spec->subnet.id, ipaddr2str(&spec->tep.ip_addr),
                    macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, spec->vnic.id);
    if (is_local_) {
        // allocate NAT table entries
        ret = add_nat_entries_(spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
        ret = add_local_mapping_entries_(vpc, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    } else {
        ret = add_remote_mapping_entries_(vpc, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    // TODO: take care of MAC entries also while printing !!
    PDS_TRACE_ERR("Failed to program mapping (vpc %u, ip %s)",
                  spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr));
    return ret;
}

sdk_ret_t
mapping_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_OK;
}

sdk_ret_t
mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                        obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                          api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::read_local_mapping_(vpc_entry *vpc, pds_mapping_spec_t *spec) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::read_remote_mapping_(vpc_entry *vpc, pds_mapping_spec_t *spec) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    return SDK_RET_INVALID_OP;
}

/// \@}

}    // namespace impl
}    // namespace api
