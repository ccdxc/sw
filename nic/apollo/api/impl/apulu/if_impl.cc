//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of interface
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/impl/apulu/if_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup IF_IMPL_IMPL - interface datapath implementation
/// \ingroup IF_IMPL
/// \@{

if_impl *
if_impl::factory(pds_if_spec_t *spec) {
    if_impl *impl;

    impl = (if_impl *)SDK_CALLOC(SDK_MEM_ALLOC_IF_IMPL_IMPL, sizeof(if_impl));
    new (impl) if_impl();
    return impl;
}

void
if_impl::destroy(if_impl *impl) {
    impl->~if_impl();
    SDK_FREE(SDK_MEM_ALLOC_IF_IMPL_IMPL, impl);
}

sdk_ret_t
if_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    pds_if_spec_t *spec;

    spec = &obj_ctxt->api_params->if_spec;
    switch (spec->type) {
    case PDS_IF_TYPE_L3:
        ret = if_impl_db()->l3if_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to alloc hw id for L3 if %u, err %u",
                          spec->key.id, ret);
            return ret;
        }
        hw_id_ = idx;
        break;

    default:
        PDS_TRACE_ERR("Unknown interface type %u for interface %u",
                      spec->type, spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_impl::release_resources(api_base *api_obj) {
    if_entry *intf = (if_entry *)api_obj;

    if (hw_id_ != 0xFFFF) {
        switch (intf->type()) {
        case PDS_IF_TYPE_L3:
            break;
        default:
            break;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
if_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
if_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
if_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#define vni_info    action_u.vni_vni_info
sdk_ret_t
if_impl::activate_if_create_(pds_epoch_t epoch, if_entry *intf,
                             pds_if_spec_t *spec) {
    sdk_ret_t ret;
    sdk_table_api_params_t tparams = { 0 };

    PDS_TRACE_DEBUG("Activating if %u, type %u, admin state %u",
                    spec->key.id, spec->type, spec->admin_state);
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
if_impl::activate_if_delete_(pds_epoch_t epoch, if_entry *intf) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
if_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                     api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_if_spec_t *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
        spec = &obj_ctxt->api_params->if_spec;
        ret = activate_if_create_(epoch, (if_entry *)api_obj, spec);
        break;

    case api::API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_if_delete_(epoch, (if_entry *)api_obj);
        break;

    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
if_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                        api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
if_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of IF_IMPL_IMPL

}    // namespace impl
}    // namespace api
