//
// Copyright (c) 2018 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of mapping
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/tep_impl.hpp"
#include "nic/apollo/api/impl/apollo/vnic_impl.hpp"
#include "nic/apollo/api/impl/apollo/mapping_impl.hpp"
#include "nic/apollo/api/impl/apollo/pds_impl_state.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

using sdk::table::sdk_table_api_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL - mapping entry datapath implementation
/// \ingroup PDS_MAPPING
/// @{

#define nat_action                    action_u.nat_nat
#define nh_action                     action_u.nexthop_nexthop_info
#define tep_mpls_udp_action           action_u.tep_mpls_udp_tep
#define tep_ipv4_vxlan_action         action_u.tep_ipv4_vxlan_tep
#define local_vnic_by_slot_rx_info    action_u.local_vnic_by_slot_rx_local_vnic_info_rx

pds_encap_t mapping_impl::mytep_encap_;

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
mapping_impl::build(pds_mapping_key_t *key, mapping_entry *mapping) {
    sdk_ret_t                             ret;
    device_entry                          *device;
    vpc_entry                             *vpc;
    mapping_impl                          *impl;
    uint16_t                              vnic_hw_id;
    bool                                  local_mapping = false;
    sdk_table_api_params_t                api_params;
    sdk_table_api_params_t                slot_api_params = { 0 };
    remote_vnic_mapping_tx_swkey_t        remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_tx_appdata_t      remote_vnic_mapping_tx_data = { 0 };
    remote_vnic_mapping_rx_swkey_t        remote_vnic_mapping_rx_key = { 0 };
    remote_vnic_mapping_rx_appdata_t      remote_vnic_mapping_rx_data = { 0 };
    local_ip_mapping_swkey_t              local_ip_mapping_key = { 0 };
    local_ip_mapping_appdata_t            local_ip_mapping_data = { 0 };
    local_vnic_by_slot_rx_swkey_t         local_vnic_by_slot_rx_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t    local_vnic_by_slot_rx_data = { 0 };
    local_vnic_by_slot_rx_swkey_t         local_vnic_by_slot_rx_key_mask = { 0 };
    nexthop_actiondata_t                  nh_data;
    tep_actiondata_t                      tep_data;
    nat_actiondata_t                      nat_data;
    handle_t                              remote_vnic_tx_hdl;

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

    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vpc->hw_id(), &key->ip_addr,
                                               true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                   NULL, &remote_vnic_mapping_tx_data,
                                   REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->get(&api_params);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }
    impl->vpc_hw_id_ = vpc->hw_id();
    memcpy(&impl->key_, key, sizeof(impl->key_));
    remote_vnic_tx_hdl = api_params.handle;

    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                        remote_vnic_mapping_tx_data.nexthop_group_index,
                        &nh_data, NULL);
    ret = nexthop_impl_db()->nh_tbl()->get(&api_params);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                        nh_data.action_u.nexthop_nexthop_info.tep_index,
                        &tep_data, NULL);
    ret = tep_impl_db()->tep_tbl()->get(&api_params);

    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    if (tep_data.action_id == TEP_MPLS_UDP_TEP_ID) {
        if (tep_data.tep_mpls_udp_action.dipo ==
                device->ip_addr().addr.v4_addr) {
            local_mapping = true;
        }
    } else if (tep_data.action_id == TEP_IPV4_VXLAN_TEP_ID) {
        if (tep_data.tep_ipv4_vxlan_action.dipo ==
                device->ip_addr().addr.v4_addr) {
            local_mapping = true;
        }
    }

    if (local_mapping == false) {
        if (tep_data.action_id == TEP_MPLS_UDP_TEP_ID) {
            remote_vnic_mapping_rx_key.vnic_metadata_src_slot_id =
                remote_vnic_mapping_tx_data.dst_slot_id;
            remote_vnic_mapping_rx_key.ipv4_1_srcAddr =
                tep_data.tep_mpls_udp_action.dipo;
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params,
                &remote_vnic_mapping_rx_key, NULL,
                &remote_vnic_mapping_rx_data,
                REMOTE_VNIC_MAPPING_RX_REMOTE_VNIC_MAPPING_RX_INFO_ID,
                sdk::table::handle_t::null());
            ret = mapping_impl_db()->remote_vnic_mapping_rx_tbl()->get(&api_params);
            if (ret != SDK_RET_OK) {
                goto error;
            }
            impl->fabric_encap_.val.value = remote_vnic_mapping_rx_key.vnic_metadata_src_slot_id;
            // impl->tep_ip_.addr.v4_addr = tep_data.tep_mpls_udp_action.dipo;
            impl->handle_.remote_.remote_vnic_rx_hdl_ = api_params.handle;
        }
        impl->handle_.remote_.remote_vnic_tx_hdl_ = remote_vnic_tx_hdl;
        return impl;
    }

    // for local mappings, we have to look at more tables
    impl->is_local_ = true;
    impl->handle_.local_.overlay_ip_remote_vnic_tx_hdl_ = remote_vnic_tx_hdl;
    if (mytep_encap_.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        local_vnic_by_slot_rx_key.mpls_dst_label =
            remote_vnic_mapping_tx_data.dst_slot_id;
        local_vnic_by_slot_rx_key_mask.mpls_dst_label = ~0;
        local_vnic_by_slot_rx_key.vxlan_1_vni = 0;
    } else {
        local_vnic_by_slot_rx_key.mpls_dst_label = 0;
        local_vnic_by_slot_rx_key.vxlan_1_vni =
            remote_vnic_mapping_tx_data.dst_slot_id;
        local_vnic_by_slot_rx_key_mask.vxlan_1_vni = ~0;
    }

    PDS_IMPL_FILL_TABLE_API_PARAMS(&slot_api_params, &local_vnic_by_slot_rx_key,
                                   NULL, &local_vnic_by_slot_rx_data,
                                   LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID,
                                   sdk::table::handle_t::null());
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->get(&slot_api_params);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    vnic_hw_id = local_vnic_by_slot_rx_data.local_vnic_by_slot_rx_info.local_vnic_tag;
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key, vnic_hw_id,
                                         &key->ip_addr, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key,
                                   NULL, &local_ip_mapping_data,
                                   LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_ip_mapping_tbl()->get(&api_params);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    impl->vnic_hw_id_ = vnic_hw_id;
    memcpy(&impl->key_, key, sizeof(impl->key_));
    impl->handle_.local_.overlay_ip_hdl_ =  api_params.handle;

    if (local_ip_mapping_data.xlate_index != NAT_TX_TBL_RSVD_ENTRY_IDX) {
        // we have public IP for this mapping
        impl->handle_.local_.overlay_ip_to_public_ip_nat_hdl_ =
            local_ip_mapping_data.xlate_index;
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                              local_ip_mapping_data.xlate_index,
                                              &nat_data, NULL);
        ret = mapping_impl_db()->nat_tbl()->get(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
        local_ip_mapping_key.key_metadata_lkp_id = vnic_hw_id;
        memcpy(local_ip_mapping_key.control_metadata_mapping_lkp_addr,
               nat_data.nat_action.nat_ip, IP6_ADDR8_LEN);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key,
                                       NULL, &local_ip_mapping_data,
                                       LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_ip_mapping_tbl()->get(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
        impl->handle_.local_.public_ip_hdl_ = api_params.handle;
        // As public IPs are V4, can copy it without memrev
        impl->public_ip_.af = IP_AF_IPV4;
        memcpy(&impl->public_ip_.addr.v4_addr,
               nat_data.nat_action.nat_ip, IP4_ADDR8_LEN);
        impl->handle_.local_.public_ip_to_overlay_ip_nat_hdl_ =
            local_ip_mapping_data.xlate_index;

        remote_vnic_mapping_tx_key.txdma_to_p4e_header_vpc_id = vpc->hw_id();
        memcpy(remote_vnic_mapping_tx_key.p4e_apollo_i2e_dst,
               nat_data.nat_action.nat_ip, IP6_ADDR8_LEN);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                       NULL, &remote_vnic_mapping_tx_data,
                                       REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->get(&api_params);
        if (unlikely(ret != SDK_RET_OK)) {
            goto error;
        }
        impl->handle_.local_.public_ip_remote_vnic_tx_hdl_ = api_params.handle;
    }
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
mapping_impl::reserve_local_ip_mapping_resources_(api_base *api_obj,
                                                  vpc_entry *vpc,
                                                  pds_mapping_spec_t *spec) {
    sdk_ret_t                         ret = SDK_RET_OK;
    vnic_impl                         *vnic_impl_obj;
    local_ip_mapping_swkey_t          local_ip_mapping_key = { 0 };
    remote_vnic_mapping_tx_swkey_t    remote_vnic_mapping_tx_key = { 0 };
    sdk_table_api_params_t            tparams;

    vnic_impl_obj = (vnic_impl *)vnic_db()->find(&spec->vnic)->impl();

    // reserve an entry in LOCAL_IP_MAPPING table for overlay IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vnic_impl_obj->hw_id(),
                                         &spec->skey.ip_addr, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_ip_mapping_key,
                                   NULL, NULL, 0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_ip_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_IP_MAPPING table "
                      "for mapping %s, err %u", api_obj->key2str().c_str(),
                      ret);
        return ret;
    }
    handle_.local_.overlay_ip_hdl_ = tparams.handle;

    // reserve an entry in REMOTE_VNIC_MAPPING_TX for overlay IP
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vpc->hw_id(),
                                               &spec->skey.ip_addr, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_tx_key,
                                   NULL, NULL, 0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in REMOTE_VNIC_MAPPING_TX "
                      "table for mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    handle_.local_.overlay_ip_remote_vnic_tx_hdl_ = tparams.handle;

    PDS_TRACE_DEBUG("Rsvd overlay_ip_hdl 0x%lx, "
                    "overlay_ip_remote_vnic_tx_hdl 0x%lx",
                    handle_.local_.overlay_ip_hdl_,
                    handle_.local_.overlay_ip_remote_vnic_tx_hdl_);

    // check if this mapping has public IP
    if (!spec->public_ip_valid) {
        return SDK_RET_OK;
    }

    // reserve an entry in LOCAL_IP_MAPPING table for public IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vnic_impl_obj->hw_id(),
                                         &spec->public_ip, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_ip_mapping_key,
                                   NULL, NULL, 0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_ip_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_IP_MAPPING table "
                      "for public IP of mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    handle_.local_.public_ip_hdl_ = tparams.handle;

    // reserve an entry in REMOTE_VNIC_MAPPING_TX for public IP
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vpc->hw_id(), &spec->public_ip,
                                               true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_tx_key,
                                   NULL, NULL, 0, sdk::table::handle_t::null());
    ret =
        mapping_impl_db()->remote_vnic_mapping_tx_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in REMOTE_VNIC_MAPPING_TX "
                      "table for public IP of mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    handle_.local_.public_ip_remote_vnic_tx_hdl_ = tparams.handle;

    PDS_TRACE_DEBUG("Rsvd public_ip_hdl 0x%lx, "
                    "public_ip_remote_vnic_tx_hdl 0x%x",
                    handle_.local_.public_ip_hdl_,
                    handle_.local_.public_ip_remote_vnic_tx_hdl_);

    // reserve an entry for overlay IP to public IP xlation in NAT_TX table
    ret = mapping_impl_db()->nat_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in NAT_TX table for mapping %s, "
                      "err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }
    handle_.local_.overlay_ip_to_public_ip_nat_hdl_ = tparams.handle.pindex();

    // reserve an entry for public IP to overlay IP xlation in NAT_TX table
    ret = mapping_impl_db()->nat_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in NAT_TX table for mapping %s, "
                      "err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }
    handle_.local_.public_ip_to_overlay_ip_nat_hdl_ = tparams.handle.pindex();
    PDS_TRACE_DEBUG("Rsvd overlay_ip_to_public_ip_nat_hdl 0x%lx, "
                    "public_ip_to_overlay_ip_nat_hdl_ 0x%lx",
                    handle_.local_.overlay_ip_to_public_ip_nat_hdl_,
                    handle_.local_.public_ip_to_overlay_ip_nat_hdl_);

    return SDK_RET_OK;

