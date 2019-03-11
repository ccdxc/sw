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
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/tep_impl.hpp"
#include "nic/apollo/api/impl/vnic_impl.hpp"
#include "nic/apollo/api/impl/mapping_impl.hpp"
#include "nic/apollo/api/impl/pds_impl_state.hpp"
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

mapping_impl *
mapping_impl::factory(pds_mapping_spec_t *pds_mapping) {
    mapping_impl    *impl;
    device_entry    *device;

    // TODO: move to slab later
    impl = (mapping_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_MAPPING_IMPL,
                                      sizeof(mapping_impl));
    new (impl) mapping_impl();
    device = device_db()->find();
    if (device->ip_addr() == pds_mapping->tep.ip_addr) {
        impl->is_local_ = true;
    }
    return impl;
}

void
mapping_impl::destroy(mapping_impl *impl) {
    impl->~mapping_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_MAPPING_IMPL, impl);
}

// TODO: IP address type (i.e., v4 or v6 bit) is not part of the key
#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(key, vnic_hw_id, ip)            \
{                                                                            \
    (key)->vnic_metadata_local_vnic_tag = vnic_hw_id;                        \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        sdk::lib::memrev((key)->control_metadata_mapping_lkp_addr,           \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        memcpy((key)->control_metadata_mapping_lkp_addr,                     \
               (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);                     \
    }                                                                        \
}

#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(data, vcn_hw_id, xlate_idx,   \
                                               iptype)                       \
{                                                                            \
    (data)->vcn_id = (vcn_hw_id);                                            \
    (data)->vcn_id_valid = true;                                             \
    (data)->xlate_index = (uint32_t)xlate_idx;                               \
    (data)->ip_type = (iptype);                                              \
}

// TODO: IP address type (i.e., v4 or v6 bit) is not part of the key
#define PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(key, vcn_hw_id, ip)       \
{                                                                            \
    (key)->txdma_to_p4e_header_vcn_id = vcn_hw_id;                           \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        sdk::lib::memrev((key)->p4e_apollo_i2e_dst,                          \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        memcpy((key)->p4e_apollo_i2e_dst,                                    \
               (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);                     \
    }                                                                        \
}

#define PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_APPDATA(data, nh_id, encap)     \
{                                                                            \
    (data)->nexthop_index = (nh_id);                                         \
    (data)->dst_slot_id_valid = 1;                                           \
    if ((encap)->type == PDS_ENCAP_TYPE_MPLSoUDP) {                          \
        (data)->dst_slot_id = (encap)->val.mpls_tag;                         \
    } else if ((encap)->type == PDS_ENCAP_TYPE_VXLAN) {                      \
        (data)->dst_slot_id = (encap)->val.vnid;                             \
    }                                                                        \
}

#define PDS_IMPL_FILL_TABLE_API_PARAMS(api_params, key_, data, action, hdl)  \
{                                                                            \
    memset((api_params), 0, sizeof(*(api_params)));                          \
    (api_params)->key = (key_);                                              \
    (api_params)->appdata = (data);                                          \
    (api_params)->action_id = (action);                                      \
    (api_params)->handle = (hdl);                                            \
}

// TODO:
// 1. index tables don't support reserve()/release() yet, so NAT entries
// are not reserved
// 2. not installing REMOTE_VNIC_MAPPING_TX for local IP mappings because
//    TEP doesn't exist for local vnic IP yet
sdk_ret_t
mapping_impl::reserve_local_ip_mapping_resources_(api_base *api_obj,
                                                  vcn_entry *vcn,
                                                  pds_mapping_spec_t *spec) {
    sdk_ret_t                         ret = SDK_RET_OK;
    vnic_impl                         *vnic_impl_obj;
    local_ip_mapping_swkey_t          local_ip_mapping_key = { 0 };
    remote_vnic_mapping_tx_swkey_t    remote_vnic_mapping_tx_key = { 0 };
    sdk_table_api_params_t            tparams;

    vnic_impl_obj =
        (vnic_impl *)vnic_db()->vnic_find(&spec->vnic)->impl();

    // reserve an entry in LOCAL_IP_MAPPING table for overlay IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vnic_impl_obj->hw_id(),
                                         &spec->key.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_ip_mapping_key,
                                   NULL, 0, SDK_TABLE_HANDLE_INVALID);
    ret = mapping_impl_db()->local_ip_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_IP_MAPPING table "
                      "for mapping %s, err %u", api_obj->key2str().c_str(),
                      ret);
        return ret;
    }
    overlay_ip_hdl_ = tparams.handle;

