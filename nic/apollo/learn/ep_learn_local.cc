//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn object handling for local endpoints
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/include/sdk/l2.hpp"
#include "nic/sdk/include/sdk/l4.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/learn/ep_aging.hpp"
#include "nic/apollo/learn/ep_ip_state.hpp"
#include "nic/apollo/learn/ep_learn_local.hpp"
#include "nic/apollo/learn/ep_mac_state.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn_state.hpp"
#include "nic/apollo/learn/learn_thread.hpp"
#include "nic/apollo/learn/utils.hpp"

namespace learn {

using namespace sdk::types;

#define ep_mac_db       learn_db()->ep_mac_db
#define ep_ip_db        learn_db()->ep_ip_db

// find out if given MAC or IP is learnt on remote TEP
static bool
remote_mapping_find (local_learn_ctxt_t *ctxt, pds_mapping_type_t mapping_type)
{
    pds_mapping_key_t mkey;
    sdk_ret_t ret;

    mkey.type = mapping_type;
    if (mkey.type == PDS_MAPPING_TYPE_L2) {
        mkey.subnet = ctxt->mac_key.subnet;
        MAC_ADDR_COPY(mkey.mac_addr, ctxt->mac_key.mac_addr);
    } else {
        mkey.vpc = ctxt->ip_key.vpc;
        mkey.ip_addr = ctxt->ip_key.ip_addr;
    }

    ret = impl::remote_mapping_find(&mkey);
    SDK_ASSERT((ret == SDK_RET_OK || ret == SDK_RET_ENTRY_NOT_FOUND));
    return (ret == SDK_RET_OK);
}

static bool
detect_l2l_move (local_learn_ctxt_t *ctxt, pds_mapping_type_t mapping_type)
{
    pds_obj_key_t vnic_key;
    vnic_entry *vnic;
    pds_ifindex_t ifindex;
    bool l2l_move = false;

    if (mapping_type == PDS_MAPPING_TYPE_L2) {
        // check if existing VNIC for the MAC address is on different LIF than
        // the one on which this pkt arrived
        vnic_key = api::uuid_from_objid(ctxt->mac_entry->vnic_obj_id());
        vnic = vnic_db()->find(&vnic_key);
        ifindex = (pds_ifindex_t) LIF_IFINDEX(ctxt->impl_info.lif);

        l2l_move = (api::objid_from_uuid(vnic->host_if()) != ifindex);
        // TODO: check if VLAN has changed
    } else {
        // check if existing MAC address mapped to this IP is different from the
        // one seen in the packet
        uint32_t vnic_obj_id = ctxt->mac_entry->vnic_obj_id();
        l2l_move = (!ctxt->ip_entry->vnic_compare(vnic_obj_id));
    }
    return l2l_move;
}

static ep_learn_type_t
detect_learn_type (local_learn_ctxt_t *ctxt, pds_mapping_type_t mapping_type)
{
    ep_learn_type_t learn_type;
    bool is_local;

    // check if this MAC or IP is known
    if (mapping_type == PDS_MAPPING_TYPE_L2) {
        is_local = (ctxt->mac_entry != nullptr);
    } else {
        is_local = (ctxt->ip_entry != nullptr);
    }

    if (is_local) {
        // local mapping already exists, check for L2L move
        if (detect_l2l_move(ctxt, mapping_type)) {
            learn_type = LEARN_TYPE_MOVE_L2L;

        } else {
            learn_type = LEARN_TYPE_NONE;
        }
    } else {
        // check if remote mapping exists
        if (remote_mapping_find(ctxt, mapping_type)) {
            learn_type = LEARN_TYPE_MOVE_R2L;
        } else {
            // neither local nor remote mappoing exists
            learn_type = LEARN_TYPE_NEW_LOCAL;
        }
    }
    return learn_type;
}

static bool
extract_ip_learn_info (char *pkt_data, local_learn_ctxt_t *ctxt)
{
    ip_addr_t *ip = &ctxt->ip_key.ip_addr;

    switch (ctxt->impl_info.pkt_type) {
    case PKT_TYPE_DHCP:
    {
        dhcp_header_t *dhcp = (dhcp_header_t *) (pkt_data +
                                                 ctxt->impl_info.l3_offset +
                                                 IPV4_MIN_HDR_LEN +
                                                 UDP_HDR_LEN);
        dhcp_option_t *op = dhcp->options;
        bool dhcp_request = false;
        bool ip_found = false;
        while (op->option != DHCP_PACKET_OPTION_END) {
            if (DHCP_PACKET_OPTION_MSG_TYPE == op->option) {
                if (DHCP_PACKET_REQUEST == op->data[0]) {
                    dhcp_request = true;
                } else {
                    // this is not msg of our interest
                    break;
                }
            } else if (DHCP_PACKET_OPTION_REQ_IP_ADDR == op->option) {
                ip->af = IP_AF_IPV4;
                ip->addr.v4_addr = ntohl((*(op->data_as_u32)));
                ip_found = true;
            }
            if (dhcp_request && ip_found) {
                break;
            }
            op = (dhcp_option_t *) (op->data + op->length);
        }
        if (!dhcp_request || !ip_found || (0 == ip->addr.v4_addr)) {
            return false;
        }
        break;
    }
    case PKT_TYPE_ARP:
    {
        arp_hdr_t *arp_hdr = (arp_hdr_t *) (pkt_data +
                                            ctxt->impl_info.l3_offset);
        arp_data_ipv4_t *arp_data = (arp_data_ipv4_t *) (arp_hdr + 1);

        if (ntohs(arp_hdr->ptype) != ETH_TYPE_IPV4) {
            return false;
        }

        // check if packet smac is same as smac in arp header
        if (memcmp(ctxt->mac_key.mac_addr, &arp_data->smac, ETH_ADDR_LEN) != 0) {
            return false;
        }

        ip->af = IP_AF_IPV4;
        ip->addr.v4_addr = ntohl(arp_data->sip);
        break;
    }
    case PKT_TYPE_IPV4:
        IPV4_HDR_SIP_GET(pkt_data + ctxt->impl_info.l3_offset, ip->addr.v4_addr);
        // dhcp packets other than ack have src IP set to 0.0.0.0
        if (ip->addr.v4_addr == 0) {
            return false;
        }
        ip->af = IP_AF_IPV4;
        break;
    case PKT_TYPE_NDP:
    case PKT_TYPE_IPV6:
    default:
        return false;
    }

    ctxt->ip_key.vpc = subnet_db()->find(&ctxt->mac_key.subnet)->vpc();
    return true;
}

static sdk_ret_t
extract_learn_info (char *pkt_data, local_learn_ctxt_t *ctxt)
{
    char *src_mac;

    // MAC addr is always available, populate ep->mac_key
    src_mac = pkt_data + ctxt->impl_info.l2_offset + ETH_ADDR_LEN;
    MAC_ADDR_COPY(&ctxt->mac_key.mac_addr, src_mac);
    ctxt->mac_key.subnet = ctxt->impl_info.subnet;
    ctxt->mac_entry = ep_mac_db()->find(&ctxt->mac_key);
    ctxt->mac_learn_type = detect_learn_type(ctxt, PDS_MAPPING_TYPE_L2);

    // extract IP address if present and populate ep->ip_key
    if (!extract_ip_learn_info(pkt_data, ctxt)) {
        ctxt->ip_learn_type = LEARN_TYPE_INVALID;
        return SDK_RET_OK;
    }

    ctxt->ip_entry = ep_ip_db()->find(&ctxt->ip_key);
    ctxt->ip_learn_type = detect_learn_type(ctxt, PDS_MAPPING_TYPE_L3);
    return SDK_RET_OK;
}

static sdk_ret_t
process_new_mac (local_learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;
    pds_vnic_spec_t spec = { 0 };
    uint32_t vnic_obj_id;

    // allocate MAC entry, VNIC object id and create new vnic
    ret = learn_db()->vnic_obj_id_alloc(&vnic_obj_id);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to allocate VNIC object id, error code %u", ret);
        ctxt->pkt_drop_reason = PKT_DROP_REASON_RES_ALLOC_FAIL;
        return ret;
    }
    ctxt->mac_entry = ep_mac_entry::factory(&ctxt->mac_key, vnic_obj_id);
    if (unlikely(ctxt->mac_entry == nullptr)) {
        learn_db()->vnic_obj_id_free(vnic_obj_id);
        PDS_TRACE_ERR("Failed to allocate MAC entry");
        ctxt->pkt_drop_reason = PKT_DROP_REASON_RES_ALLOC_FAIL;
        return SDK_RET_ERR;
    }

