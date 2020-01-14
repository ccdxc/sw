//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of vpc peering
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/vpc_peer.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/artemis/vpc_peer_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/p4/include/artemis_defines.h"

using sdk::table::sdk_table_api_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_PEER - vpc peering datapath implementation
/// \ingroup PDS_VPC
/// @{

#define PDS_IMPL_FILL_VPC_PEER_SWKEY(key, vpc_hw_id1, vpc_hw_id2)            \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->vpc_id1 = vpc_hw_id1;                                             \
    (key)->vpc_id2 = vpc_hw_id2;                                             \
}

#define PDS_IMPL_FILL_VPC_PEER_APPDATA(data)                                 \
    memset((data), 0, sizeof(*(data)));                                      \
}

vpc_peer_impl *
vpc_peer_impl::factory(pds_vpc_peer_spec_t *spec) {
    vpc_peer_impl    *impl;

    impl = vpc_peer_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) vpc_peer_impl();
    return impl;
}

void
vpc_peer_impl::soft_delete(vpc_peer_impl *impl) {
    impl->~vpc_peer_impl();
    vpc_peer_impl_db()->free(impl);
}

void
vpc_peer_impl::destroy(vpc_peer_impl *impl) {
    vpc_peer_impl::soft_delete(impl);
}

// TODO: read from p4 tables
vpc_peer_impl *
vpc_peer_impl::build(pds_obj_key_t *key, vpc_peer_entry *vpc_peer) {
    vpc_peer_impl *impl;

    impl = vpc_peer_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) vpc_peer_impl();
    return NULL;
}

sdk_ret_t
vpc_peer_impl::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    vpc_entry *vpc1, *vpc2;
    pds_vpc_peer_spec_t *spec;
    //vpc_peer_swkey_t vpc_peer_key;
    sdk_table_api_params_t api_params;

    spec = &obj_ctxt->api_params->vpc_peer_spec;
    vpc1 = vpc_db()->find(&spec->vpc1);
    if (unlikely(vpc1 == NULL)) {
        return SDK_RET_INVALID_ARG;
    }

    vpc2 = vpc_db()->find(&spec->vpc2);
    if (unlikely(vpc2 == NULL)) {
        return SDK_RET_INVALID_ARG;
    }
    PDS_TRACE_DEBUG("Reserving resources for vpc peering of (vpc %u, vpc %u)",
                    spec->vpc1.id, spec->vpc2.id);

#if 0
    // reserve an entry in VPC_PEER table with (vpc1, vpc2) as key
    PDS_IMPL_FILL_VPC_PEER_SWKEY(&vpc_peer_key, vpc1->hw_id(), vpc2->hw_id());
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &vpc_peer_key, NULL,
                                   NULL, 0, sdk::table::handle_t::null());
    ret = vpc_peer_impl_db()->vpc_peer_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in VPC_PEER table "
                      "for %s, err %u", api_obj->key2str().c_str(), ret);
        return ret;
    }
    vpc_peer_handle1_ = api_params.handle;

    // reserve an entry in VPC_PEER table with (vpc2, vpc1) as key
    PDS_IMPL_FILL_VPC_PEER_SWKEY(&vpc_peer_key, vpc2->hw_id(), vpc1->hw_id());
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &vpc_peer_key, NULL,
                                   NULL, 0, sdk::table::handle_t::null());
    ret = vpc_peer_impl_db()->vpc_peer_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve reverse entry in VPC_PEER table "
                      "for %s, err %u", api_obj->key2str().c_str(), ret);
        return ret;
    }
    vpc_peer_handle2_ = api_params.handle;
#endif
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t    api_params = { 0 };

    if (vpc_peer_handle1_.valid()) {
        api_params.handle = vpc_peer_handle1_;
        vpc_peer_impl_db()->vpc_peer_tbl()->remove(&api_params);
    }

    if (vpc_peer_handle2_.valid()) {
        api_params.handle = vpc_peer_handle2_;
        vpc_peer_impl_db()->vpc_peer_tbl()->remove(&api_params);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t    api_params = { 0 };

    if (vpc_peer_handle1_.valid()) {
        api_params.handle = vpc_peer_handle1_;
        vpc_peer_impl_db()->vpc_peer_tbl()->release(&api_params);
    }
    if (vpc_peer_handle2_.valid()) {
        api_params.handle = vpc_peer_handle2_;
        vpc_peer_impl_db()->vpc_peer_tbl()->release(&api_params);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_peer_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                        api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_peer_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                           pds_epoch_t epoch, api_op_t api_op,
                           api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vpc_entry *vpc1, *vpc2;
    pds_vpc_peer_spec_t *vpc_peer_spec;
    vpc_peer_entry *vpc_peer = (vpc_peer_entry *)api_obj;
    //vpc_peer_swkey_t vpc_peer_key;
    //vpc_peer_swdata_t vpc_peer_data;
    sdk_table_api_params_t api_params;

    vpc_peer_spec = &obj_ctxt->api_params->vpc_peer_spec;
    vpc1 = vpc_db()->find(&vpc_peer_spec->vpc1);
    if (unlikely(vpc1 == NULL)) {
        return SDK_RET_INVALID_ARG;
    }

    vpc2 = vpc_db()->find(&vpc_peer_spec->vpc2);
    if (unlikely(vpc2 == NULL)) {
        return SDK_RET_INVALID_ARG;
    }

    PDS_TRACE_DEBUG("Programming vpc peering for (vpc %u, vpc %u)",
                    vpc_peer_spec->vpc1.id, vpc_peer_spec->vpc2.id);

#if 0
    // program (vpc1, vpc2) entry
    PDS_IMPL_FILL_VPC_PEER_SWKEY(&vpc_peer_key, vpc1->hw_id(), vpc2->hw_id());
    PDS_IMPL_FILL_VPC_PEER_APPDATA(&vpc_peer_data);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &vpc_peer_key, NULL,
                                   &vpc_peer_data, VPC_PEER_INFO_ID,
                                   vpc_peer_handle1_);
    ret = vpc_peer_impl_db()->vpc_peer_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program entry in VPC_PEER table "
                      "for %s, err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }

    // program (vpc2, vpc1) entry
    PDS_IMPL_FILL_VPC_PEER_SWKEY(&vpc_peer_key, vpc2->hw_id(), vpc1->hw_id());
    PDS_IMPL_FILL_VPC_PEER_APPDATA(&vpc_peer_data);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &vpc_peer_key, NULL,
                                   &vpc_peer_data, VPC_PEER_INFO_ID,
                                   vpc_peer_handle2_);
    ret = vpc_peer_impl_db()->vpc_peer_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program reverse entry in VPC_PEER table "
                      "for %s, err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }
    return SDK_RET_OK;

error:
    return ret;
#endif
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_impl::read_hw(pds_obj_key_t *key, pds_vpc_peer_info_t *info) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_VPC_PEER

}    // namespace impl
}    // namespace api