#if 0
    // reserve an entry in REMOTE_VNIC_MAPPING_TX for overlay IP
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vcn->hw_id(),
                                               &spec->key.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_tx_key,
                                   NULL, 0, SDK_TABLE_HANDLE_INVALID);
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in REMOTE_VNIC_MAPPING_TX "
                      "table for mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    overlay_ip_remote_vnic_tx_hdl_ = tparams.handle;
#endif

    PDS_TRACE_DEBUG("Reserved overlay_ip_hdl %lu, "
                    "overlay_ip_remote_vnic_tx_hdl %lu",
                    overlay_ip_hdl_, overlay_ip_remote_vnic_tx_hdl_);

    // check if this mapping has public IP
    if (!spec->public_ip_valid) {
        return SDK_RET_OK;
    }

    // reserve an entry in LOCAL_IP_MAPPING table for public IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vnic_impl_obj->hw_id(),
                                         &spec->public_ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_ip_mapping_key,
                                   NULL, 0, SDK_TABLE_HANDLE_INVALID);
    ret = mapping_impl_db()->local_ip_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_IP_MAPPING table "
                      "for public IP of mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    public_ip_hdl_ = tparams.handle;

#if 0
    // reserve an entry in REMOTE_VNIC_MAPPING_TX for public IP
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vcn->hw_id(), &spec->public_ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_tx_key,
                                   NULL, 0, SDK_TABLE_HANDLE_INVALID);
    ret =
        mapping_impl_db()->remote_vnic_mapping_tx_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in REMOTE_VNIC_MAPPING_TX "
                      "table for public IP of mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    public_ip_remote_vnic_tx_hdl_ = tparams.handle;
#endif

    PDS_TRACE_DEBUG("Reserved public_ip_hdl %lu, "
                    "public_ip_remote_vnic_tx_hdl %u",
                    public_ip_hdl_, public_ip_remote_vnic_tx_hdl_);

    return SDK_RET_OK;

error:

    // TODO: release all allocated resources
    return ret;
}

sdk_ret_t
mapping_impl::reserve_remote_ip_mapping_resources_(api_base *api_obj,
                                                   vcn_entry *vcn,
                                                   pds_mapping_spec_t *spec) {
    sdk_ret_t                         ret = SDK_RET_OK;
    remote_vnic_mapping_tx_swkey_t    remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_rx_swkey_t    remote_vnic_mapping_rx_key = { 0 };
    sdk_table_api_params_t            tparams;

    // reserve an entry in REMOTE_VNIC_MAPPING_TX table
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vcn->hw_id(),
                                               &spec->key.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_tx_key,
                                   NULL, 0, SDK_TABLE_HANDLE_INVALID);
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in REMOTE_VNIC_MAPPING_TX "
                      "table for mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    remote_vnic_tx_hdl_ = tparams.handle;

    // REMOTE_VNIC_MAPPING_RX table is not used for non MPLSoUDP encaps
    if (spec->fabric_encap.type != PDS_ENCAP_TYPE_MPLSoUDP) {
        return SDK_RET_OK;
    }

    // reserve an entry in REMOTE_VNIC_MAPPING_RX table
    remote_vnic_mapping_rx_key.vnic_metadata_src_slot_id =
        spec->fabric_encap.val.mpls_tag;
    remote_vnic_mapping_rx_key.ipv4_1_srcAddr = spec->tep.ip_addr;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &remote_vnic_mapping_rx_key,
                                   NULL, 0, SDK_TABLE_HANDLE_INVALID);
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
    remote_vnic_rx_hdl_ = tparams.handle;

    return SDK_RET_OK;

