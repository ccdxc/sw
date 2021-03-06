//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// utilities to handle EP MAC and IP entries
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/l2.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/internal/pds_mapping.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"
#include "nic/apollo/learn/ep_aging.hpp"
#include "nic/apollo/learn/learn_state.hpp"
#include "nic/apollo/learn/utils.hpp"

namespace event = sdk::event_thread;

namespace learn {

typedef struct ep_mac_clear_args_s {
    sdk_ret_t retcode;
    pds_obj_key_t subnet;
    pds_obj_key_t *lifs;
    uint8_t num_lifs;
    bool (*filter) (ep_mac_entry *mac_entry, struct ep_mac_clear_args_s *args);
} ep_mac_clear_args_t;

sdk_ret_t
delete_local_ip_mapping (ep_ip_entry *ip_entry, pds_batch_ctxt_t bctxt)
{
    pds_mapping_key_t mapping_key;
    const ep_ip_key_t *ep_ip_key = ip_entry->key();

    mapping_key.type = PDS_MAPPING_TYPE_L3;
    mapping_key.vpc = ep_ip_key->vpc;
    mapping_key.ip_addr = ep_ip_key->ip_addr ;
    return api::pds_local_mapping_delete(&mapping_key, bctxt);
}

sdk_ret_t
delete_ip_entry (ep_ip_entry *ip_entry, ep_mac_entry *mac_entry)
{
    sdk_ret_t ret;

    event::timer_stop(ip_entry->timer());
    ip_entry->set_state(EP_STATE_DELETED);
    mac_entry->del_ip(ip_entry);

    ret = ip_entry->del_from_db();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete %s from db, error code %u",
                      ip_entry->key2str().c_str(), ret);
        return ret;
    }
    PDS_TRACE_INFO("Deleted %s", ip_entry->key2str().c_str());

    ret = ip_entry->delay_delete();
    if (ret == SDK_RET_OK) {
        // if this was the last IP on this EP, start MAC aging timer
        if (mac_entry->ip_count() == 0) {
            mac_aging_timer_restart(mac_entry);
        }
    }
    return ret;
}

sdk_ret_t
delete_ip_from_ep (ep_ip_entry *ip_entry, ep_mac_entry *mac_entry)
{
    sdk_ret_t ret;

    ret = delete_local_ip_mapping(ip_entry, PDS_BATCH_CTXT_INVALID);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete %s, error code %u",
                      ip_entry->key2str().c_str(), ret);
        LEARN_COUNTER_INCR(local_ip_map_err[OP_DELETE]);

        // if the failure is because the entry was not found, continue to delete
        // the software state to keep it in sync with HAL
        if (ret != SDK_RET_ENTRY_NOT_FOUND) {
            return ret;
        }
    } else {
        LEARN_COUNTER_INCR(local_ip_map_ok[OP_DELETE]);
    }
    return delete_ip_entry(ip_entry, mac_entry);
}

sdk_ret_t
delete_vnic (ep_mac_entry *mac_entry, pds_batch_ctxt_t bctxt)
{
    pds_obj_key_t vnic_key;

    vnic_key = api::uuid_from_objid(mac_entry->vnic_obj_id());
    return pds_vnic_delete(&vnic_key, bctxt);
}

sdk_ret_t
delete_mac_entry (ep_mac_entry *mac_entry)
{
    sdk_ret_t ret;

    timer_stop(mac_entry->timer());
    mac_entry->set_state(EP_STATE_DELETED);
    learn_db()->vnic_obj_id_free(mac_entry->vnic_obj_id());

    ret = mac_entry->del_from_db();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete %s from db, error code %u",
                      mac_entry->key2str().c_str(), ret);
        return ret;
    }
    PDS_TRACE_INFO("Deleted %s", mac_entry->key2str().c_str());
    return mac_entry->delay_delete();
}

