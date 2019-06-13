//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
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
#include "nic/apollo/api/impl/artemis/tep_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/p4/include/artemis_defines.h"
#include "gen/p4gen/artemis/include/p4pd.h"

// TODO: this MAC needs to be resolved by VPP or some app, until then we use
//       this hack
//       other approach is to have route->NH (of type ILB) and NH will have
//       Tunnel and DMAC of ILB
uint64_t g_ilb_mac = 0x000202020202;

namespace api {
namespace impl {

/// \defgroup PDS_TEP_IMPL - tep datapath implementation
/// \ingroup PDS_TEP
/// @{

tep_impl *
tep_impl::factory(pds_tep_spec_t *spec) {
    tep_impl *impl;

    if (spec->type != PDS_TEP_TYPE_SERVICE) {
        // other type of TEPs are not supported in this pipeline
        return NULL;
    }

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
tep_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t idx;
    pds_tep_spec_t *spec;
    tep1_rx_swkey_t tep1_rx_key = { 0 };
    tep1_rx_swkey_mask_t tep1_rx_mask = { 0 };
    sdk_table_api_params_t api_params;

    spec = &obj_ctxt->api_params->tep_spec;
    if (spec->type == PDS_TEP_TYPE_SERVICE) {
        // reserve an entry in REMOTE_46_TEP table for service type TEPs
        if (tep_impl_db()->remote_46_tep_idxr()->alloc(&idx) !=
                sdk::lib::indexer::SUCCESS) {
            PDS_TRACE_ERR("Failed to allocate hw id for TEP %s",
                          ipaddr2str(&spec->key.ip_addr));
            return sdk::SDK_RET_NO_RESOURCE;
        }
        remote46_hw_id_ = idx & 0xFFFF;

        // reserve an entry in the TEP1_RX table
        tep1_rx_key.vxlan_1_vni = spec->encap.val.value;
        tep1_rx_mask.vxlan_1_vni_mask = 0xFFFFFFFF;
        memset(&api_params, 0, sizeof(api_params));
        api_params.key = &tep1_rx_key;
        api_params.mask = &tep1_rx_mask;
        api_params.handle = sdk::table::handle_t::null();
        ret = tep_impl_db()->tep1_rx_tbl()->reserve(&api_params);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve entry in TEP1_RX "
                          "table for svc TEP %s, err %u",
                          orig_obj->key2str().c_str(), ret);
            goto error;
        }
        tep1_rx_handle_ = api_params.handle;
    }

    // reserve an entry in NEXTHOP table
    ret = nexthop_impl_db()->nh_tbl()->reserve(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in NH table for svc TEP %s, "
                      "err %u", orig_obj->key2str().c_str(), ret);
        return ret;
    }
    return SDK_RET_OK;

error:
    return ret;
}

sdk_ret_t
tep_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };

    if (remote46_hw_id_ != 0xFFFF) {
        tep_impl_db()->remote_46_tep_idxr()->free(remote46_hw_id_);
    }
    if (tep1_rx_handle_.valid()) {
        api_params.handle = tep1_rx_handle_;
        tep_impl_db()->tep1_rx_tbl()->release(&api_params);
    }
    if (nh_idx_ != 0xFFFFFFFF) {
        nexthop_impl_db()->nh_tbl()->release(nh_idx_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };

    if (remote46_hw_id_ != 0xFFFF) {
        tep_impl_db()->remote_46_tep_idxr()->free(remote46_hw_id_);
    }
    if (tep1_rx_handle_.valid()) {
        api_params.handle = tep1_rx_handle_;
        tep_impl_db()->tep1_rx_tbl()->remove(&api_params);
    }
    if (nh_idx_ != 0xFFFFFFFF) {
        nexthop_impl_db()->nh_tbl()->remove(nh_idx_);
    }
    return SDK_RET_OK;
}