    // TODO: encode learn module specific identifier in uuid
    spec.key = api::uuid_from_objid(vnic_obj_id);
    spec.subnet = ctxt->mac_key.subnet;
    spec.fabric_encap.type = PDS_ENCAP_TYPE_NONE;
    spec.vnic_encap = ctxt->impl_info.encap;
    MAC_ADDR_COPY(spec.mac_addr, ctxt->mac_key.mac_addr);
    spec.host_if = api::uuid_from_objid(LIF_IFINDEX(ctxt->impl_info.lif));

    PDS_TRACE_INFO("Creating VNIC %s for EP %s", spec.key.str(), ctxt->str());
    return pds_vnic_create(&spec, ctxt->bctxt);
}

static sdk_ret_t
process_l2l_mac_move (local_learn_ctxt_t *ctxt)
{
    return SDK_RET_INVALID_OP;
}

static sdk_ret_t
process_r2l_mac_move (local_learn_ctxt_t *ctxt)
{
    return SDK_RET_INVALID_OP;
}

static sdk_ret_t
process_mac_learn_info (local_learn_ctxt_t *ctxt)
{
    switch (ctxt->mac_learn_type) {
    case LEARN_TYPE_NEW_LOCAL:
        return process_new_mac(ctxt);
    case LEARN_TYPE_MOVE_L2L:
        return process_l2l_mac_move(ctxt);
    case LEARN_TYPE_MOVE_R2L:
        return process_r2l_mac_move(ctxt);
    case LEARN_TYPE_NONE:
        return SDK_RET_OK;
    default:
        break;
    }
    return SDK_RET_ERR;
}