error:

    return ret;
}

sdk_ret_t
mapping_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    pds_mapping_spec_t    *spec;
    vcn_entry             *vcn;

    spec = &obj_ctxt->api_params->mapping_spec;
    vcn = vcn_db()->find(&spec->key.vcn);

    PDS_TRACE_DEBUG("Reserving resources for mapping (vcn %u, ip %s), "
                    "subnet %u, tep %s, vnic %u",
                    spec->key.vcn.id, ipaddr2str(&spec->key.ip_addr),
                    spec->subnet.id, ipv4addr2str(spec->tep.ip_addr),
                    spec->vnic.id);

    if (is_local_) {
        return reserve_local_ip_mapping_resources_(orig_obj, vcn, spec);
    }
    return reserve_remote_ip_mapping_resources_(orig_obj, vcn, spec);
}

sdk_ret_t
mapping_impl::release_resources(api_base *api_obj) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::add_remote_vnic_mapping_tx_entries_(vcn_entry *vcn,
                                                  pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    remote_vnic_mapping_tx_swkey_t remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_tx_appdata_t remote_vnic_mapping_tx_data = { 0 };
    tep_impl *tep_impl_obj;
    sdk_table_api_params_t api_params;

    tep_impl_obj =
        (tep_impl *)tep_db()->tep_find(&spec->tep)->impl();
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vcn->hw_id(),
                                               &spec->key.ip_addr);
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_APPDATA(&remote_vnic_mapping_tx_data,
                                                 tep_impl_obj->nh_id(),
                                                 &spec->fabric_encap);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                   &remote_vnic_mapping_tx_data,
                                   REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID,
                                   is_local_ ? overlay_ip_remote_vnic_tx_hdl_ :
                                               remote_vnic_tx_hdl_);
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add program entry in REMOTE_VNIC_MAPPING_TX "
                      "table for (vcn %u, IP %s), err %u\n", vcn->hw_id(),
                      ipaddr2str(&spec->key.ip_addr), ret);
        goto error;
    }
    PDS_TRACE_DEBUG("Programmed REMOTE_VNIC_MAPPING_TX table entry "
                    "TEP %s, vcn hw id %u, encap type %u, encap val %u "
                    "nh id %u", ipv4addr2str(spec->tep.ip_addr),
                    vcn->hw_id(), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, tep_impl_obj->nh_id());

    if (!is_local_ || !spec->public_ip_valid) {
        return SDK_RET_OK;
    }

    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_SWKEY(&remote_vnic_mapping_tx_key,
                                               vcn->hw_id(), &spec->public_ip);
    PDS_IMPL_FILL_REMOTE_VNIC_MAPPING_TX_APPDATA(&remote_vnic_mapping_tx_data,
                                                 tep_impl_obj->nh_id(),
                                                 &spec->fabric_encap);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_tx_key,
                                   &remote_vnic_mapping_tx_data,
                                   REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID,
                                   public_ip_remote_vnic_tx_hdl_);
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add program entry in REMOTE_VNIC_MAPPING_TX "
                      "table for (vcn %u, IP %s), err %u\n", vcn->hw_id(),
                      ipaddr2str(&spec->public_ip), ret);
        goto error;
    }
    return SDK_RET_OK;

error:

    return ret;
}

