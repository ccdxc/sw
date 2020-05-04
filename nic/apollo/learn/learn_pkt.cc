//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// process packets received on learn lif
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/include/sdk/l2.hpp"
#include "nic/sdk/include/sdk/l4.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/learn/ep_aging.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn_ctxt.hpp"
#include "nic/apollo/learn/utils.hpp"

namespace learn {

using namespace sdk::types;

#define ep_mac_db       learn_db()->ep_mac_db
#define ep_ip_db        learn_db()->ep_ip_db

// find out if given MAC or IP is learnt on remote TEP
static bool
remote_mapping_find (learn_ctxt_t *ctxt, pds_mapping_type_t mapping_type)
{
    pds_mapping_key_t mkey;
    sdk_ret_t ret;

    if (mapping_type == PDS_MAPPING_TYPE_L2) {
        ep_mac_to_pds_mapping_key(&ctxt->mac_key, &mkey);
    } else {
        ep_ip_to_pds_mapping_key(&ctxt->ip_key, &mkey);
    }
    ret = impl::remote_mapping_find(&mkey);
    SDK_ASSERT((ret == SDK_RET_OK || ret == SDK_RET_ENTRY_NOT_FOUND));
    return (ret == SDK_RET_OK);
}

static bool
detect_l2l_move (learn_ctxt_t *ctxt, pds_mapping_type_t mapping_type)
{
    pds_obj_key_t vnic_key;
    vnic_entry *vnic;
    pds_ifindex_t vnic_ifindex;
    pds_ifindex_t pkt_ifindex;
    bool l2l_move = false;
    impl::learn_info_t  *impl = &ctxt->pkt_ctxt.impl_info;
    const mac_addr_t *vnic_mac;

    if (mapping_type == PDS_MAPPING_TYPE_L2) {
        // L2L move for MAC
        // 1) same MAC, subnet is now associated with different LIF, subnet has
        //    moved LIFs, in this case, update VNIC but keep IPs learnt
        // 2) MAC is seen with different VLAN tag than what we learnt before,
        //    in this case, update VNIC with new VLAN tag and keep IPs intact
        // 3) VLAN has changed
        //    note, if both LIF and VLAN changed,we will handle correctly since
        //    VNIC spec is always rebuilt
        vnic_key = api::uuid_from_objid(ctxt->mac_entry->vnic_obj_id());
        vnic = vnic_db()->find(&vnic_key);
        vnic_ifindex = api::objid_from_uuid(vnic->host_if());
        pkt_ifindex = (pds_ifindex_t) LIF_IFINDEX(impl->lif);

        // encap comparison assumes encap type is 802.1Q
        l2l_move = (vnic_ifindex != pkt_ifindex) ||
                   (vnic->vnic_encap().val.vlan_tag !=
                    impl->encap.val.vlan_tag);

        if (l2l_move) {
            ctxt->mac_move_log = "attributes changed ifindex-(" +
                                to_string(vnic_ifindex) + ", " +
                                to_string(pkt_ifindex) + "), " +
                                "VLAN TAG-(" +
                                to_string(vnic->vnic_encap().val.vlan_tag) +
                                ", " + to_string(impl->encap.val.vlan_tag) +
                                ")";
        }
    } else {
        // check if existing MAC address mapped to this IP is different from the
        // one seen in the packet, if yes, move the IP to the MAC seen in the
        // packet, this could be an existing MAC or a new MAC
        vnic_mac = &ctxt->ip_entry->mac_entry()->key()->mac_addr;
        l2l_move = (memcmp(vnic_mac, ctxt->mac_key.mac_addr, ETH_ADDR_LEN)
                    != 0);
        if (l2l_move) {
            ctxt->ip_move_log = "MAC address changed (" +
                                std::string(macaddr2str(*vnic_mac)) + ", " +
                                std::string(macaddr2str(ctxt->mac_key.mac_addr))
                                + ")";
        }
    }
    return l2l_move;
}

static ep_learn_type_t
detect_learn_type (learn_ctxt_t *ctxt, pds_mapping_type_t mapping_type)
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
extract_ip_learn_info (char *pkt_data, learn_ctxt_t *ctxt)
{
    ip_addr_t *ip = &ctxt->ip_key.ip_addr;
    impl::learn_info_t *impl = &ctxt->pkt_ctxt.impl_info;

    switch (impl->pkt_type) {
    case PKT_TYPE_DHCP:
    {
        dhcp_header_t *dhcp = (dhcp_header_t *) (pkt_data +
                                                 impl->l3_offset +
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
        if (!dhcp_request || !ip_found) {
            return false;
        }
        break;
    }
    case PKT_TYPE_ARP:
    {
        arp_hdr_t *arp_hdr = (arp_hdr_t *) (pkt_data + impl->l3_offset);
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
        IPV4_HDR_SIP_GET(pkt_data + impl->l3_offset, ip->addr.v4_addr);
        ip->af = IP_AF_IPV4;
        break;
    case PKT_TYPE_NDP:
    case PKT_TYPE_IPV6:
    default:
        return false;
    }

    // check for zeroed out IPv4 address
    if (ip->af == IP_AF_IPV4 && ip->addr.v4_addr == 0) {
        return false;
    }
    ctxt->ip_key.vpc = subnet_db()->find(&ctxt->mac_key.subnet)->vpc();
    return true;
}

static sdk_ret_t
extract_learn_info (char *pkt_data, learn_ctxt_t *ctxt)
{
    char *src_mac;
    impl::learn_info_t *impl = &ctxt->pkt_ctxt.impl_info;

    // MAC addr is always available, populate ep->mac_key
    src_mac = pkt_data + impl->l2_offset + ETH_ADDR_LEN;
    MAC_ADDR_COPY(&ctxt->mac_key.mac_addr, src_mac);
    ctxt->mac_key.subnet = impl->subnet;
    ctxt->mac_entry = ep_mac_db()->find(&ctxt->mac_key);
    ctxt->mac_learn_type = detect_learn_type(ctxt, PDS_MAPPING_TYPE_L2);

    // construct ifindex
    ctxt->ifindex = LIF_IFINDEX(impl->lif);

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
validate_learn (learn_ctxt_t *ctxt)
{
    subnet_entry  *subnet;
    ipv4_prefix_t v4_prefix;
    ip_prefix_t   v6_prefix;

    if (!ctxt->mac_entry && ((ctxt->mac_learn_type == LEARN_TYPE_NEW_LOCAL)
        || (ctxt->mac_learn_type == LEARN_TYPE_MOVE_R2L))) {
        // validate max limit for mac entries if it is a new learn or R2L move.
        if (ep_mac_db()->num_entries() >= EP_MAX_MAC_ENTRY) {
            PDS_TRACE_ERR("Already reached maximum limit for MACs, dropping "
                          "EP %s", ctxt->str());
            LEARN_COUNTER_INCR(validation_err[MAC_LIMIT]);
            return SDK_RET_ERR;
        }
        // only one untagged vnic per subnet is allowed, since untagged vnics
        //  use hostif's vnic_hw_id.
        if (ctxt->pkt_ctxt.impl_info.encap.type == PDS_ENCAP_TYPE_NONE) {
            if (impl::untagged_vnic_exists_on_lif(
                                           ctxt->pkt_ctxt.impl_info.lif)) {
                PDS_TRACE_ERR("Only one untagged VNIC supported per LIF, "
                              "dropping EP %s", ctxt->str());
                LEARN_COUNTER_INCR(validation_err[UNTAGGED_MAC_LIMIT]);
                return SDK_RET_ERR;
            }
        }

    }

    // validate only if it is a new learn or R2L move.
    if (!ctxt->ip_entry && ((ctxt->ip_learn_type == LEARN_TYPE_NEW_LOCAL)
        || (ctxt->ip_learn_type == LEARN_TYPE_MOVE_R2L)
        || (ctxt->ip_learn_type == LEARN_TYPE_MOVE_L2L))) {
        // validate max limit for ip entries only if it is not a L2L move. For
        // L2L move, we update/delete the old entry and total remains same.
        if ((ctxt->ip_learn_type != LEARN_TYPE_MOVE_L2L) &&
           (ep_ip_db()->num_entries() >= EP_MAX_IP_ENTRY)) {
            PDS_TRACE_ERR("Already reached maximum limit for IPs, dropping EP "
                          "%s", ctxt->str());
            LEARN_COUNTER_INCR(validation_err[IP_LIMIT]);
            return SDK_RET_ERR;
        }
        // validate if IP belongs to the subnet. subnet should be present now,
        // since bdid to subnetid lookup was successful
        subnet = subnet_db()->find(&ctxt->mac_key.subnet);
        if(ctxt->ip_key.ip_addr.af == IP_AF_IPV4) {
            v4_prefix = subnet->v4_prefix();
            if (!ipv4_addr_within_prefix(&v4_prefix,
                                         &ctxt->ip_key.ip_addr.addr.v4_addr)) {
                PDS_TRACE_ERR("IP address %s does not belong to the subnet %s",
                              ipaddr2str(&ctxt->ip_key.ip_addr),
                              ipv4pfx2str(&v4_prefix));
                LEARN_COUNTER_INCR(validation_err[IP_ADDR_SUBNET_MISMATCH]);
                return SDK_RET_ERR;
            }
        } else {
            v6_prefix = subnet->v6_prefix();
            if (!ip_addr_within_prefix(&v6_prefix, &ctxt->ip_key.ip_addr)) {
                PDS_TRACE_ERR("IPv6 address %s doesnt belong to the subnet %s",
                              ipaddr2str(&ctxt->ip_key.ip_addr),
                              ippfx2str(&v6_prefix));
                LEARN_COUNTER_INCR(validation_err[IP_ADDR_SUBNET_MISMATCH]);
                return SDK_RET_ERR;
            }
        }
    }
    return SDK_RET_OK;
}

static sdk_ret_t
update_ep_mac (learn_ctxt_t *ctxt)
{
    switch (ctxt->mac_learn_type) {
    case LEARN_TYPE_NONE:
        // nothing to do
        break;
    case LEARN_TYPE_NEW_LOCAL:
    case LEARN_TYPE_MOVE_R2L:
        ctxt->mac_entry->set_state(EP_STATE_CREATED);
        ctxt->mac_entry->add_to_db();
        // start MAC aging timer
        mac_aging_timer_restart(ctxt->mac_entry);

        // for R2L we broadcast both new learn and r2l messages
        add_mac_to_event_list(ctxt, EVENT_ID_MAC_LEARN);
        if (ctxt->mac_learn_type == LEARN_TYPE_MOVE_R2L) {
            add_mac_to_event_list(ctxt, EVENT_ID_MAC_MOVE_R2L);
        }
        break;
    case LEARN_TYPE_MOVE_L2L:
        // nothing to do
        // TODO: do we need to broadcast event to MS?
        break;
    default:
        SDK_ASSERT_RETURN(false, SDK_RET_ERR);
    }
    return SDK_RET_OK;
}

static sdk_ret_t
update_ep_ip (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;
    ep_ip_entry *old_ip_entry;

    switch (ctxt->ip_learn_type) {
    case LEARN_TYPE_NONE:
        // we may be under ARP probe, update the state
        ctxt->ip_entry->set_state(EP_STATE_CREATED);
        break;
    case LEARN_TYPE_NEW_LOCAL:
    case LEARN_TYPE_MOVE_R2L:
        ctxt->ip_entry->set_state(EP_STATE_CREATED);
        ctxt->ip_entry->add_to_db();
        ctxt->mac_entry->add_ip(ctxt->ip_entry);

        // for R2L we broadcast both new learn and r2l messages
        add_ip_to_event_list(ctxt, EVENT_ID_IP_LEARN);
        if (ctxt->ip_learn_type == LEARN_TYPE_MOVE_R2L) {
            add_ip_to_event_list(ctxt, EVENT_ID_IP_MOVE_R2L);
        }
        break;
    case LEARN_TYPE_MOVE_L2L:
        old_ip_entry = ctxt->pkt_ctxt.old_ip_entry;
        // TODO: remove this check once deletes are batched with create
        if (old_ip_entry) {
            ret = delete_ip_entry(old_ip_entry, old_ip_entry->mac_entry());
            if (ret!= SDK_RET_OK) {
                // newly allocated ip entry is cleaned up by ctxt reset
                return ret;
            }
        }
        ctxt->ip_entry->set_state(EP_STATE_CREATED);
        ctxt->ip_entry->add_to_db();
        ctxt->mac_entry->add_ip(ctxt->ip_entry);
        break;
    default:
        SDK_ASSERT_RETURN(false, SDK_RET_ERR);
    }

    // restart the aging timer for all cases
    // in case we received response to ARP probe, learn type is LEARN_TYPE_NONE
    // and we reset the timer here
    // since now at least one IP is active, stop MAC aging timer in case it is
    // started
    ip_aging_timer_restart(ctxt->ip_entry);
    mac_aging_timer_stop(ctxt->mac_entry);
    return SDK_RET_OK;
}

static sdk_ret_t
update_ep (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;

    ret = update_ep_mac(ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to update EP %s MAC state (error code %u)",
                      ctxt->str(), ret);
        return ret;
    }
    PDS_TRACE_INFO("Processed %s", ctxt->log_str(PDS_MAPPING_TYPE_L2));

    if (ctxt->ip_learn_type != LEARN_TYPE_INVALID) {
        ret = update_ep_ip(ctxt);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to update EP %s IP state (error code %u)",
                          ctxt->str(), ret);
        }
        PDS_TRACE_INFO("Processed %s", ctxt->log_str(PDS_MAPPING_TYPE_L3));
    }
    return ret;
}

static inline sdk_ret_t
add_tx_pkt_hdr (void *mbuf, learn_ctxt_t *ctxt)
{
    impl::p4_tx_info_t tx_info;
    char *tx_hdr;

    // remove cpu to arm rx hdr and insert arm to cpu tx hdr
    tx_hdr = learn_lif_mbuf_rx_to_tx(mbuf);
    if (tx_hdr == nullptr) {
        ctxt->pkt_ctxt.pkt_drop_reason = PKT_DROP_REASON_MBUF_ERR;
        return SDK_RET_ERR;
    }

    tx_info.slif = ctxt->pkt_ctxt.impl_info.lif;
    tx_info.nh_type = impl::LEARN_NH_TYPE_NONE;
    return impl::arm_to_p4_tx_hdr_fill(learn_lif_mbuf_data_start(mbuf),
                                       &tx_info);
}

static sdk_ret_t
reinject_pkt_to_p4 (void *mbuf, learn_ctxt_t *ctxt)
{
    // check if packet needs to be dropped
    if (ctxt->pkt_ctxt.impl_info.hints & LEARN_HINT_ARP_REPLY) {
        learn_lif_drop_pkt(mbuf, PKT_DROP_REASON_ARP_REPLY);
        return SDK_RET_OK;
    }

    // reinject packet to p4
    if (likely(add_tx_pkt_hdr(mbuf, ctxt) == SDK_RET_OK)) {
        learn_lif_send_pkt(mbuf);
        return SDK_RET_OK;
    }
    return SDK_RET_ERR;
}

void
process_learn_pkt (void *mbuf)
{
    learn_ctxt_t ctxt = { 0 };
    learn_batch_ctxt_t lbctxt;
    sdk_ret_t ret = SDK_RET_ERR;
    char *pkt_data = learn_lif_mbuf_data_start(mbuf);
#ifdef BATCH_SUPPORT
    pds_batch_params_t batch_params {learn_db()->epoch_next(), false, nullptr,
                                      nullptr};
#endif
    bool has_learn_info = false;

    ctxt.ctxt_type = LEARN_CTXT_TYPE_PKT;
    memset(&lbctxt.counters, 0, sizeof(lbctxt.counters));
    ctxt.lbctxt = &lbctxt;

    // default drop reason
    ctxt.pkt_ctxt.pkt_drop_reason = PKT_DROP_REASON_PARSE_ERR;

    // get impl specific p4 header results
    ret = impl::extract_info_from_p4_hdr(pkt_data, &ctxt.pkt_ctxt.impl_info);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    // parse MAC and IP address and extract learn info
    ret = extract_learn_info(pkt_data, &ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    PDS_TRACE_DEBUG("Learn context %s", ctxt.str());
    has_learn_info = true;
    ctxt.pkt_ctxt.pkt_drop_reason = PKT_DROP_REASON_LEARNING_FAIL;

    // all required info is gathered, validate the learn
    ret = validate_learn(&ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    // now process the learn start API batch
#ifdef BATCH_SUPPORT
    ctxt.bctxt = pds_batch_start(&batch_params);
    if (unlikely(ctxt.bctxt == PDS_BATCH_CTXT_INVALID)) {
        PDS_TRACE_ERR("Failed to start a batch for %s", ctxt.str());
        ctxt.pkt_drop_reason = PKT_DROP_REASON_RES_ALLOC_FAIL;
        goto error;
    }
#else
    // each API is committed individually
    ctxt.bctxt = PDS_BATCH_CTXT_INVALID;
#endif

    // process learn info and update batch with required apis
    ret = process_learn(&ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

#ifdef BATCH_SUPPORT
    // submit the apis in sync mode
    ret = pds_batch_commit(ctxt.bctxt);
    ctxt.bctxt = PDS_BATCH_CTXT_INVALID;
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }
#endif

    // apis executed successfully, update sw state and notify cp
    ret = update_ep(&ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }
    update_batch_counters(&lbctxt, true);

    // broadcast learn events
    broadcast_events(&lbctxt);

    // reinject the packet to p4 if needed
    if (reinject_pkt_to_p4(mbuf, &ctxt) == SDK_RET_OK) {
        return;
    }
    // packet not reinjected, drop it

error:

    if (has_learn_info) {
        PDS_TRACE_ERR("Failed to process %s",
                       ctxt.log_str(PDS_MAPPING_TYPE_L2));
        if (ctxt.ip_learn_type != LEARN_TYPE_INVALID) {
            PDS_TRACE_ERR("Failed to process %s",
                          ctxt.log_str(PDS_MAPPING_TYPE_L3));
        }
    }
    if (ret != SDK_RET_OK) {
        update_batch_counters(&lbctxt, false);
    }

    // clean up any allocated resources
    learn_lif_drop_pkt(mbuf, ctxt.pkt_ctxt.pkt_drop_reason);
    ctxt.reset();
}

}    // namespace learn