error:

    // TODO: release all allocated resources
    return ret;
}

sdk_ret_t
mapping_impl::reserve_remote_ip_mapping_resources_(api_base *api_obj,
                                                   vpc_entry *vpc,
                                                   pds_mapping_spec_t *spec) {
    sdk_ret_t                         ret;
    remote_vnic_mapping_tx_swkey_t    remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_rx_swkey_t    remote_vnic_mapping_rx_key = { 0 };
    sdk_table_api_params_t            tparams;

    // reserve an entry in REMOTE_VNIC_MAPPING_TX table
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vpc->hw_id(),
                                               &spec->skey.ip_addr, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_tx_key,
                                   NULL, NULL, 0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in REMOTE_VNIC_MAPPING_TX "
                      "table for mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    handle_.remote_.remote_vnic_tx_hdl_ = tparams.handle;

    // REMOTE_VNIC_MAPPING_RX table is not used for non MPLSoUDP encaps
    if (spec->fabric_encap.type != PDS_ENCAP_TYPE_MPLSoUDP) {
        return SDK_RET_OK;
    }

    // reserve an entry in REMOTE_VNIC_MAPPING_RX table
    remote_vnic_mapping_rx_key.vnic_metadata_src_slot_id =
        spec->fabric_encap.val.mpls_tag;
    remote_vnic_mapping_rx_key.ipv4_1_srcAddr = tep_ipaddr(spec).addr.v4_addr;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_rx_key,
                                   NULL, NULL, 0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->remote_vnic_mapping_rx_tbl()->reserve(&tparams);
    // TODO: key here is (slot, SIPo), so we need to handle the
    // refcounts, if there are multiple inserts with same key (e.g.,
    // dual-stack case or remote VNIC with multiple IPs etc.)
    if ((ret != SDK_RET_OK) && (ret != sdk::SDK_RET_ENTRY_EXISTS)) {
        PDS_TRACE_ERR("Failed to reserve entry in "
                      "REMOTE_VNIC_MAPPING_RX table for mapping %s, "
                      "err %u", api_obj->key2str().c_str(), ret);
        return ret;
    }
    handle_.remote_.remote_vnic_rx_hdl_ = tparams.handle;

    return SDK_RET_OK;

error:

    return ret;
}

