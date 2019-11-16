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
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/artemis/tep_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/artemis_impl.hpp"
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
    tep2_rx_swkey_t tep2_rx_key = { 0 };
    tep1_rx_swkey_mask_t tep1_rx_mask = { 0 };
    tep2_rx_swkey_mask_t tep2_rx_mask = { 0 };
    sdk_table_api_params_t api_params;

    spec = &obj_ctxt->api_params->tep_spec;
    if (spec->type == PDS_TEP_TYPE_SERVICE) {
        // reserve an entry in REMOTE_46_TEP table for service type TEPs
        if (tep_impl_db()->remote_46_tep_idxr()->alloc(&idx) !=
                sdk::lib::indexer::SUCCESS) {
            PDS_TRACE_ERR("Failed to allocate hw id for TEP id %u, ip %s",
                          spec->key.id, ipaddr2str(&spec->remote_ip));
            return sdk::SDK_RET_NO_RESOURCE;
        }
        remote46_hw_id_ = idx & 0xFFFF;

        // reserve an entry in the TEP1_RX table
        if (spec->remote_svc) {
            tep1_rx_key.vxlan_1_vni = spec->remote_svc_encap.val.value;
        } else {
            tep1_rx_key.vxlan_1_vni = spec->encap.val.value;
        }
        tep1_rx_mask.vxlan_1_vni_mask = ~0;
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

        // reserve an entry in the TEP2_RX table for remote services
        if (spec->remote_svc) {
            tep2_rx_key.vxlan_2_vni = spec->encap.val.value;
            tep2_rx_mask.vxlan_2_vni_mask = ~0;
            memcpy(tep2_rx_key.tunnel_metadata_tep2_dst,
                   &spec->remote_ip.addr.v6_addr.addr32[3], IP4_ADDR8_LEN);
            memset(tep2_rx_mask.tunnel_metadata_tep2_dst_mask, 0xFF,
                   sizeof(tep2_rx_mask.tunnel_metadata_tep2_dst_mask));
            memset(&api_params, 0, sizeof(api_params));
            api_params.key = &tep2_rx_key;
            api_params.mask = &tep2_rx_mask;
            api_params.handle = sdk::table::handle_t::null();
            ret = tep_impl_db()->tep2_rx_tbl()->reserve(&api_params);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to reserve entry in TEP2_RX "
                              "table for svc TEP %s, err %u",
                              orig_obj->key2str().c_str(), ret);
                goto error;
            }
            tep2_rx_handle_ = api_params.handle;
        }
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
    if (tep2_rx_handle_.valid()) {
        api_params.handle = tep2_rx_handle_;
        tep_impl_db()->tep2_rx_tbl()->release(&api_params);
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
    if (tep2_rx_handle_.valid()) {
        api_params.handle = tep2_rx_handle_;
        tep_impl_db()->tep2_rx_tbl()->remove(&api_params);
    }
    if (nh_idx_ != 0xFFFFFFFF) {
        nexthop_impl_db()->nh_tbl()->remove(nh_idx_);
    }
    return SDK_RET_OK;
}