static sdk_ret_t
process_new_ip (local_learn_ctxt_t *ctxt)
{
    pds_local_mapping_spec_t spec{};
    uint32_t vnic_obj_id;

    vnic_obj_id = ctxt->mac_entry->vnic_obj_id();
    ctxt->ip_entry = ep_ip_entry::factory(&ctxt->ip_key, vnic_obj_id);
    if (unlikely(ctxt->ip_entry == nullptr)) {
        PDS_TRACE_ERR("Failed to allocate IP entry for EP %s", ctxt->str());
        ctxt->pkt_drop_reason = PKT_DROP_REASON_RES_ALLOC_FAIL;
        return SDK_RET_ERR;
    }

    // create local l3 mapping
    spec.skey.type = PDS_MAPPING_TYPE_L3;
    spec.skey.vpc = ctxt->ip_key.vpc;
    spec.skey.ip_addr = ctxt->ip_key.ip_addr;
    spec.vnic = api::uuid_from_objid(vnic_obj_id);
    spec.subnet = ctxt->mac_key.subnet;
    spec.num_tags = 0;
    MAC_ADDR_COPY(spec.vnic_mac, ctxt->mac_key.mac_addr);

    PDS_TRACE_INFO("Creating IP mapping for EP %s", ctxt->str());
    return pds_local_mapping_create(&spec, ctxt->bctxt);
}