sdk_ret_t
mapping_impl::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    pds_mapping_spec_t    *spec;
    vpc_entry             *vpc;

    spec = &obj_ctxt->api_params->mapping_spec;
    vpc = vpc_db()->find(&spec->skey.vpc);

    PDS_TRACE_DEBUG("Reserving resources for mapping (vpc %s, ip %s), "
                    "local %u, subnet %s, tep %s, vnic %s, "
                    "pub_ip_valid %u, pub_ip %s, "
                    "prov_ip_valid %u, prov_ip %s",
                    spec->skey.vpc.str(), ipaddr2str(&spec->skey.ip_addr),
                    is_local_, spec->subnet.str(), spec->tep.str(),
                    spec->vnic.str(), spec->public_ip_valid,
                    ipaddr2str(&spec->public_ip),
                    spec->provider_ip_valid, ipaddr2str(&spec->provider_ip));

    if (is_local_) {
        return reserve_local_ip_mapping_resources_(orig_obj, vpc, spec);
    }
    return reserve_remote_ip_mapping_resources_(orig_obj, vpc, spec);
}

sdk_ret_t
mapping_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t         api_params;
    sdk_ret_t ret = SDK_RET_OK;
    local_ip_mapping_swkey_t       local_ip_mapping_key = { 0 };
    remote_vnic_mapping_tx_swkey_t remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_rx_swkey_t remote_vnic_mapping_rx_key = { 0 };

    if (is_local_) {
        if (handle_.local_.overlay_ip_hdl_.valid()) {
            PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                                 vnic_hw_id_,
                                                 &key_.ip_addr, true);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key,
                                           NULL, NULL, 0, sdk::table::handle_t::null());
            ret = mapping_impl_db()->local_ip_mapping_tbl()->remove(&api_params);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to remove entry in LOCAL_IP_MAPPING table"
                              " for mapping %s, err %u",
                              api_obj->key2str().c_str(), ret);
                goto err;
            }
        }
        if (handle_.local_.overlay_ip_remote_vnic_tx_hdl_.valid()) {
            PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                                       vpc_hw_id_,
                                                       &key_.ip_addr, true);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                           NULL, NULL, 0, sdk::table::handle_t::null());
            ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->remove(&api_params);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to remove entry in REMOTE_VNIC_MAPPING_TX"
                              " table for mapping %s, err %u",
                              api_obj->key2str().c_str(), ret);
                goto err;
            }
        }
        if (handle_.local_.public_ip_hdl_.valid()) {
            PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                                 vnic_hw_id_,
                                                 &public_ip_, true);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key,
                                           NULL, NULL, 0, sdk::table::handle_t::null());
            ret = mapping_impl_db()->local_ip_mapping_tbl()->remove(&api_params);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to remove entry in LOCAL_IP_MAPPING table"
                              " for mapping %s, err %u",
                              api_obj->key2str().c_str(), ret);
                goto err;
            }
        }
        if (handle_.local_.public_ip_remote_vnic_tx_hdl_.valid()) {
            PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                                       vpc_hw_id_,
                                                       &public_ip_, true);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                           NULL, NULL, 0, sdk::table::handle_t::null());
            ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->remove(&api_params);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to remove entry in REMOTE_VNIC_MAPPING_TX"
                              " table for mapping %s, err %u",
                              api_obj->key2str().c_str(), ret);
                goto err;
            }
        }
        if (handle_.local_.overlay_ip_to_public_ip_nat_hdl_) {
            PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                                  handle_.local_.overlay_ip_to_public_ip_nat_hdl_,
                                                  NULL, NULL);
            mapping_impl_db()->nat_tbl()->remove(&api_params);
        }
        if (handle_.local_.public_ip_to_overlay_ip_nat_hdl_) {
            PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                                  handle_.local_.public_ip_to_overlay_ip_nat_hdl_,
                                                  NULL, NULL);
            mapping_impl_db()->nat_tbl()->remove(&api_params);
        }
    } else {
        if (handle_.remote_.remote_vnic_tx_hdl_.valid()) {
            PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                                       vpc_hw_id_,
                                                       &key_.ip_addr, true);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                           NULL, NULL, 0, sdk::table::handle_t::null());
            ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->remove(&api_params);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to remove entry in REMOTE_VNIC_MAPPING_TX"
                              " table for mapping %s, err %u",
                              api_obj->key2str().c_str(), ret);
                goto err;
            }
        }
        if (handle_.remote_.remote_vnic_rx_hdl_.valid()) {
            remote_vnic_mapping_rx_key.vnic_metadata_src_slot_id = fabric_encap_.val.mpls_tag;
#if 0
            if (is_local()) {
                remote_vnic_mapping_rx_key.ipv4_1_srcAddr = device_db()->find()->ip_addr().addr.v4_addr;
            } else {
                remote_vnic_mapping_rx_key.ipv4_1_srcAddr = tep_ip_.addr.v4_addr;
            }
#endif
            PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_rx_key,
                                           NULL, NULL, 0, sdk::table::handle_t::null());
            ret = mapping_impl_db()->remote_vnic_mapping_rx_tbl()->remove(&api_params);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to remove entry in "
                              "REMOTE_VNIC_MAPPING_RX table for mapping %s, "
                              "err %u", api_obj->key2str().c_str(), ret);
            }
        }
    }