#define nat_action    action_u.nat_nat
sdk_ret_t
mapping_impl::add_nat_entries_(pds_mapping_spec_t *spec) {
    sdk_ret_t           ret;
    nat_actiondata_t    nat_data = { 0 };

    // allocate NAT table entries
    if (spec->public_ip_valid) {
        // add private to public IP xlation NAT entry
        nat_data.action_id = NAT_NAT_ID;
        memcpy(nat_data.nat_action.nat_ip,
               spec->public_ip.addr.v6_addr.addr8,
               IP6_ADDR8_LEN);
        ret =
            mapping_impl_db()->nat_tbl()->insert(&nat_data,
                                                 (uint32_t *)&overlay_ip_to_public_ip_nat_hdl_);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        // add public to private IP xlation NAT entry
        memcpy(nat_data.nat_action.nat_ip,
               spec->key.ip_addr.addr.v6_addr.addr8,
               IP6_ADDR8_LEN);
        ret =
            mapping_impl_db()->nat_tbl()->insert(&nat_data,
                                                 (uint32_t *)&public_ip_to_overlay_ip_nat_hdl_);
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
mapping_impl::add_local_ip_mapping_entries_(vcn_entry *vcn,
                                            pds_mapping_spec_t *spec) {
    sdk_ret_t                          ret;
    vnic_impl                          *vnic_impl_obj;
    local_ip_mapping_swkey_t           local_ip_mapping_key = { 0 };
    local_ip_mapping_appdata_t          local_ip_mapping_data = { 0 };
    remote_vnic_mapping_tx_swkey_t      remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_tx_appdata_t    remote_vnic_mapping_tx_data = { 0 };
    sdk_table_api_params_t              api_params = { 0 };

    // add entry to LOCAL_IP_MAPPING table for overlay IP
    vnic_impl_obj =
        (vnic_impl *)vnic_db()->vnic_find(&spec->vnic)->impl();
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vnic_impl_obj->hw_id(),
                                         &spec->key.ip_addr);
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_ip_mapping_data, vcn->hw_id(),
                                           overlay_ip_to_public_ip_nat_hdl_,
                                           IP_TYPE_OVERLAY);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key,
                                   &local_ip_mapping_data,
                                   LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                   overlay_ip_hdl_);
    ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    // add entry to LOCAL_IP_MAPPING table for public IP
    if (spec->public_ip_valid) {
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                             vnic_impl_obj->hw_id(),
                                             &spec->public_ip);
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_ip_mapping_data,
                                               vcn->hw_id(),
                                               public_ip_to_overlay_ip_nat_hdl_,
                                               IP_TYPE_PUBLIC);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params,
                                       &local_ip_mapping_key,
                                       &local_ip_mapping_data,
                                       LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                       public_ip_hdl_);
        ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }

    // TODO: add entry to REMOTE_VNIC_MAPPING_TX table for overlay IP
    // add_remote_vnic_mapping_tx_entries_(vcn, spec);

    return SDK_RET_OK;

error:

    // TODO: handle cleanup in case of failure
    return ret;
}

sdk_ret_t
mapping_impl::add_remote_vnic_mapping_rx_entries_(vcn_entry *vcn,
                                                  subnet_entry *subnet,
                                                  pds_mapping_spec_t *spec) {
    sdk_ret_t                         ret;
    remote_vnic_mapping_rx_swkey_t remote_vnic_mapping_rx_key = { 0 };
    remote_vnic_mapping_rx_appdata_t remote_vnic_mapping_rx_data = { 0 };
    sdk_table_api_params_t api_params = { 0 };

    if (spec->fabric_encap.type != PDS_ENCAP_TYPE_MPLSoUDP) {
        return SDK_RET_OK;
    }
    remote_vnic_mapping_rx_key.vnic_metadata_src_slot_id =
        spec->fabric_encap.val.mpls_tag;
    remote_vnic_mapping_rx_key.ipv4_1_srcAddr = spec->tep.ip_addr;
    remote_vnic_mapping_rx_data.vcn_id = vcn->hw_id();
    // TODO: why do we need subnet id here ??
    remote_vnic_mapping_rx_data.subnet_id = subnet->hw_id();
    sdk::lib::memrev(remote_vnic_mapping_rx_data.overlay_mac,
                     spec->overlay_mac, ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &remote_vnic_mapping_rx_key,
                                   &remote_vnic_mapping_rx_data,
                                   REMOTE_VNIC_MAPPING_RX_REMOTE_VNIC_MAPPING_RX_INFO_ID,
                                   remote_vnic_rx_hdl_);
    ret = mapping_impl_db()->remote_vnic_mapping_rx_tbl()->insert(&api_params);
    // TODO: Remote mapping key is SIPo, we need to handle the refcounts, if
    // there are multiple inserts with same key (e.g. dual-stack)
    if (ret == sdk::SDK_RET_ENTRY_EXISTS || ret == SDK_RET_OK) {
        return SDK_RET_OK;
    }
    return ret;
}