static sdk_ret_t
process_l2l_ip_move (local_learn_ctxt_t *ctxt)
{
    return SDK_RET_INVALID_OP;
}

static sdk_ret_t
process_r2l_ip_move (local_learn_ctxt_t *ctxt)
{
    return SDK_RET_INVALID_OP;
}

static sdk_ret_t
process_ip_learn_info (local_learn_ctxt_t *ctxt)
{
    switch (ctxt->ip_learn_type) {
    case LEARN_TYPE_NEW_LOCAL:
        return process_new_ip(ctxt);
    case LEARN_TYPE_MOVE_L2L:
        return process_l2l_ip_move(ctxt);
    case LEARN_TYPE_MOVE_R2L:
        return process_r2l_ip_move(ctxt);
    case LEARN_TYPE_NONE:
        return SDK_RET_OK;
    default:
        break;
    }
    return SDK_RET_ERR;
}

static sdk_ret_t
validate_learn (local_learn_ctxt_t *ctxt)
{
    // TODO: validate that learning mac/IP is OK
    //
    // for MAC address, check if we have number of vnics limit, if vlan in pkt
    // does not match expected vlan on the subnet, or any such conditions
    //
    // for IP address, check if IP belongs to the associated mac's subnet,
    // if we have number of IP mappings per VNIC limit or any such criteria
    //

    return SDK_RET_OK;
}

static sdk_ret_t
process_learn_info (local_learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;

    // process MAC learn first and then IP learn as we need VNIC object id to
    // create IP mapping
    ret = process_mac_learn_info(ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to process MAC learn for EP %s (error code %u)",
                      ctxt->str(), ret);
        return ret;
    }
    if (ctxt->ip_learn_type != LEARN_TYPE_INVALID) {
        ret = process_ip_learn_info(ctxt);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to process MAC learn for EP %s "
                          "(error code %u)", ctxt->str(), ret);
        }
    }
    return ret;
}

static sdk_ret_t
update_ep_mac (local_learn_ctxt_t *ctxt)
{
    switch (ctxt->mac_learn_type) {
    case LEARN_TYPE_NONE:
        // nothing to do
        break;
    case LEARN_TYPE_NEW_LOCAL:
        ctxt->mac_entry->set_state(EP_STATE_CREATED);
        ctxt->mac_entry->add_to_db();
        // start MAC aging timer
        aging_timer_restart(ctxt->mac_entry->timer());
        broadcast_mac_event(EVENT_ID_MAC_LEARN, ctxt->mac_entry);
        LEARN_COUNTER_INCR(vnics);
        break;
    case LEARN_TYPE_MOVE_L2L:
    case LEARN_TYPE_MOVE_R2L:
    default:
        SDK_ASSERT_RETURN(false, SDK_RET_ERR);
    }
    return SDK_RET_OK;
}

static sdk_ret_t
update_ep_ip (local_learn_ctxt_t *ctxt)
{
    switch (ctxt->ip_learn_type) {
    case LEARN_TYPE_NONE:
        // we may be under ARP probe, update the state
        ctxt->ip_entry->set_state(EP_STATE_CREATED);
        break;
    case LEARN_TYPE_NEW_LOCAL:
        ctxt->ip_entry->set_state(EP_STATE_CREATED);
        ctxt->ip_entry->add_to_db();
        ctxt->mac_entry->add_ip(ctxt->ip_entry);
        broadcast_ip_event(EVENT_ID_IP_LEARN, ctxt->ip_entry);
        LEARN_COUNTER_INCR(l3_mappings);
        break;
    case LEARN_TYPE_MOVE_L2L:
    case LEARN_TYPE_MOVE_R2L:
    default:
        SDK_ASSERT_RETURN(false, SDK_RET_ERR);
    }

    // restart the aging timer for all cases
    // in case we received response to ARP probe, learn type is LEARN_TYPE_NONE
    // and we reset the timer here
    // since now at least one IP is active, stop MAC aging timer in case it is
    // started
    aging_timer_restart(ctxt->ip_entry->timer());
    aging_timer_stop(ctxt->mac_entry->timer());
    return SDK_RET_OK;
}