err:
    return ret;
}

sdk_ret_t
mapping_impl::release_local_ip_mapping_resources_(api_base *api_obj) {
    sdk_table_api_params_t    api_params = { 0 };

    if (handle_.local_.overlay_ip_hdl_.valid()) {
        api_params.handle = handle_.local_.overlay_ip_hdl_;
        mapping_impl_db()->local_ip_mapping_tbl()->release(&api_params);
    }
    if (handle_.local_.overlay_ip_remote_vnic_tx_hdl_.valid()) {
        api_params.handle = handle_.local_.overlay_ip_remote_vnic_tx_hdl_;
        mapping_impl_db()->remote_vnic_mapping_tx_tbl()->release(&api_params);
    }
    if (handle_.local_.public_ip_hdl_.valid()) {
        api_params.handle = handle_.local_.public_ip_hdl_;
        mapping_impl_db()->local_ip_mapping_tbl()->release(&api_params);
    }
    if (handle_.local_.public_ip_remote_vnic_tx_hdl_.valid()) {
        api_params.handle = handle_.local_.public_ip_remote_vnic_tx_hdl_;
        mapping_impl_db()->remote_vnic_mapping_tx_tbl()->release(&api_params);
    }

    if (handle_.local_.overlay_ip_to_public_ip_nat_hdl_) {
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                              handle_.local_.overlay_ip_to_public_ip_nat_hdl_,
                                              NULL, NULL);
        mapping_impl_db()->nat_tbl()->release(&api_params);
    }
    if (handle_.local_.public_ip_to_overlay_ip_nat_hdl_) {
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                              handle_.local_.public_ip_to_overlay_ip_nat_hdl_,
                                              NULL, NULL);
        mapping_impl_db()->nat_tbl()->release(&api_params);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_remote_ip_mapping_resources_(api_base *api_obj) {
    sdk_table_api_params_t    api_params = { 0 };

    if (handle_.remote_.remote_vnic_tx_hdl_.valid()) {
        api_params.handle = handle_.remote_.remote_vnic_tx_hdl_;
        mapping_impl_db()->remote_vnic_mapping_tx_tbl()->release(&api_params);
    }
    if (handle_.remote_.remote_vnic_rx_hdl_.valid()) {
        api_params.handle = handle_.remote_.remote_vnic_rx_hdl_;
        mapping_impl_db()->remote_vnic_mapping_rx_tbl()->release(&api_params);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_resources(api_base *api_obj) {
    if (is_local_) {
        return release_local_ip_mapping_resources_(api_obj);
    }
    return release_remote_ip_mapping_resources_(api_obj);
}

sdk_ret_t
mapping_impl::add_remote_vnic_mapping_tx_entries_(vpc_entry *vpc,
                                                  pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    remote_vnic_mapping_tx_swkey_t remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_tx_appdata_t remote_vnic_mapping_tx_data = { 0 };
    tep_impl *tep_impl_obj = NULL;
    sdk_table_api_params_t api_params;
    uint32_t nh_id;

    if (is_local()) {
        nh_id = PDS_IMPL_MYTEP_NEXTHOP_HW_ID;
    } else {
        tep_impl_obj = (tep_impl *)tep_db()->find(&spec->tep)->impl();
        nh_id = tep_impl_obj->nh_id();
    }

    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vpc->hw_id(),
                                               &spec->skey.ip_addr, true);
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_APPDATA(&remote_vnic_mapping_tx_data,
                                                 nh_id,
                                                 &spec->fabric_encap);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                        NULL, &remote_vnic_mapping_tx_data,
                        REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID,
                        is_local_ ?
                            handle_.local_.overlay_ip_remote_vnic_tx_hdl_ :
                            handle_.remote_.remote_vnic_tx_hdl_);
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program entry in REMOTE_VNIC_MAPPING_TX "
                      "table for (vpc %u, IP %s), err %u\n", vpc->hw_id(),
                      ipaddr2str(&spec->skey.ip_addr), ret);
        goto error;
    }

    PDS_TRACE_DEBUG("Programmed REMOTE_VNIC_MAPPING_TX table entry "
                    "TEP %s, vpc hw id %u, encap type %u, encap val %u "
                    "nh id %u", spec->tep.str(),
                    vpc->hw_id(), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, nh_id);

    if (!is_local_ || !spec->public_ip_valid) {
        return SDK_RET_OK;
    }

    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vpc->hw_id(), &spec->public_ip,
                                               true);
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_APPDATA(&remote_vnic_mapping_tx_data,
                                                 nh_id,
                                                 &spec->fabric_encap);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                   NULL, &remote_vnic_mapping_tx_data,
                                   REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID,
                                   handle_.local_.public_ip_remote_vnic_tx_hdl_);
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add program entry in REMOTE_VNIC_MAPPING_TX "
                      "table for (vpc %u, IP %s), err %u\n", vpc->hw_id(),
                      ipaddr2str(&spec->public_ip), ret);
        goto error;
    }
    return SDK_RET_OK;

