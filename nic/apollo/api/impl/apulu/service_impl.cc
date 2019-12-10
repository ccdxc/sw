//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of service mapping
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/service.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/service_impl.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

using sdk::table::sdk_table_api_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_SVC_MAPPING_IMPL - service mapping datapath implementation
/// \ingroup PDS_SERVICE
/// @{

#define PDS_IMPL_FILL_SVC_MAPPING_SWKEY(key, vpc_hw_id, dip, dip_port)         \
{                                                                              \
    memset((key), 0, sizeof(*(key)));                                          \
    (key)->vnic_metadata_vpc_id = vpc_hw_id;                                   \
    if ((dip)->af == IP_AF_IPV6) {                                             \
        sdk::lib::memrev((key)->key_metadata_dst,                              \
                         (dip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);            \
    } else {                                                                   \
        memcpy((key)->key_metadata_dst, &(dip)->addr.v4_addr, IP4_ADDR8_LEN);  \
    }                                                                          \
    (key)->key_metadata_dport = (dip_port);                                    \
}

#define svc_mapping_action action_u.service_mapping_service_mapping_info
#define PDS_IMPL_FILL_SVC_MAPPING_DATA(data, xlate_idx)                        \
{                                                                              \
    memset((data), 0, sizeof(*(data)));                                        \
    (data)->action_id = SERVICE_MAPPING_SERVICE_MAPPING_INFO_ID;               \
    (data)->svc_mapping_action.xlate_id = (xlate_idx);                         \
}

svc_mapping_impl *
svc_mapping_impl::factory(pds_svc_mapping_spec_t *mapping) {
    svc_mapping_impl    *impl;

    impl = svc_mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) svc_mapping_impl();
    return impl;
}

void
svc_mapping_impl::destroy(svc_mapping_impl *impl) {
    svc_mapping_impl::soft_delete(impl);
}

impl_base *
svc_mapping_impl::clone(void) {
    svc_mapping_impl *cloned_impl;

    cloned_impl = svc_mapping_impl_db()->alloc();
    new (cloned_impl) svc_mapping_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
svc_mapping_impl::free(svc_mapping_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

void
svc_mapping_impl::soft_delete(svc_mapping_impl *impl) {
    impl->~svc_mapping_impl();
    svc_mapping_impl_db()->free(impl);
}

svc_mapping_impl *
svc_mapping_impl::build(pds_svc_mapping_key_t *key, svc_mapping *mapping) {
    svc_mapping_impl *impl;
    sdk_ret_t ret;
    vpc_entry *vpc;
    sdk_table_api_params_t tparams;
    service_mapping_swkey_t svc_mapping_key;
    service_mapping_actiondata_t svc_mapping_data;

    vpc = vpc_db()->find(&key->vpc);
    if (unlikely(vpc == NULL)) {
        return NULL;
    }
    PDS_IMPL_FILL_SVC_MAPPING_SWKEY(&svc_mapping_key,
                                    vpc->hw_id(), &key->backend_ip,
                                    key->backend_port);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &svc_mapping_key, NULL,
                                   &svc_mapping_data,
                                   SERVICE_MAPPING_SERVICE_MAPPING_INFO_ID,
                                   sdk::table::handle_t::null());
    ret = svc_mapping_impl_db()->svc_mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return NULL;
    }

    impl = svc_mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) svc_mapping_impl();
    impl->to_vip_nat_idx_ = svc_mapping_data.svc_mapping_action.xlate_id;
    impl->to_vip_handle_ = tparams.handle;
    return impl;
}

sdk_ret_t
svc_mapping_impl::reserve_resources(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    pds_svc_mapping_spec_t *spec;
    sdk_table_api_params_t tparams;
    service_mapping_swkey_t svc_mapping_key;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        spec = &obj_ctxt->api_params->svc_mapping_spec;
        vpc = vpc_db()->find(&spec->key.vpc);

        // reserve an entry in SERVICE_MAPPING table for
        // (vpc, overlay_ip, port) -> (vpc, VIP, port)
        vpc = vpc_db()->find(&spec->key.vpc);
        PDS_IMPL_FILL_SVC_MAPPING_SWKEY(&svc_mapping_key,
                                        vpc->hw_id(), &spec->key.backend_ip,
                                        spec->key.backend_port);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &svc_mapping_key, NULL,
                                       NULL, 0, sdk::table::handle_t::null());
        ret = svc_mapping_impl_db()->svc_mapping_tbl()->reserve(&tparams);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve an entry for (PIP %s, PIP port %u)"
                          " in SERVICE_MAPPING table, err %u",
                          ipaddr2str(&spec->key.backend_ip),
                          spec->key.backend_port, ret);
            goto error;
        }
        to_vip_handle_ = tparams.handle;

        // reserve an entry in the NAT table for (PIP, PIP port) ->
        // (VIP, VIP port) xlation for the source IP and port in the
        // Tx/egress direction
        ret = apulu_impl_db()->nat_idxr()->alloc(&to_vip_nat_idx_);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve (PIP %s, PIP port %u) -> VIP "
                          "xlation entry in NAT table, err %u",
                          ipaddr2str(&spec->key.backend_ip),
                          spec->key.backend_port, ret);
            goto error;
        }
        break;

    case API_OP_UPDATE:
        // we will use the same h/w resources as the original object
    default:
        break;
    }
    return SDK_RET_OK;