static sdk_ret_t
update_ep (local_learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;

    ret = update_ep_mac(ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to update EP %s MAC state (error code %u)",
                      ctxt->str(), ret);
        return ret;
    }

    if (ctxt->ip_learn_type != LEARN_TYPE_INVALID) {
        ret = update_ep_ip(ctxt);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to update EP %s IP state (error code %u)",
                          ctxt->str(), ret);
        }
    }

    PDS_TRACE_VERBOSE("Processed learn for EP %s", ctxt->str());
    return ret;
}

static inline sdk_ret_t
add_tx_pkt_hdr (void *mbuf, local_learn_ctxt_t *ctxt)
{
    impl::p4_tx_info_t tx_info;
    char *tx_hdr;

    // remove cpu to arm rx hdr and insert arm to cpu tx hdr
    tx_hdr = learn_lif_mbuf_rx_to_tx(mbuf);
    if (tx_hdr == nullptr) {
        ctxt->pkt_drop_reason = PKT_DROP_REASON_MBUF_ERR;
        return SDK_RET_ERR;
    }

    tx_info.slif = ctxt->impl_info.lif;
    tx_info.nh_type = impl::LEARN_NH_TYPE_NONE;
    impl::arm_to_p4_tx_hdr_fill(learn_lif_mbuf_data_start(mbuf), &tx_info);
    return SDK_RET_OK;
}

void
process_learn_pkt (void *mbuf)
{
    local_learn_ctxt_t ctxt = { 0 };
    sdk_ret_t ret;
    char *pkt_data = learn_lif_mbuf_data_start(mbuf);

    pds_batch_params_t batch_params {learn_db()->epoch_next(), false, nullptr,
                                      nullptr};

    // default drop reason
    ctxt.pkt_drop_reason = PKT_DROP_REASON_PARSE_ERR;

    // get impl specific p4 header results
    ret = impl::extract_info_from_p4_hdr(pkt_data, &ctxt.impl_info);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    // parse MAC and IP address and extract learn info
    ret = extract_learn_info(pkt_data, &ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    PDS_TRACE_VERBOSE("Rcvd learn pkt with info  %s", ctxt.str());

    // all required info is gathered, validate the learn
    ret = validate_learn(&ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    // now process the learn start API batch
    ctxt.bctxt = pds_batch_start(&batch_params);
    if (unlikely(ctxt.bctxt == PDS_BATCH_CTXT_INVALID)) {
        goto error;
    }

    // process learn info and update batch with required apis
    ret = process_learn_info(&ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    // submit the apis in sync mode
    ret = pds_batch_commit(ctxt.bctxt);
    LEARN_COUNTER_INCR(api_calls);
    if (unlikely(ret != SDK_RET_OK)) {
        LEARN_COUNTER_INCR(api_failure);
        goto error;
    }

    // we are done with the batch, clear batch context to avoid trying to free
    // it on error path
    ctxt.bctxt = PDS_BATCH_CTXT_INVALID;

    // apis executed successfully, update sw state and notify cp
    ret = update_ep(&ctxt);
    if (likely(ret == SDK_RET_OK)) {
        // send the packet to its nexthop
        if (unlikely(add_tx_pkt_hdr(mbuf, &ctxt) != SDK_RET_OK)) {
            goto error;
        }
        learn_lif_send_pkt(mbuf);
        return;
    }

error:

    // clean up any allocated resources
    ctxt.reset();
    learn_lif_drop_pkt(mbuf, ctxt.pkt_drop_reason);
}

}    // namespace learn
