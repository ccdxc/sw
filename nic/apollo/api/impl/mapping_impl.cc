/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping_impl.cc
 *
 * @brief   datapath implementation of mapping
 */

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

/**
 * @defgroup PDS_MAPPING_IMPL - mapping entry datapath implementation
 * @ingroup PDS_MAPPING
 * @{
 */

/**
 * @brief    factory method to allocate & initialize mapping impl instance
 * @param[in] pds_mapping    mapping information
 * @return    new instance of mapping or NULL, in case of error
 */
mapping_impl *
mapping_impl::factory(pds_mapping_spec_t *pds_mapping) {
    mapping_impl    *impl;
    device_entry    *device;

    // TODO: move to slab later
    impl = (mapping_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_MAPPING_IMPL,
                                      sizeof(mapping_impl));
    new (impl) mapping_impl();
    device = device_db()->device_find();
    if (device->ip_addr() == pds_mapping->tep.ip_addr) {
        impl->is_local_ = true;
    }
    return impl;
}

/**
 * @brief    release all the s/w state associated with the given mapping,
 *           if any, and free the memory
 * @param[in] mapping     mapping to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
mapping_impl::destroy(mapping_impl *impl) {
    impl->~mapping_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_MAPPING_IMPL, impl);
}

/**
 * @brief    allocate/reserve h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::reserve_resources(api_base *api_obj) {
#if 0
    pds_mapping_spec_t    *spec;

    spec = &obj_ctxt->api_params->mapping_spec;
    if (is_local_) {
        mapping_impl_db()->local_ip_mapping_tbl()->reserve(key,
                                                           &local_ip_mapping_idx_);
        if (spec->public_ip_valid) {
            mapping_impl_db()->nat_tbl()->reserve(&nat_idx_);
        }
    }
    mapping_impl_db()->remote_vnic_mapping_tx_tbl()->reserve(key,
                                                             &remote_vnic_mapping_tx_idx_);
    mapping_impl_db()->remote_vnic_mapping_rx_tbl()->reserve(key,
                                                             &remote_vnic_mapping_rx_idx_);
#endif
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::release_resources(api_base *api_obj) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief     add necessary entries to NAT table
 * @param[in] spec    IP mapping details
 * @return    SDK_RET_OK on success, failure status code on error
 */
#define nat_action    action_u.nat_nat
sdk_ret_t
mapping_impl::add_nat_entries_(pds_mapping_spec_t *spec) {
    sdk_ret_t           ret;
    nat_actiondata_t    nat_data = { 0 };

    /**< allocate NAT table entries */
    if (spec->public_ip_valid) {
        /**< add private to public IP xlation NAT entry */
        nat_data.action_id = NAT_NAT_ID;
        memcpy(nat_data.nat_action.nat_ip,
               spec->public_ip.addr.v6_addr.addr8,
               IP6_ADDR8_LEN);
        ret = mapping_impl_db()->nat_tbl()->insert(&nat_data, &nat_idx1_);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        /**< add public to private IP xlation NAT entry */
        memcpy(nat_data.nat_action.nat_ip,
               spec->key.ip_addr.addr.v6_addr.addr8,
               IP6_ADDR8_LEN);
        ret = mapping_impl_db()->nat_tbl()->insert(&nat_data, &nat_idx2_);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:
    // TODO: handle cleanup in case of failure
    return ret;
}

/**
 * @brief     add necessary entries to LOCAL_IP_MAPPING table
 * @param[in] vcn     VCN of this IP
 * @param[in] spec    IP mapping details
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::add_local_ip_mapping_entries_(vcn_entry *vcn,
                                            pds_mapping_spec_t *spec) {
    sdk_ret_t                   ret;
    vnic_impl                   *vnic_impl_obj;
    local_ip_mapping_swkey_t    local_ip_mapping_key = { 0 };
    local_ip_mapping_appdata_t  local_ip_mapping_data = { 0 };
    sdk_table_api_params_t       api_params = { 0 };

    vnic_impl_obj =
        (vnic_impl *)vnic_db()->vnic_find(&spec->vnic)->impl();
    local_ip_mapping_key.vnic_metadata_local_vnic_tag =
        vnic_impl_obj->hw_id();
    // TODO: ipv4 or ipv6 is not part of the key ? p4 needs to change
    memcpy(local_ip_mapping_key.control_metadata_mapping_lkp_addr,
           spec->key.ip_addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
    local_ip_mapping_data.vcn_id = vcn->hw_id();
    local_ip_mapping_data.vcn_id_valid = true;
    local_ip_mapping_data.xlate_index = nat_idx1_;
    local_ip_mapping_data.ip_type = IP_TYPE_OVERLAY;

    // prepare the api parameters
    api_params.key = &local_ip_mapping_key;
    api_params.appdata = &local_ip_mapping_data;
    api_params.action_id = LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID;
    ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
    if ((ret != SDK_RET_OK) && (ret != sdk::SDK_RET_COLLISION)) {
        goto error;
    }

    if (spec->public_ip_valid) {
        local_ip_mapping_key.vnic_metadata_local_vnic_tag =
            vnic_impl_obj->hw_id();
        // TODO: ipv4 or ipv6 is not part of the key ? p4 needs to change
        memcpy(local_ip_mapping_key.control_metadata_mapping_lkp_addr,
               spec->public_ip.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        local_ip_mapping_data.vcn_id = vcn->hw_id();
        local_ip_mapping_data.vcn_id_valid = true;
        local_ip_mapping_data.xlate_index = nat_idx2_;
        local_ip_mapping_data.ip_type = IP_TYPE_PUBLIC;
   
        // API params has input/output fields, cannot re-use it as it is.
        memset(&api_params, 0, sizeof(api_params));
        api_params.key = &local_ip_mapping_key;
        api_params.appdata = &local_ip_mapping_data;
        api_params.action_id = LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID;
        ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:
    // TODO: handle cleanup in case of failure
    return ret;
}

/**
 * @brief     program REMOTE_VNIC_MAPPING_RX table entry
 * @param[in] vcn     VCN of this IP
 * @param[in] subnet  subnet of this IP
 * @param[in] spec    IP mapping details
 * @return    SDK_RET_OK on success, failure status code on error
 */
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
    api_params.key = &remote_vnic_mapping_rx_key;
    api_params.appdata = &remote_vnic_mapping_rx_data;
    api_params.action_id =
        REMOTE_VNIC_MAPPING_RX_REMOTE_VNIC_MAPPING_RX_INFO_ID;
    ret = mapping_impl_db()->remote_vnic_mapping_rx_tbl()->insert(&api_params);
    return ret;
}

