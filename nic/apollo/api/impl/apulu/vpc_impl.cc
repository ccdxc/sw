//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of vpc
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL - vpc entry datapath implementation
/// \ingroup PDS_VPC
/// \@{

vpc_impl *
vpc_impl::factory(pds_vpc_spec_t *spec) {
    vpc_impl *impl;

    // TODO: move to slab later
    impl = (vpc_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VPC_IMPL,
                                   sizeof(vpc_impl));
    new (impl) vpc_impl();
    return impl;
}

void
vpc_impl::destroy(vpc_impl *impl) {
    impl->~vpc_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_VPC_IMPL, impl);
}

sdk_ret_t
vpc_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    sdk_table_api_params_t api_params = { 0 };
    pds_vpc_spec_t *spec = &obj_ctxt->api_params->vpc_spec;

    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };

    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
vpc_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_impl::activate_vpc_create_(pds_epoch_t epoch, vpc_entry *vpc,
                               pds_vpc_spec_t *spec) {
    sdk_ret_t ret;
    sdk_table_api_params_t api_params = { 0 };

    PDS_TRACE_DEBUG("Activating vpc %u, type %u, fabric encap (%u, %u)",
                    spec->key.id, spec->type, spec->fabric_encap.type,
                    spec->fabric_encap.val.vnid);
    return ret;
}

sdk_ret_t
vpc_impl::activate_vpc_delete_(pds_epoch_t epoch, vpc_entry *vpc) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                       api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_vpc_spec_t *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
    case api::API_OP_DELETE:
    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
vpc_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                        api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
vpc_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_VPC_IMPL

}    // namespace impl
}    // namespace api