// note: caller should clear all IPs linked to this MAC
// before invoking this function.
sdk_ret_t
delete_ep (ep_mac_entry *mac_entry)
{
    sdk_ret_t ret;

    ret = delete_vnic(mac_entry, PDS_BATCH_CTXT_INVALID);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete EP %s, error code %u",
                      mac_entry->key2str().c_str(), ret);
        LEARN_COUNTER_INCR(vnic_err[OP_DELETE]);

        // if the failure is because the entry was not found, continue to delete
        // the software state to keep it in sync with HAL
        if (ret != SDK_RET_ENTRY_NOT_FOUND) {
            return ret;
        }
    } else {
        LEARN_COUNTER_INCR(vnic_ok[OP_DELETE]);
    }
    return delete_mac_entry(mac_entry);
}

static void
fill_learn_event (event_t *event, event_id_t learn_event,
                  pds_obj_key_t *vnic_key, ep_ip_entry *ip_entry)
{
    vnic_entry *vnic;
    core::learn_event_info_t *info = &event->learn;

    vnic = vnic_db()->find(vnic_key);
    if (unlikely(vnic == nullptr)) {
        PDS_TRACE_ERR("Failed to broadcast learn event %u for vnic %s, IP %s",
                      learn_event, vnic_key->str(), ip_entry ?
                      ipaddr2str(&ip_entry->key()->ip_addr) : "null");
        return;
    }
    event->event_id = learn_event;
    info->subnet = vnic->subnet();
    info->ifindex = api::objid_from_uuid(vnic->host_if());
    MAC_ADDR_COPY(info->mac_addr, vnic->mac());
    if (ip_entry) {
        info->vpc = ip_entry->key()->vpc;
        info->ip_addr = ip_entry->key()->ip_addr;
    } else {
        info->vpc = { 0 };
        info->ip_addr = { 0 };
    }
}

static void
fill_mac_event (event_t *event, event_id_t learn_event, ep_mac_entry *mac_entry)
{
    pds_obj_key_t vnic_key;

    vnic_key = api::uuid_from_objid(mac_entry->vnic_obj_id());
    fill_learn_event(event, learn_event, &vnic_key, nullptr);
}

static void
fill_ip_event (event_t *event, event_id_t learn_event, ep_ip_entry *ip_entry)
{
    pds_obj_key_t vnic_key;

    vnic_key = api::uuid_from_objid(ip_entry->vnic_obj_id());
    fill_learn_event(event, learn_event, &vnic_key, ip_entry);
}

sdk_ret_t
mac_ageout (ep_mac_entry *mac_entry)
{
    sdk_ret_t ret;
    event_t event;

    if (mac_entry->state() != EP_STATE_CREATED) {
        return SDK_RET_INVALID_OP;
    }
    // before MAC entry ages out, all the IP entries must have aged out
    if (unlikely(mac_entry->ip_count() != 0)) {
        PDS_TRACE_ERR("Failed to ageout EP %s, IP count %u",
                      mac_entry->key2str().c_str(), mac_entry->ip_count());
        return SDK_RET_INVALID_OP;
    }
    fill_mac_event(&event, EVENT_ID_MAC_AGE, mac_entry);
    ret = delete_ep(mac_entry);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to delete %s, error code %u",
                      mac_entry->key2str().c_str(), ret);
        return ret;
    }
    broadcast_learn_event(&event);
    return SDK_RET_OK;
}

sdk_ret_t
ip_ageout (ep_ip_entry *ip_entry)
{
    sdk_ret_t    ret;
    event_t      event;
    ep_mac_entry *mac_entry = ip_entry->mac_entry();

    if ((ip_entry->state() != EP_STATE_CREATED) &&
        (ip_entry->state() != EP_STATE_PROBING)) {
        return SDK_RET_INVALID_OP;
    }
    fill_ip_event(&event, EVENT_ID_IP_AGE, ip_entry);
    ret = delete_ip_from_ep(ip_entry, mac_entry);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    broadcast_learn_event(&event);
    return SDK_RET_OK;
}