/**
 * @brief     program REMOTE_VNIC_MAPPING_TX table entry
 * @param[in] vcn     VCN of this IP
 * @param[in] spec    IP mapping details
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::add_remote_vnic_mapping_tx_entries_(vcn_entry *vcn,
                                                  pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    remote_vnic_mapping_tx_swkey_t remote_vnic_mapping_tx_key = { 0 };
    remote_vnic_mapping_tx_appdata_t remote_vnic_mapping_tx_data = { 0 };
    tep_impl *tep_impl_obj;
    sdk_table_api_params_t api_params = { 0 };

    tep_impl_obj =
        (tep_impl *)tep_db()->tep_find(&spec->tep)->impl();
    remote_vnic_mapping_tx_key.txdma_to_p4e_header_vcn_id = vcn->hw_id();
    // TODO: ipv4 or ipv6 is not part of the key ? p4 needs to change
    memcpy(remote_vnic_mapping_tx_key.p4e_apollo_i2e_dst,
           spec->key.ip_addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
    remote_vnic_mapping_tx_data.nexthop_index = tep_impl_obj->nh_id();
    remote_vnic_mapping_tx_data.dst_slot_id_valid = 1;
    if (spec->fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        remote_vnic_mapping_tx_data.dst_slot_id =
            spec->fabric_encap.val.mpls_tag;
    } else if (spec->fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
        remote_vnic_mapping_tx_data.dst_slot_id = spec->fabric_encap.val.vnid;
    }

    api_params.key = &remote_vnic_mapping_tx_key;
    api_params.appdata = &remote_vnic_mapping_tx_data;
    api_params.action_id =
        REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID;

    ret = mapping_impl_db()->remote_vnic_mapping_tx_tbl()->insert(&api_params);
    if (ret == SDK_RET_OK) {
        PDS_TRACE_DEBUG("Programmed REMOTE_VNIC_MAPPING_TX table entry "
                        "TEP %s, vcn hw id %u, encap type %u, encap val %u "
                        "nh id %u", ipv4addr2str(spec->tep.ip_addr),
                        vcn->hw_id(), spec->fabric_encap.type,
                        spec->fabric_encap.val.value, tep_impl_obj->nh_id());
    }
    return ret;
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

    vcn = vcn_db()->vcn_find(&key->vcn);
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

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
// TODO: as we are not able to reserve() ahead of time currently, entries are
//       programmed here, ideally we just use pre-allocated indices
sdk_ret_t
mapping_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t             ret;
    pds_mapping_spec_t    *spec;
    vcn_entry             *vcn;
    subnet_entry          *subnet;

    spec = &obj_ctxt->api_params->mapping_spec;
    vcn = vcn_db()->vcn_find(&spec->key.vcn);
    subnet = subnet_db()->subnet_find(&spec->subnet);
    PDS_TRACE_DEBUG("Programming mapping (vcn %u, ip %s), subnet %u, tep %s, "
                    "overlay mac %s, fabric encap type %u "
                    "fabric encap value %u, vnic %u",
                    spec->key.vcn.id, ipaddr2str(&spec->key.ip_addr),
                    spec->subnet.id, ipv4addr2str(spec->tep.ip_addr),
                    macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, spec->vnic.id);
    if (is_local_) {
        /**< allocate NAT table entries */
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

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                        obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    activate the epoch in the dataplane by programming stage 0
 *           tables, if any
 * @param[in] epoch       epoch being activated
 * @param[in] api_op      api operation
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
mapping_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                          api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/** @} */    // end of PDS_MAPPING_IMPL

}    // namespace impl
}    // namespace api