#define tep1_rx_info      action_u.tep1_rx_tep1_rx_info
#define remote_46_info    action_u.remote_46_mapping_remote_46_info
#define nexthop_info      action_u.nexthop_nexthop_info
sdk_ret_t
tep_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    tep_entry *tep;
    p4pd_error_t p4pd_ret;
    pds_tep_spec_t *spec;
    sdk_table_api_params_t api_params;
    tep1_rx_swkey_t tep1_rx_key = { 0 };
    nexthop_actiondata_t nh_data = { 0 };
    tep1_rx_swkey_mask_t tep1_rx_mask = { 0 };
    tep1_rx_actiondata_t tep1_rx_data = { 0 };
    remote_46_mapping_actiondata_t remote_46_mapping_data = { 0 };

    spec = &obj_ctxt->api_params->tep_spec;
    tep = (tep_entry *)api_obj;
    switch (spec->type) {
    case PDS_TEP_TYPE_SERVICE:
        // program NEXTHOP table entry
        nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
        nh_data.nexthop_info.port = TM_PORT_UPLINK_1;
        nh_data.nexthop_info.vni = spec->encap.val.value;
        ret = nexthop_impl_db()->nh_tbl()->insert_atid(&nh_data, nh_idx_);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to program NEXTHOP table at %u for "
                          "service TEP %s, err %u", nh_idx_,
                          api_obj->key2str().c_str(), ret);
            return ret;
        }

        // program REMOTE_46_MAPPING table entry
        remote_46_mapping_data.action_id = REMOTE_46_MAPPING_REMOTE_46_INFO_ID;
        sdk::lib::memrev(remote_46_mapping_data.remote_46_info.ipv6_tx_da,
                         spec->ip_addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        remote_46_mapping_data.remote_46_info.nh_id = nh_idx_;
        p4pd_ret =
            p4pd_global_entry_write(P4_ARTEMIS_TXDMA_TBL_ID_REMOTE_46_MAPPING,
                                    remote46_hw_id_, NULL, NULL,
                                    &remote_46_mapping_data);

        if (unlikely(p4pd_ret != P4PD_SUCCESS)) {
            PDS_TRACE_ERR("TEP table programming failed for TEP %s, "
                          "TEP hw id %u", api_obj->key2str().c_str(),
                          remote46_hw_id_);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }

        // program TEP1_RX table entry
        tep1_rx_key.vxlan_1_vni = spec->encap.val.value;
        tep1_rx_mask.vxlan_1_vni_mask = 0xFFFFFFFF;
        tep1_rx_data.tep1_rx_info.decap_next = 0;
        // @pirabhu let us discuss what the vpc id should be here
        // if needed we may have to make vpc id in pds_tep_spec_t for service
        // tunnel mandatory
        //tep1_rx_data.tep1_rx_info.src_vpc_id = ;
        memset(&api_params, 0, sizeof(api_params));
        api_params.key = &tep1_rx_key;
        api_params.mask = &tep1_rx_mask;
        api_params.appdata = &tep1_rx_data;
        api_params.action_id = TEP1_RX_TEP1_RX_INFO_ID;
        api_params.handle = tep1_rx_handle_;
        ret = tep_impl_db()->tep1_rx_tbl()->insert(&api_params);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Programming of TEP1_RX table failed for svc TEP %s, "
                          "err %u", api_obj->key2str().c_str(), ret);
            return ret;
        }
        break;

    case PDS_TEP_TYPE_WORKLOAD:
        // program NEXTHOP table entry
        nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
        nh_data.nexthop_info.port = TM_PORT_UPLINK_1;
        nh_data.nexthop_info.vni = spec->encap.val.value;
        if (spec->key.ip_addr.af == IP_AF_IPV6) {
            nh_data.nexthop_info.ip_type = IPTYPE_IPV6;
            sdk::lib::memrev(nh_data.nexthop_info.dipo,
                             spec->key.ip_addr.addr.v6_addr.addr8,
                             IP6_ADDR8_LEN);
        } else {
            nh_data.nexthop_info.ip_type = IPTYPE_IPV4;
            memcpy(nh_data.nexthop_info.dipo, &spec->key.ip_addr.addr.v4_addr,
                   IP4_ADDR8_LEN);
        }
        sdk::lib::memrev(nh_data.nexthop_info.dmaco, spec->mac, ETH_ADDR_LEN);
        sdk::lib::memrev(nh_data.nexthop_info.dmaci,
                         (uint8_t *)&g_ilb_mac, ETH_ADDR_LEN);
        ret = nexthop_impl_db()->nh_tbl()->insert_atid(&nh_data, nh_idx_);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to program NEXTHOP table at %u for "
                          "service TEP %s, err %u", nh_idx_,
                          api_obj->key2str().c_str(), ret);
            return ret;
        }
        break;

    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }
    PDS_TRACE_DEBUG("Programmed TEP %s, hw id %u",
                    ipaddr2str(&spec->key.ip_addr), remote46_hw_id_);
    return ret;
}

sdk_ret_t
tep_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_OK;
}

sdk_ret_t
tep_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

void
tep_impl::fill_status_(pds_tep_status_t *status) {
    status->hw_id = remote46_hw_id_;
}

void
tep_impl::fill_spec_(remote_46_mapping_actiondata_t *data,
                     pds_tep_spec_t *spec) {
    spec->type = PDS_TEP_TYPE_SERVICE;
    spec->ip_addr.af = IP_AF_IPV6;
    sdk::lib::memrev(spec->ip_addr.addr.v6_addr.addr8,
                     data->action_u.remote_46_mapping_remote_46_info.ipv6_tx_da,
                     IP6_ADDR8_LEN);
}

sdk_ret_t
tep_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    remote_46_mapping_actiondata_t remote_46_mapping_data = { 0 };
    p4pd_error_t p4pdret;
    pds_tep_info_t *tep_info = (pds_tep_info_t *)info;

    p4pdret = p4pd_global_entry_read(P4_ARTEMIS_TXDMA_TBL_ID_REMOTE_46_MAPPING,
                                     remote46_hw_id_, NULL, NULL,
                                     &remote_46_mapping_data);
    if (unlikely(p4pdret != P4PD_SUCCESS)) {
        PDS_TRACE_ERR("Failed to read REMOTE_46_MAPPING table for TEP %s at "
                      "hw id %u, ret %d", api_obj->key2str().c_str(),
                      remote46_hw_id_, p4pdret);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    fill_spec_(&remote_46_mapping_data, &tep_info->spec);
    fill_status_(&tep_info->status);

    return sdk::SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