#define tep1_rx_info      action_u.tep1_rx_tep1_rx_info
#define tep2_rx_info      action_u.tep2_rx_tep2_rx_info
#define remote_46_info    action_u.remote_46_mapping_remote_46_info
#define nexthop_info      action_u.nexthop_nexthop_info
sdk_ret_t
tep_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    tep_entry *tep;
    pds_tep_spec_t *spec;
    p4pd_error_t p4pd_ret;
    ipv4_addr_t st_v4_dipo;
    sdk_table_api_params_t api_params;
    tep1_rx_swkey_t tep1_rx_key = { 0 };
    tep2_rx_swkey_t tep2_rx_key = { 0 };
    nexthop_actiondata_t nh_data = { 0 };
    tep1_rx_swkey_mask_t tep1_rx_mask = { 0 };
    tep2_rx_swkey_mask_t tep2_rx_mask = { 0 };
    tep1_rx_actiondata_t tep1_rx_data = { 0 };
    tep2_rx_actiondata_t tep2_rx_data = { 0 };
    remote_46_mapping_actiondata_t remote_46_mapping_data = { 0 };

    spec = &obj_ctxt->api_params->tep_spec;
    tep = (tep_entry *)api_obj;
    switch (spec->type) {
    case PDS_TEP_TYPE_SERVICE:
        PDS_TRACE_DEBUG("Programming service TEP %s, DIPo %s, mac %s remote %s,"
                        " public IP %s, nh hw id %u, remote_46 hw id %u",
                        api_obj->key2str().c_str(),
                        ipv4addr2str(spec->remote_ip.addr.v6_addr.addr32[3]),
                        macaddr2str(spec->mac),
                        spec->remote_svc ? "true" : "false",
                        ipv4addr2str(spec->remote_svc_public_ip.addr.v4_addr),
                        nh_idx_, remote46_hw_id_);
        // program NEXTHOP table entry
        nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
        nh_data.nexthop_info.port = TM_PORT_UPLINK_1;
        nh_data.nexthop_info.vni = spec->encap.val.value;
        if (spec->remote_svc) {
            memcpy(nh_data.nexthop_info.dipo,
                   &spec->remote_svc_public_ip.addr.v4_addr, IP4_ADDR8_LEN);
        } else {
            st_v4_dipo = ntohl(spec->remote_ip.addr.v6_addr.addr32[3]);
            memcpy(nh_data.nexthop_info.dipo, &st_v4_dipo, IP4_ADDR8_LEN);
        }
        sdk::lib::memrev(nh_data.nexthop_info.dmaco, spec->mac, ETH_ADDR_LEN);
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
                         spec->remote_ip.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        remote_46_mapping_data.remote_46_info.nh_id = nh_idx_;
        p4pd_ret =
            p4pd_global_entry_write(P4_P4PLUS_TXDMA_TBL_ID_REMOTE_46_MAPPING,
                                    remote46_hw_id_, NULL, NULL,
                                    &remote_46_mapping_data);

        if (unlikely(p4pd_ret != P4PD_SUCCESS)) {
            PDS_TRACE_ERR("TEP table programming failed for TEP %s, "
                          "TEP hw id %u", api_obj->key2str().c_str(),
                          remote46_hw_id_);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }

        tep1_rx_data.action_id = TEP1_RX_TEP1_RX_INFO_ID;
        if (spec->remote_svc) {
            // program TEP1_RX table entry with remote service TEP's encap
            tep1_rx_key.vxlan_1_vni = spec->remote_svc_encap.val.value;
            tep1_rx_data.tep1_rx_info.decap_next = 1;
        } else {
            // program TEP1_RX table entry
            tep1_rx_key.vxlan_1_vni = spec->encap.val.value;
            tep1_rx_data.tep1_rx_info.decap_next = 0;
        }
        tep1_rx_mask.vxlan_1_vni_mask = ~0;
        tep1_rx_data.tep1_rx_info.src_vpc_id =
            PDS_IMPL_SERVICE_TUNNEL_VPC_HW_ID;
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

        // in case of remote service TEP, we need to program TEP2 table as well
        if (spec->remote_svc) {
            tep2_rx_key.vxlan_2_vni = spec->encap.val.value;
            tep2_rx_mask.vxlan_2_vni_mask = ~0;
            memcpy(tep2_rx_key.tunnel_metadata_tep2_dst,
                   &spec->remote_svc_public_ip.addr.v4_addr,
                   IP4_ADDR8_LEN);
            memset(tep2_rx_mask.tunnel_metadata_tep2_dst_mask, 0xFF,
                   sizeof(tep2_rx_mask.tunnel_metadata_tep2_dst_mask));
            tep2_rx_data.action_id = TEP2_RX_TEP2_RX_INFO_ID;
            tep2_rx_data.tep2_rx_info.src_vpc_id =
                PDS_IMPL_SERVICE_TUNNEL_VPC_HW_ID;
            memset(&api_params, 0, sizeof(api_params));
            api_params.key = &tep2_rx_key;
            api_params.mask = &tep2_rx_mask;
            api_params.appdata = &tep2_rx_data;
            api_params.action_id = TEP2_RX_TEP2_RX_INFO_ID;
            api_params.handle = tep2_rx_handle_;
            ret = tep_impl_db()->tep2_rx_tbl()->insert(&api_params);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Programming of TEP2_RX table failed for remote "
                              "svc TEP %s, err %u",
                              api_obj->key2str().c_str(), ret);
                return ret;
            }
        }
        break;

    case PDS_TEP_TYPE_WORKLOAD:
        // program NEXTHOP table entry
        PDS_TRACE_DEBUG("Programming workload TEP %s, nh hw id %u\n",
                        api_obj->key2str().c_str(), nh_idx_);
        nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
        nh_data.nexthop_info.port = TM_PORT_UPLINK_1;
        nh_data.nexthop_info.vni = spec->encap.val.value;
        if (spec->remote_ip.af == IP_AF_IPV6) {
            nh_data.nexthop_info.ip_type = IPTYPE_IPV6;
            sdk::lib::memrev(nh_data.nexthop_info.dipo,
                             spec->remote_ip.addr.v6_addr.addr8,
                             IP6_ADDR8_LEN);
        } else {
            nh_data.nexthop_info.ip_type = IPTYPE_IPV4;
            memcpy(nh_data.nexthop_info.dipo, &spec->remote_ip.addr.v4_addr,
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
    PDS_TRACE_DEBUG("Programmed TEP id %u, ip %s, hw id %u",
                    spec->key.id, ipaddr2str(&spec->remote_ip),
                    remote46_hw_id_);
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

sdk_ret_t
tep_impl::fill_spec_(pds_tep_spec_t *spec) {
    remote_46_mapping_actiondata_t remote_46_mapping_data = { 0 };
    p4pd_error_t p4pdret;

    p4pdret = p4pd_global_entry_read(P4_P4PLUS_TXDMA_TBL_ID_REMOTE_46_MAPPING,
                                     remote46_hw_id_, NULL, NULL,
                                     &remote_46_mapping_data);
    if (unlikely(p4pdret != P4PD_SUCCESS)) {
        PDS_TRACE_ERR("p4 global entry read failed for hw id %u, ret %d",
                      remote46_hw_id_, p4pdret);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    spec->type = PDS_TEP_TYPE_SERVICE;
    spec->remote_ip.af = IP_AF_IPV6;
    sdk::lib::memrev(spec->remote_ip.addr.v6_addr.addr8,
                     remote_46_mapping_data.action_u.
                     remote_46_mapping_remote_46_info.ipv6_tx_da,
                     IP6_ADDR8_LEN);

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
