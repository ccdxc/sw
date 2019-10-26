//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP datapath implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL - tep datapath implementation
/// \ingroup PDS_TEP
/// @{

tep_impl *
tep_impl::factory(pds_tep_spec_t *pds_tep) {
    tep_impl *impl;

    // TODO: move to slab later
    impl = (tep_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_TEP_IMPL,
                                  sizeof(tep_impl));
    new (impl) tep_impl();
    return impl;
}

void
tep_impl::destroy(tep_impl *impl) {
    impl->~tep_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_TEP_IMPL, impl);
}

sdk_ret_t
tep_impl::reserve_resources(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;

    ret = tep_impl_db()->tep_idxr()->alloc(&idx);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in TEP table for TEP %s, err %u",
                      api_obj->key2str().c_str(), ret);
        return ret;
    }
    hw_id_ = idx;
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::release_resources(api_base *api_obj) {
    if (hw_id_ != 0xFFFF) {
        tep_impl_db()->tep_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::nuke_resources(api_base *api_obj) {
    if (hw_id_ != 0xFFFF) {
        tep_impl_db()->tep_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

#define PDS_NUM_NH_NO_ECMP 1
#define tunnel_action    action_u.tunnel_tunnel_info
sdk_ret_t
tep_impl::activate_create_(pds_epoch_t epoch, tep_entry *tep,
                               pds_tep_spec_t *spec) {
    sdk_ret_t ret;
    nexthop_impl *nh_impl;
    p4pd_error_t p4pd_ret;
    nexthop_group *nhgroup;
    nexthop_group_impl *nhgroup_impl;
    tunnel_actiondata_t tep_data = { 0 };

    PDS_TRACE_DEBUG("Activating TEP %u create, hw id %u", spec->key.id, hw_id_);
    if (spec->nh_type == PDS_NH_TYPE_UNDERLAY_ECMP) {
        nhgroup = nexthop_group_db()->find(&spec->nh_group);
        nhgroup_impl = (nexthop_group_impl *)nhgroup->impl();
        tep_data.tunnel_action.nexthop_base = nhgroup_impl->hw_id();
        tep_data.tunnel_action.num_nexthops = nhgroup->num_nexthops();
    } else if (spec->nh_type == PDS_NH_TYPE_UNDERLAY) {
        nh_impl = (nexthop_impl *)nexthop_db()->find(&spec->nh)->impl();
        tep_data.tunnel_action.nexthop_base = nh_impl->hw_id();
        tep_data.tunnel_action.num_nexthops = PDS_NUM_NH_NO_ECMP;
    } else {
        // TODO: uncomment once testapp is fixed
        //SDK_ASSERT_RETURN(false, SDK_RET_INVALID_ARG);
    }
    if (spec->remote_ip.af == IP_AF_IPV4) {
        tep_data.tunnel_action.ip_type = IPTYPE_IPV4;
        memcpy(tep_data.tunnel_action.dipo, &spec->remote_ip.addr.v4_addr,
               IP4_ADDR8_LEN);
    } else if (spec->remote_ip.af == IP_AF_IPV6) {
        tep_data.tunnel_action.ip_type = IPTYPE_IPV6;
        sdk::lib::memrev(tep_data.tunnel_action.dipo,
                         spec->remote_ip.addr.v6_addr.addr8,
                         IP6_ADDR8_LEN);
    }
    sdk::lib::memrev(tep_data.tunnel_action.dmaci, spec->mac, ETH_ADDR_LEN);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_TUNNEL, hw_id_,
                                       NULL, NULL, &tep_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program TEP %u at idx %u",
                      spec->key.id, hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::activate_delete_(pds_epoch_t epoch, tep_entry *tep) {
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                      api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_tep_spec_t *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
        spec = &obj_ctxt->api_params->tep_spec;
        ret = activate_create_(epoch, (tep_entry *)api_obj, spec);
        break;

    case api::API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (tep_entry *)api_obj);
        break;

    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

void
tep_impl::fill_status_(pds_tep_status_t *status) {
    status->hw_id = hw_id_;
}

sdk_ret_t
tep_impl::fill_spec_(pds_tep_spec_t *spec) {
    tunnel_actiondata_t tep_data;
    p4pd_error_t p4pdret;

    p4pdret = p4pd_global_entry_read(P4TBL_ID_TUNNEL, hw_id_,
                                     NULL, NULL, &tep_data);
    if (unlikely(p4pdret != P4PD_SUCCESS)) {
        PDS_TRACE_ERR("p4 global entry read failed for hw id %u, ret %d",
                      hw_id_, p4pdret);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    switch (tep_data.tunnel_action.ip_type) {
    case IPTYPE_IPV4:
        spec->remote_ip.af = IP_AF_IPV4;
        memcpy(&spec->remote_ip.addr.v4_addr, tep_data.tunnel_action.dipo,
               IP4_ADDR8_LEN);
        break;
    case IPTYPE_IPV6:
        spec->remote_ip.af = IP_AF_IPV6;
        sdk::lib::memrev(spec->remote_ip.addr.v6_addr.addr8,
                         tep_data.tunnel_action.dipo,
                         IP6_ADDR8_LEN);
        break;
    default:
        break;
    }
    sdk::lib::memrev(spec->mac, tep_data.tunnel_action.dmaci, ETH_ADDR_LEN);
    if (tep_data.tunnel_action.num_nexthops > PDS_NUM_NH_NO_ECMP) {
        spec->nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
    } else if (tep_data.tunnel_action.num_nexthops == PDS_NUM_NH_NO_ECMP) {
        spec->nh_type = PDS_NH_TYPE_UNDERLAY;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t rv;
    pds_tep_info_t *tep_info = (pds_tep_info_t *)info;

    rv = fill_spec_(&tep_info->spec);
    if (unlikely(rv != sdk::SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read hardware table for TEP %s",
                      api_obj->key2str().c_str());
        return rv;
    }

    fill_status_(&tep_info->status);
    return sdk::SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
