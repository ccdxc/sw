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
    (key)->key_metadata_dport = (dip_port);                                    \
    if ((dip)->af == IP_AF_IPV6) {                                             \
        sdk::lib::memrev((key)->key_metadata_dst,                              \
                         (dip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);            \
    } else {                                                                   \
        memcpy((key)->key_metadata_dst, &(dip)->addr.v4_addr, IP4_ADDR8_LEN);  \
    }                                                                          \
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
svc_mapping_impl::soft_delete(svc_mapping_impl *impl) {
    impl->~svc_mapping_impl();
    svc_mapping_impl_db()->free(impl);
}

void
svc_mapping_impl::destroy(svc_mapping_impl *impl) {
    svc_mapping_impl::soft_delete(impl);
}

svc_mapping_impl *
svc_mapping_impl::build(pds_svc_mapping_key_t *key) {
    return NULL;
}

sdk_ret_t
svc_mapping_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    pds_svc_mapping_spec_t *spec;
    sdk_table_api_params_t tparams;
    service_mapping_swkey_t svc_mapping_key;

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
        PDS_TRACE_ERR("Failed to reserve an entry for (PIP %s, PIP port %u) in "
                      "SERVICE_MAPPING table, err %u",
                      ipaddr2str(&spec->key.backend_ip),
                      spec->key.backend_port, ret);
        goto error;
    }
    to_vip_handle_ = tparams.handle;

    // reserve an entry in the NAT table for (PIP, PIP port) -> (VIP, VIP port)
    // xlation for the source IP and port in the Tx/egress direction
    ret = apulu_impl_db()->nat_idxr()->alloc(&to_vip_nat_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve (PIP %s, PIP port %u) -> VIP xlation "
                      "entry in NAT table, err %u",
                      ipaddr2str(&spec->key.backend_ip),
                      spec->key.backend_port, ret);
        goto error;
    }
    return SDK_RET_OK;

error:
    return ret;
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
svc_mapping_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    pds_svc_mapping_spec_t *spec;
    nat_actiondata_t nat_data = { 0 };

    spec = &obj_ctxt->api_params->svc_mapping_spec;
    // add an entry for (PIP, PIP port) -> (VIP, VIP port) xlation
    PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->vip, spec->svc_port);
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
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                            obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
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
    service_mapping_swkey_t svc_mapping_key;
    service_mapping_actiondata_t svc_mapping_data;

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
        PDS_TRACE_ERR("Failed to deactivate (PIP %s, PIP port %u) -> "
                      "(VIP, svc port) xlation in SERVICE_MAPPING table, "
                      "err %u", ipaddr2str(&key->backend_ip),
                      key->backend_port, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                              api_op_t api_op, obj_ctxt_t *obj_ctxt) {
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
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
svc_mapping_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    return SDK_RET_OK;
}

/// \@}    // end of PDS_SVC_MAPPING_IMPL

}    // namespace impl
}    // namespace api