error:

    return ret;
}

sdk_ret_t
svc_mapping_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };

    if (to_vip_handle_.valid()) {
        tparams.handle = to_vip_handle_;
        svc_mapping_impl_db()->svc_mapping_tbl()->release(&tparams);
    }

    if (to_vip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
        apulu_impl_db()->nat_idxr()->free(to_vip_nat_idx_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };

    if (to_vip_handle_.valid()) {
        tparams.handle = to_vip_handle_;
        svc_mapping_impl_db()->svc_mapping_tbl()->remove(&tparams);
    }
    if (to_vip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
        apulu_impl_db()->nat_idxr()->free(to_vip_nat_idx_);
    }
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    pds_svc_mapping_spec_t *spec;
    nat_actiondata_t nat_data = { 0 };

    spec = &obj_ctxt->api_params->svc_mapping_spec;
    // add an entry for (PIP, PIP port) -> (VIP, VIP port) xlation
    PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->vip,
                           spec->svc_port ?
                               spec->svc_port : spec->key.backend_port);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NAT, to_vip_nat_idx_,
                                       NULL, NULL, &nat_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NAT table entry %u for "
                      "(PIP %s, PIP port %u) -> (VIP %s, svc port %u) xlation",
                      to_vip_nat_idx_, ipaddr2str(&spec->key.backend_ip),
                      spec->key.backend_port, ipaddr2str(&spec->vip),
                      spec->svc_port);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    pds_svc_mapping_spec_t *spec;
    nat_actiondata_t nat_data = { 0 };

    spec = &obj_ctxt->api_params->svc_mapping_spec;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NAT, to_vip_nat_idx_,
                                       NULL, NULL, &nat_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to cleanup NAT table entry %u for "
                      "(PIP %s, PIP port %u) -> (VIP %s, svc port %u) xlation",
                      to_vip_nat_idx_, ipaddr2str(&spec->key.backend_ip),
                      spec->key.backend_port, ipaddr2str(&spec->vip),
                      spec->svc_port);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                            obj_ctxt_t *obj_ctxt) {
    return program_hw(curr_obj, obj_ctxt);
}