static bool
ep_ip_entry_clear_cb (void *entry, void *retcode)
{
    ep_ip_entry *ip_entry = (ep_ip_entry *)entry;
    sdk_ret_t *ret = (sdk_ret_t *)retcode;

    *ret = ip_ageout(ip_entry);
    if (*ret != SDK_RET_OK) {
        // stop iterating
        return true;
    }
    return false;
}

static bool
ep_mac_entry_clear_cb (void *entry, void *ctxt)
{
    ep_mac_entry *mac_entry = (ep_mac_entry *)entry;
    ep_mac_clear_args_t *args = (ep_mac_clear_args_t *)ctxt;

    if (args->filter && !args->filter(mac_entry, args)) {
        // this entry does not match the filter, skip it
        return false;
    }
    mac_entry->walk_ip_list(ep_ip_entry_clear_cb, &args->retcode);
    if (args->retcode == SDK_RET_OK) {
        args->retcode = mac_ageout(mac_entry);
    }
    if (args->retcode != SDK_RET_OK) {
        // stop iterating
        return true;
    }
    return false;
}

sdk_ret_t
ep_mac_entry_clear (ep_mac_key_t *mac_key)
{
    sdk_ret_t ret = SDK_RET_OK;
    ep_mac_entry *mac_entry;

    mac_entry = learn_db()->ep_mac_db()->find(mac_key);
    if (mac_entry == nullptr) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    // clear all IPs linked to this MAC first
    mac_entry->walk_ip_list(ep_ip_entry_clear_cb, &ret);
    if (ret == SDK_RET_OK) {
        ret = mac_ageout(mac_entry);
    }
    return ret;
}

sdk_ret_t
ep_mac_entry_clear_all (void)
{
    ep_mac_clear_args_t args;

    args.retcode = SDK_RET_OK;
    args.filter = nullptr;
    learn_db()->ep_mac_db()->walk(ep_mac_entry_clear_cb, &args);
    return args.retcode;
}

sdk_ret_t
ep_ip_entry_clear (ep_ip_key_t *ip_key)
{
    ep_ip_entry *ip_entry;

    ip_entry = learn_db()->ep_ip_db()->find(ip_key);
    if (ip_entry == nullptr) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return ip_ageout(ip_entry);
}

sdk_ret_t
ep_ip_entry_clear_all (void)
{
    sdk_ret_t ret = SDK_RET_OK;

    learn_db()->ep_ip_db()->walk(ep_ip_entry_clear_cb, &ret);
    return ret;
}

static bool
mac_entry_filter_subnet (ep_mac_entry *mac_entry, ep_mac_clear_args_t *args)
{
    return mac_entry->key()->subnet == args->subnet;
}

sdk_ret_t
clear_all_eps_in_subnet (pds_obj_key_t key)
{
    ep_mac_clear_args_t args;

    args.retcode = SDK_RET_OK;
    args.subnet = key;
    args.filter = mac_entry_filter_subnet;
    learn_db()->ep_mac_db()->walk(ep_mac_entry_clear_cb, &args);
    return args.retcode;
}

static bool
mac_entry_filter_lifs (ep_mac_entry *mac_entry, ep_mac_clear_args_t *args)
{
    pds_obj_key_t vnic_key;
    vnic_entry *vnic;
    pds_obj_key_t host_if;

    vnic_key = api::uuid_from_objid(mac_entry->vnic_obj_id());
    vnic = vnic_db()->find(&vnic_key);
    if (unlikely(vnic == nullptr)) {
        PDS_TRACE_ERR("Failed to look up vnic with key %s", vnic_key.str());
        return false;
    }
    host_if = vnic->host_if();
    for (int i = 0; i < args->num_lifs; i++) {
        if (host_if == args->lifs[i]) {
            return true;
        }
    }
    return false;
}

sdk_ret_t
clear_all_eps_on_lifs (pds_obj_key_t *lifs, uint8_t num_lifs)
{
    ep_mac_clear_args_t args;

    args.retcode = SDK_RET_OK;
    args.lifs = lifs;
    args.num_lifs = num_lifs;
    args.filter = mac_entry_filter_lifs;
    learn_db()->ep_mac_db()->walk(ep_mac_entry_clear_cb, &args);
    return args.retcode;
}