error:
    return ret;
}

sdk_ret_t
mapping_impl::add_nat_entries_(pds_mapping_spec_t *spec) {
    sdk_ret_t           ret;
    nat_actiondata_t    nat_data = { 0 };
    sdk_table_api_params_t api_params;

    // allocate NAT table entries
    if (spec->public_ip_valid) {
        // add private to public IP xlation NAT entry
        PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->public_ip);
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                handle_.local_.overlay_ip_to_public_ip_nat_hdl_,
                                &nat_data, NULL);
        ret = mapping_impl_db()->nat_tbl()->insert_atid(&api_params);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        // add public to private IP xlation NAT entry
        PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->skey.ip_addr);
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                handle_.local_.public_ip_to_overlay_ip_nat_hdl_,
                                &nat_data, NULL);
        ret = mapping_impl_db()->nat_tbl()->insert_atid(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:
    // TODO: handle cleanup in case of failure
    return ret;
}

sdk_ret_t
mapping_impl::add_local_ip_mapping_entries_(vpc_entry *vpc,
                                            pds_mapping_spec_t *spec) {
    sdk_ret_t                           ret;
    vnic_impl                           *vnic_impl_obj;
    local_ip_mapping_swkey_t            local_ip_mapping_key = { 0 };
    local_ip_mapping_appdata_t          local_ip_mapping_data = { 0 };
    remote_vnic_mapping_tx_swkey_t      remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_tx_appdata_t    remote_vnic_mapping_tx_data = { 0 };
    sdk_table_api_params_t              api_params;

    // add entry to LOCAL_IP_MAPPING table for overlay IP
    vnic_impl_obj = (vnic_impl *)vnic_db()->find(&spec->vnic)->impl();
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vnic_impl_obj->hw_id(),
                                         &spec->skey.ip_addr, true);
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_ip_mapping_data, vpc->hw_id(),
                                           handle_.local_.overlay_ip_to_public_ip_nat_hdl_,
                                           IP_TYPE_OVERLAY);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key,
                                   NULL, &local_ip_mapping_data,
                                   LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                   handle_.local_.overlay_ip_hdl_);
    ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    // add entry to LOCAL_IP_MAPPING table for public IP
    if (spec->public_ip_valid) {
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                             vnic_impl_obj->hw_id(),
                                             &spec->public_ip, true);
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_ip_mapping_data,
                                               vpc->hw_id(),
                                               handle_.local_.public_ip_to_overlay_ip_nat_hdl_,
                                               IP_TYPE_PUBLIC);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params,
                                       &local_ip_mapping_key,
                                       NULL, &local_ip_mapping_data,
                                       LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                       handle_.local_.public_ip_hdl_);
        ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    // add entry to REMOTE_VNIC_MAPPING_TX table for overlay & public IPs
    return add_remote_vnic_mapping_tx_entries_(vpc, spec);