// TODO: as we are not able to reserve() ahead of time currently, entries are
//       programmed here, ideally we just use pre-allocated indices
sdk_ret_t
mapping_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t             ret;
    pds_mapping_spec_t    *spec;
    vcn_entry             *vcn;
    subnet_entry          *subnet;

    spec = &obj_ctxt->api_params->mapping_spec;
    vcn = vcn_db()->find(&spec->key.vcn);
    subnet = subnet_db()->subnet_find(&spec->subnet);
    PDS_TRACE_DEBUG("Programming mapping (vcn %u, ip %s), subnet %u, tep %s, "
                    "overlay mac %s, fabric encap type %u "
                    "fabric encap value %u, vnic %u",
                    spec->key.vcn.id, ipaddr2str(&spec->key.ip_addr),
                    spec->subnet.id, ipv4addr2str(spec->tep.ip_addr),
                    macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, spec->vnic.id);
    if (is_local_) {
        // allocate NAT table entries
        ret = add_nat_entries_(spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }

        ret = add_local_ip_mapping_entries_(vcn, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    } else {
        // TODO: right now locals are not added to remote_vnic_mapping_rx/tx
        //       tables but once host-to-host path is finalized we can relax
        //       this, if needed !!
        ret = add_remote_vnic_mapping_rx_entries_(vcn, subnet, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }

        ret = add_remote_vnic_mapping_tx_entries_(vcn, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

#if 0
    // TODO: cleanup everything thats allocated here !!
    if (nat_idx1_ != 0xFFFFFFFF) {
    }
    ...
    ...
#endif
    return ret;
}

sdk_ret_t
mapping_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                        obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                          api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

void
mapping_impl::fill_mapping_spec_(
                  remote_vnic_mapping_tx_appdata_t *remote_vnic_map_tx,
                  pds_mapping_spec_t *spec) {
    // TODO: we have to set encap type as well !!
    spec->fabric_encap.val.value = remote_vnic_map_tx->dst_slot_id;
    // TODO fill the remaining
}

sdk_ret_t
mapping_impl::read_hw(pds_mapping_key_t *key,
                      pds_mapping_info_t    *info) {
    p4pd_error_t p4pd_ret;
    sdk_ret_t ret;
    vcn_entry *vcn;
    remote_vnic_mapping_tx_swkey_t remote_vnic_mapping_tx_key;
    remote_vnic_mapping_tx_appdata_t remote_vnic_mapping_tx_data;
    sdk_table_api_params_t api_params = { 0 };

    vcn = vcn_db()->find(&key->vcn);
    memcpy(remote_vnic_mapping_tx_key.p4e_apollo_i2e_dst,
           key->ip_addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
    remote_vnic_mapping_tx_key.txdma_to_p4e_header_vcn_id = vcn->hw_id();

    api_params.key = &remote_vnic_mapping_tx_key;
    api_params.appdata = &remote_vnic_mapping_tx_data;
    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->get(&api_params);
    if (ret == SDK_RET_OK) {
        fill_mapping_spec_(&remote_vnic_mapping_tx_data, &info->spec);
    }
    return ret;
}

/// \@}    // end of PDS_MAPPING_IMPL

}    // namespace impl
}    // namespace api