void
send_arp_probe (vnic_entry *vnic, ipv4_addr_t v4_addr)
{
    void *mbuf;
    char *tx_hdr;
    eth_hdr_t *eth_hdr;
    arp_hdr_t *arp_hdr;
    arp_data_ipv4_t *arp_data;
    impl::p4_tx_info_t tx_info = { 0 };
    pds_obj_key_t subnet_key;
    subnet_entry *subnet;

    subnet_key = vnic->subnet();
    subnet = subnet_db()->find(&subnet_key);
    if (unlikely(subnet == nullptr)) {
        PDS_TRACE_ERR("Failed to send ARP probe to %s, subnet lookup error for "
                      "IP %s", ipv4addr2str(v4_addr), subnet_key.str());
        LEARN_COUNTER_INCR(arp_probes_err);
        return;
    }

    mbuf = learn_lif_alloc_mbuf();
    if (unlikely(mbuf == nullptr)) {
        PDS_TRACE_ERR("Failed to allocate pkt buffer for ARP probe, IP %s, "
                      "%s, %s", ipv4addr2str(v4_addr),
                      subnet->key2str().c_str(), vnic->key2str().c_str());
        LEARN_COUNTER_INCR(arp_probes_err);
        return;
    }
    tx_hdr = learn_lif_mbuf_append_data(mbuf, ARP_PKT_ETH_FRAME_LEN +
                                        impl::arm_to_p4_hdr_sz());

    // fill Ethernet header, P4 adds encap header if required
    eth_hdr = (eth_hdr_t *)(tx_hdr + impl::arm_to_p4_hdr_sz());
    MAC_ADDR_COPY(eth_hdr->dmac, vnic->mac());
    MAC_ADDR_COPY(eth_hdr->smac, subnet->vr_mac());
    eth_hdr->eth_type = htons(ETH_TYPE_ARP);

    // fill ARP header
    arp_hdr = (arp_hdr_t *)(eth_hdr + 1);
    arp_hdr->htype = htons(ARP_HRD_TYPE_ETHER);
    arp_hdr->ptype = htons(ETH_TYPE_IPV4);
    arp_hdr->hlen = ETH_ADDR_LEN;
    arp_hdr->plen = IP4_ADDR8_LEN;
    arp_hdr->op = htons(ARP_OP_REQUEST);

    // fill ARP data
    arp_data = (arp_data_ipv4_t *)(arp_hdr + 1);
    MAC_ADDR_COPY(arp_data->smac, subnet->vr_mac());
    arp_data->sip = 0;
    memset(arp_data->tmac, 0, ETH_ADDR_LEN);
    arp_data->tip = htonl(v4_addr);

    // padding
    memset((arp_data + 1), 0, ARP_PKT_ETH_FRAME_LEN - ARP_PKT_LEN);

    // add ARM to P4 tx header and send the pkt
    tx_info.nh_type = impl::LEARN_NH_TYPE_VNIC;
    tx_info.vnic_key = vnic->key();
    impl::arm_to_p4_tx_hdr_fill(tx_hdr, &tx_info);
    learn_lif_send_pkt(mbuf);
    LEARN_COUNTER_INCR(arp_probes_ok);
}

void
send_arp_probe (ep_ip_entry *ip_entry)
{
    pds_obj_key_t vnic_key;
    vnic_entry *vnic;
    const ip_addr_t *ip_addr = &ip_entry->key()->ip_addr;

    vnic_key = api::uuid_from_objid(ip_entry->vnic_obj_id());
    vnic = vnic_db()->find(&vnic_key);
    if (unlikely(vnic == nullptr)) {
        PDS_TRACE_ERR("Failed to send ARP probe to %s, vnic lookup error for "
                      "key %s", ipaddr2str(ip_addr), vnic_key.str());
        return;
    }
    send_arp_probe(vnic, ip_addr->addr.v4_addr);
}

}    // namespace learn