error:
    // TODO: handle cleanup in case of failure
    return ret;
}

sdk_ret_t
mapping_impl::add_remote_vnic_mapping_rx_entries_(vpc_entry *vpc,
                                                  subnet_entry *subnet,
                                                  pds_mapping_spec_t *spec) {
    sdk_ret_t                         ret;
    remote_vnic_mapping_rx_swkey_t remote_vnic_mapping_rx_key = { 0 };
    remote_vnic_mapping_rx_appdata_t remote_vnic_mapping_rx_data = { 0 };
    sdk_table_api_params_t api_params;

    if (spec->fabric_encap.type != PDS_ENCAP_TYPE_MPLSoUDP) {
        return SDK_RET_OK;
    }
    remote_vnic_mapping_rx_key.vnic_metadata_src_slot_id =
        spec->fabric_encap.val.mpls_tag;
    remote_vnic_mapping_rx_key.ipv4_1_srcAddr = tep_ipaddr(spec).addr.v4_addr;
    remote_vnic_mapping_rx_data.vpc_id = vpc->hw_id();
    // TODO: why do we need subnet id here ??
    remote_vnic_mapping_rx_data.subnet_id = subnet->hw_id();
    sdk::lib::memrev(remote_vnic_mapping_rx_data.overlay_mac,
                     spec->overlay_mac, ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_rx_key,
                                   NULL, &remote_vnic_mapping_rx_data,
                                   REMOTE_VNIC_MAPPING_RX_REMOTE_VNIC_MAPPING_RX_INFO_ID,
                                   handle_.remote_.remote_vnic_rx_hdl_);
    ret = mapping_impl_db()->remote_vnic_mapping_rx_tbl()->insert(&api_params);
    // TODO: remote mapping key is SIPo, we need to handle the refcounts, if
    //       there are multiple inserts with same key (e.g. dual-stack)
    if (ret == sdk::SDK_RET_ENTRY_EXISTS || ret == SDK_RET_OK) {
        return SDK_RET_OK;
    }
    return ret;
}

