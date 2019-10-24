//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of nexthop
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/nexthop.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_IMPL - nexthop datapath implementation
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_impl *
nexthop_impl::factory(pds_nexthop_spec_t *spec) {
    nexthop_impl *impl;

    // TODO: move to slab later
    impl = (nexthop_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_NEXTHOP_IMPL,
                                   sizeof(nexthop_impl));
    new (impl) nexthop_impl();
    return impl;
}

void
nexthop_impl::destroy(nexthop_impl *impl) {
    impl->~nexthop_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_NEXTHOP_IMPL, impl);
}

sdk_ret_t
nexthop_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    pds_nexthop_spec_t *spec;

    spec = &obj_ctxt->api_params->nexthop_spec;
    // for blackhole nexthop we can (re)use PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID
    if (spec->type != PDS_NH_TYPE_BLACKHOLE) {
        // reserve an entry in NEXTHOP table
        ret = nexthop_impl_db()->nh_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve entry in nexthop table, err %u",
                          ret);
            return ret;
        }
        hw_id_ = idx;
    } else {
        hw_id_ = PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::release_resources(api_base *api_obj) {
    if ((hw_id_ != PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID) &&
        (hw_id_ != 0xFFFFFFFF)) {
        return nexthop_impl_db()->nh_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::nuke_resources(api_base *api_obj) {
    // for indexer, release and nuke operations are same
    return this->release_resources(api_obj);
}

sdk_ret_t
nexthop_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

typedef struct __attribute__((__packed__)) __nexthop_nexthop_info {
    uint16_t lif;
    uint8_t qtype;
    uint32_t qid;
    uint8_t port;
    uint16_t vlan;
    uint8_t ip_type;
    uint8_t dipo[16];
    uint8_t dmaco[6];
    uint8_t smaco[6];
    uint8_t dmaci[6];
} nexthop_nexthop_info_t;

sdk_ret_t
nexthop_impl::activate_create_(pds_epoch_t epoch, nexthop *nh,
                               pds_nexthop_spec_t *spec) {
    sdk_ret_t ret;
    if_entry *intf;
    pds_encap_t encap;
    ip_prefix_t ip_pfx;
    p4pd_error_t p4pd_ret;
    nexthop_actiondata_t nh_data = { 0 };

#if 0
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    switch (spec->type) {
    case PDS_NH_TYPE_BLACKHOLE:
        // nothing to program for system-wide blackhole nexthop
        break;

    case PDS_NH_TYPE_UNDERLAY:
        intf = if_db()->find(&spec->l3_if);
        if (intf->type() != PDS_IF_TYPE_L3) {
            PDS_TRACE_ERR("Invalid nexthop %u spec, nh pointing to non L3 "
                          "intf %u", spec->key.id, intf->key().id);
            return SDK_RET_INVALID_ARG;
        }
        nh_data.nexthop_info.port = intf->port();
        encap = intf->l3_encap();
        if (encap.type == PDS_ENCAP_TYPE_DOT1Q) {
            nh_data.nexthop_info.vlan = encap.val.vlan_tag;
        } else if (encap.type != PDS_ENCAP_TYPE_NONE) {
            PDS_TRACE_ERR("Unsupported encap type %u in nexthop %u",
                          encap.type, spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        ip_pfx = intf->l3_ip_prefix();
        if (ip_pfx.addr.af == IP_AF_IPV4) {
            nh_data.nexthop_info.ip_type = IPTYPE_IPV4;
            memcpy(nh_data.nexthop_info.dipo,
                   &ip_pfx.addr.addr.v4_addr, IP4_ADDR8_LEN);
        } else if (ip_pfx.addr.af == IP_AF_IPV6) {
            nh_data.nexthop_info.ip_type = IPTYPE_IPV6;
            sdk::lib::memrev(nh_data.nexthop_info.dipo,
                             ip_pfx.addr.addr.v6_addr.addr8,
                             IP6_ADDR8_LEN);
        }
        sdk::lib::memrev(nh_data.nexthop_info.dmaco,
                         spec->underlay_mac, ETH_ADDR_LEN);
        // TODO: get this from the pinned mnic
        sdk::lib::memrev(nh_data.nexthop_info.smaco,
                         intf->l3_mac(), ETH_ADDR_LEN);
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP, hw_id_,
                                           NULL, NULL, &nh_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program nexthop %u at idx %u",
                          spec->key.id, hw_id_);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
        break;

    default:
        PDS_TRACE_ERR("Failed to program NEXTHOP %u, type %u at %u, err %u",
                      spec->key.id, spec->type, hw_id_, ret);
        return SDK_RET_INVALID_ARG;
        break;
    }
#endif
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::activate_delete_(pds_epoch_t epoch, nexthop *nh) {
    return SDK_RET_ERR;
}

sdk_ret_t
nexthop_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                          api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_nexthop_spec_t *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
        spec = &obj_ctxt->api_params->nexthop_spec;
        ret = activate_create_(epoch, (nexthop *)api_obj, spec);
        break;

    case api::API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (nexthop *)api_obj);
        break;

    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
nexthop_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                            api_op_t api_op) {
    return SDK_RET_OK;
}
void
nexthop_impl::fill_status_(pds_nexthop_status_t *status) {
    status->hw_id = hw_id_;
}

sdk_ret_t
nexthop_impl::fill_spec_(pds_nexthop_spec_t *spec) {
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t rv;
    pds_nexthop_info_t *nh_info = (pds_nexthop_info_t *)info;

    rv = fill_spec_(&nh_info->spec);
    if (unlikely(rv != sdk::SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read NEXTHOP %s table entry",
                      api_obj->key2str().c_str());
        return rv;
    }
    fill_status_(&nh_info->status);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_NEXTHOP_IMPL

}    // namespace impl
}    // namespace api