sdk_ret_t
svc_mapping_impl::activate_create_(pds_epoch_t epoch, svc_mapping *mapping,
                                   obj_ctxt_t *obj_ctxt,
                                   pds_svc_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    sdk_table_api_params_t tparams;
    service_mapping_swkey_t svc_mapping_key;
    service_mapping_actiondata_t svc_mapping_data;

    vpc = vpc_db()->find(&spec->key.vpc);
    PDS_IMPL_FILL_SVC_MAPPING_SWKEY(&svc_mapping_key,
                                    vpc->hw_id(), &spec->key.backend_ip,
                                    spec->key.backend_port);
    PDS_IMPL_FILL_SVC_MAPPING_DATA(&svc_mapping_data, to_vip_nat_idx_);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &svc_mapping_key, NULL,
                                   &svc_mapping_data,
                                   SERVICE_MAPPING_SERVICE_MAPPING_INFO_ID,
                                   to_vip_handle_);
    ret = svc_mapping_impl_db()->svc_mapping_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add (PIP %s, PIP port %u) -> "
                      "(VIP %s, svc port %u) xlation in SERVICE_MAPPING table, "
                      "err %u", ipaddr2str(&spec->key.backend_ip),
                      spec->key.backend_port, ipaddr2str(&spec->vip),
                      spec->svc_port, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::activate_delete_(pds_epoch_t epoch,
                                   pds_svc_mapping_key_t *key,
                                   svc_mapping *mapping) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    sdk_table_api_params_t tparams;
    p4pd_error_t p4pd_ret;
    service_mapping_swkey_t svc_mapping_key;
    service_mapping_actiondata_t svc_mapping_data;
    nat_actiondata_t nat_data = { 0 };

    // update the service mapping xlation idx to PDS_IMPL_RSVD_NAT_HW_ID to
    // disable NAT
    vpc = vpc_db()->find(&key->vpc);
    PDS_IMPL_FILL_SVC_MAPPING_SWKEY(&svc_mapping_key,
                                    vpc->hw_id(), &key->backend_ip,
                                    key->backend_port);
    PDS_IMPL_FILL_SVC_MAPPING_DATA(&svc_mapping_data, PDS_IMPL_RSVD_NAT_HW_ID);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &svc_mapping_key, NULL,
                                   &svc_mapping_data,
                                   SERVICE_MAPPING_SERVICE_MAPPING_INFO_ID,
                                   sdk::table::handle_t::null());
    ret = svc_mapping_impl_db()->svc_mapping_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to deactivate (%s) -> "
                      "(VIP, svc port) xlation in SERVICE_MAPPING table, "
                      "err %u", mapping->key2str().c_str(), ret);
        return ret;
    }

    /* save the handle 'update' returns */
    if (!to_vip_handle_.valid()) {
        to_vip_handle_ = tparams.handle;
    }

    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                              pds_epoch_t epoch, api_op_t api_op,
                              obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_svc_mapping_key_t *key;
    pds_svc_mapping_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->svc_mapping_spec;
        ret = activate_create_(epoch, (svc_mapping *)api_obj, obj_ctxt, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        key = &obj_ctxt->api_params->svc_mapping_key;
        ret = activate_delete_(epoch, key, (svc_mapping *)api_obj);
        break;

    case API_OP_UPDATE:
        // xlate id is not changing in update, hence no reprogramming needed
        return SDK_RET_OK;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
svc_mapping_impl::fill_spec_(pds_svc_mapping_spec_t *spec) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    vpc_entry *vpc;
    uint32_t xlate_idx;
    sdk_table_api_params_t tparams;
    service_mapping_swkey_t svc_mapping_key;
    service_mapping_actiondata_t svc_mapping_data;
    nat_actiondata_t nat_data;
    pds_svc_mapping_key_t *key = &spec->key;

    vpc = vpc_db()->find(&key->vpc);
    PDS_IMPL_FILL_SVC_MAPPING_SWKEY(&svc_mapping_key,
                                    vpc->hw_id(), &key->backend_ip,
                                    key->backend_port);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &svc_mapping_key, NULL,
                                   &svc_mapping_data,
                                   SERVICE_MAPPING_SERVICE_MAPPING_INFO_ID,
                                   sdk::table::handle_t::null());
    ret = svc_mapping_impl_db()->svc_mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to get svc-(%u, %s:%u) "
                      "xlation in SERVICE_MAPPING table, "
                      "err %u", vpc->key().id, ipaddr2str(&key->backend_ip),
                      key->backend_port, ret);
        return ret;
    }

    // now, read the NAT table to get the (VIP, svc port) tuple
    xlate_idx = svc_mapping_data.svc_mapping_action.xlate_id;
    if (xlate_idx == PDS_IMPL_RSVD_NAT_HW_ID) {
        // if the entry exists but points to the reserved NAT hardware id,
        // the entry existed but has been deleted since.
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_NAT, xlate_idx, NULL, NULL,
                                      &nat_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read NAT table entry %u for "
                      "svc-(%u, %s:%u) xlation",
                      xlate_idx, vpc->key().id, ipaddr2str(&key->backend_ip),
                      key->backend_port);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // copy out the data to the spec
    P4_IPADDR_TO_IPADDR(nat_data.nat_action.ip, spec->vip, key->backend_ip.af);
    spec->svc_port = nat_data.nat_action.port;
    memcpy(&spec->backend_provider_ip, &key->backend_ip, sizeof(ip_addr_t));

    return sdk::SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::fill_stats_(pds_svc_mapping_stats_t *stats) {
    return SDK_RET_OK;
}

void
svc_mapping_impl::fill_status_(pds_svc_mapping_status_t *status) {
}

sdk_ret_t
svc_mapping_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t ret;
    pds_svc_mapping_info_t *svc_mapping_info = (pds_svc_mapping_info_t *)info;

    // populate the key in the spec
    memcpy(&svc_mapping_info->spec.key, key, sizeof(pds_svc_mapping_key_t));
    ret = fill_spec_(&svc_mapping_info->spec);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // fill in status and stats also, though they are no-ops now
    fill_stats_(&svc_mapping_info->stats);
    fill_status_(&svc_mapping_info->status);

    return SDK_RET_OK;
}

/// \@}    // end of PDS_SVC_MAPPING_IMPL

}    // namespace impl
}    // namespace api