// TODO: as we are not able to reserve() ahead of time currently, entries are
//       programmed here, ideally we just use pre-allocated indices
sdk_ret_t
mapping_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t             ret;
    pds_mapping_spec_t    *spec;
    vpc_entry             *vpc;
    subnet_entry          *subnet;

    spec = &obj_ctxt->api_params->mapping_spec;
    vpc = vpc_db()->find(&spec->skey.vpc);
    subnet = subnet_db()->find(&spec->subnet);
    PDS_TRACE_DEBUG("Programming mapping (vpc %s, ip %s), subnet %s, tep %s, "
                    "overlay mac %s, fabric encap type %u "
                    "fabric encap value %u, vnic %s",
                    spec->skey.vpc.str(), ipaddr2str(&spec->skey.ip_addr),
                    spec->subnet.str(), spec->tep.str(),
                    macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, spec->vnic.str());
    if (is_local_) {
        // allocate NAT table entries
        ret = add_nat_entries_(spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }

        ret = add_local_ip_mapping_entries_(vpc, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
        // store the local mapping encap as mytep encap
        mytep_encap_ = spec->fabric_encap;
    } else {
        ret = add_remote_vnic_mapping_rx_entries_(vpc, subnet, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }

        ret = add_remote_vnic_mapping_tx_entries_(vpc, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    return ret;
}

sdk_ret_t
mapping_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                        api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                          pds_epoch_t epoch, api_op_t api_op,
                          api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

void
mapping_impl::fill_mapping_spec_(
    remote_vnic_mapping_tx_appdata_t *remote_vnic_map_tx_data,
    nexthop_actiondata_t             *nh_data,
    tep_actiondata_t                 *tep_data,
    pds_mapping_spec_t               *spec) {

    spec->fabric_encap.val.value = remote_vnic_map_tx_data->dst_slot_id;
    if (tep_data->action_id  == TEP_MPLS_UDP_TEP_ID) {
        spec->fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        // spec->tep.ip_addr.af = IP_AF_IPV4;
        // spec->tep.ip_addr.addr.v4_addr = tep_data->tep_mpls_udp_action.dipo;
    } else if (tep_data->action_id  == TEP_IPV4_VXLAN_TEP_ID) {
        spec->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
        // spec->tep.ip_addr.af = IP_AF_IPV4;
        // spec->tep.ip_addr.addr.v4_addr = tep_data->tep_ipv4_vxlan_action.dipo;
    }
    // TODO: tag support
    spec->num_tags = 0;
}

sdk_ret_t
mapping_impl::read_local_mapping_(vpc_entry *vpc, pds_mapping_spec_t *spec) {
    sdk_ret_t                           ret;
    local_ip_mapping_swkey_t            local_ip_mapping_key = { 0 };
    local_ip_mapping_appdata_t          local_ip_mapping_data = { 0 };
    sdk_table_api_params_t              tparams;
    sdk_table_api_params_t              slot_api_params;
    local_vnic_by_slot_rx_swkey_t       vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_swkey_t       vnic_by_slot_key_mask = { 0 };
    local_vnic_by_slot_rx_actiondata_t  vnic_by_slot_data = { 0 };
    uint32_t                            vnic_hw_id;

    // first read the remote mapping, it can provide all the info except vnic id
    ret = read_remote_mapping_(vpc, spec);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // Read vnic id
    if (spec->fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        vnic_by_slot_key.mpls_dst_label = spec->fabric_encap.val.mpls_tag;
        vnic_by_slot_key_mask.mpls_dst_label = ~0;
    } else if (spec->fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
        vnic_by_slot_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
        vnic_by_slot_key_mask.vxlan_1_vni = ~0;
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&slot_api_params, &vnic_by_slot_key,
                                   NULL, &vnic_by_slot_data,
                                   LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID,
                                   sdk::table::handle_t::null());
    ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->get(&slot_api_params);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    vnic_hw_id = vnic_by_slot_data.local_vnic_by_slot_rx_info.local_vnic_tag;
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vnic_hw_id,
                                         &spec->skey.ip_addr, true);
    // prepare the api parameters to read the LOCAL_IP_MAPPING table
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_ip_mapping_key,
                                   NULL, &local_ip_mapping_data,
                                   LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_ip_mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // Make sure the VPC are matching , return error otherwise
    if (local_ip_mapping_data.vpc_id != vpc->hw_id()) {
        PDS_TRACE_ERR("LOCAL_IP_MAPPING table, vpc value mismatch read %u, "
                      "expected %u", local_ip_mapping_data.vpc_id,
                      vpc->hw_id());
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::read_remote_mapping_(vpc_entry *vpc, pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    remote_vnic_mapping_tx_swkey_t      remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_tx_appdata_t    remote_vnic_mapping_tx_data = { 0 };
    nexthop_actiondata_t                nh_data;
    tep_actiondata_t                    tep_data;
    remote_vnic_mapping_rx_swkey_t      remote_vnic_mapping_rx_key = { 0 };
    remote_vnic_mapping_rx_appdata_t    remote_vnic_mapping_rx_data = { 0 };
    sdk_table_api_params_t              tparams;
    uint32_t                            nh_index, tep_index;

    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vpc->hw_id(),
                                               &spec->skey.ip_addr, true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_tx_key,
                                   NULL, &remote_vnic_mapping_tx_data,
                                   REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(
        &tparams, remote_vnic_mapping_tx_data.nexthop_group_index, &nh_data, NULL);
    ret = nexthop_impl_db()->nh_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(
        &tparams, nh_data.action_u.nexthop_nexthop_info.tep_index, &tep_data, NULL);
    ret = tep_impl_db()->tep_tbl()->get(&tparams);
     if (ret != SDK_RET_OK) {
         return ret;
     }

    fill_mapping_spec_(&remote_vnic_mapping_tx_data, &nh_data, &tep_data,
                       spec);

    if (is_local_) {
        // REMOTE_VNIC_MAPPING_RX tables are not programmed for local
        return SDK_RET_OK;
    }
    // The below read requires data from the previous tables
    // It is valid only for mplsoudp encap
    if (spec->fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        memset(&tparams, 0, sizeof(tparams));
        remote_vnic_mapping_rx_key.vnic_metadata_src_slot_id =
            spec->fabric_encap.val.mpls_tag;
        remote_vnic_mapping_rx_key.ipv4_1_srcAddr = tep_ipaddr(spec).addr.v4_addr;
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_rx_key,
                                       NULL, &remote_vnic_mapping_rx_data,
                                       REMOTE_VNIC_MAPPING_RX_REMOTE_VNIC_MAPPING_RX_INFO_ID,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->remote_vnic_mapping_rx_tbl()->get(&tparams);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        // Fill overlay mac and subnet ID
        sdk::lib::memrev(spec->overlay_mac, remote_vnic_mapping_rx_data.overlay_mac,
                         ETH_ADDR_LEN);
        //spec->subnet.id = remote_vnic_mapping_rx_data.subnet_id;
        // Make sure the VPC are matching , return error otherwise
        if (remote_vnic_mapping_rx_data.vpc_id != vpc->hw_id()) {
            PDS_TRACE_ERR("REMOTE_VNIC_MAPPING_RX table, vpc value mismatch read %u, expected %u",
                      remote_vnic_mapping_rx_data.vpc_id, vpc->hw_id());
            return SDK_RET_ERR;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    nat_actiondata_t nat_data = { 0 };
    pds_mapping_key_t *mkey = (pds_mapping_key_t *)key;
    pds_mapping_info_t *minfo = (pds_mapping_info_t *)info;
    mapping_entry *mapping = (mapping_entry *)api_obj;
    sdk_table_api_params_t api_params;

    is_local_ = mapping->is_local();
    vpc = vpc_db()->find(&mkey->vpc);
    if (is_local_) {
        ret = read_local_mapping_(vpc, &minfo->spec);
    } else {
        ret = read_remote_mapping_(vpc, &minfo->spec);
    }
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // read public IP if it has been configured.
    if (handle_.local_.overlay_ip_to_public_ip_nat_hdl_) {
        PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(&api_params,
                                handle_.local_.overlay_ip_to_public_ip_nat_hdl_,
                                &nat_data, NULL);
        ret = mapping_impl_db()->nat_tbl()->get(&api_params);
        if (ret == SDK_RET_OK) {
            minfo->spec.public_ip_valid = true;
            // As public IPs are V4, can copy it without memrev
            minfo->spec.public_ip.af = IP_AF_IPV4;
            memcpy(&minfo->spec.public_ip.addr.v4_addr,
                   nat_data.nat_action.nat_ip, IP4_ADDR8_LEN);
        }
    }
    return ret;
}
uint16_t
mapping_impl::tep_idx(pds_mapping_key_t *key) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    sdk_table_api_params_t api_params;
    nexthop_actiondata_t nh_data;
    remote_vnic_mapping_tx_swkey_t remote_vnic_mapping_tx_key;
    remote_vnic_mapping_tx_appdata_t remote_vnic_mapping_tx_data;

    vpc = vpc_db()->find(&key->vpc);
    if (unlikely(vpc == NULL)) {
        return PDS_IMPL_TEP_INVALID_INDEX;
    }
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vpc->hw_id(), &key->ip_addr,
                                               true);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                   NULL, &remote_vnic_mapping_tx_data,
                                   REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->get(&api_params);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to find mapping (%s, %s) in "
                      "REMOTE_VNIC_MAPPING_TX table, err %u",
                      key->vpc.str(), ipaddr2str(&key->ip_addr), ret);
        return PDS_IMPL_TEP_INVALID_INDEX;
    }
    PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(
        &api_params, remote_vnic_mapping_tx_data.nexthop_group_index, &nh_data, NULL);
    ret = nexthop_impl_db()->nh_tbl()->get(&api_params);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read NH_TX table at index %u, err %u",
                      remote_vnic_mapping_tx_data.nexthop_group_index, ret);
        return PDS_IMPL_TEP_INVALID_INDEX;
    }
    return nh_data.action_u.nexthop_nexthop_info.tep_index;
}

ip_addr_t
mapping_impl::tep_ipaddr(pds_mapping_spec_t *spec) {
    if (is_local()) {
        return device_db()->find()->ip_addr();
    } else {
        return tep_db()->find(&spec->tep)->ip();
    }
}

/// \@}

}    // namespace impl
}    // namespace api
