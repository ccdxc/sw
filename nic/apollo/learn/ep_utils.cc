//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// utilities to handle EP MAC and IP entries
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/internal/pds_mapping.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn_state.hpp"

namespace event = sdk::event_thread;

namespace learn {

static sdk_ret_t
delete_ip_mapping (ep_ip_entry *ip_entry, pds_batch_ctxt_t bctxt)
{
    pds_mapping_key_t mapping_key;
    const ep_ip_key_t *ep_ip_key = ip_entry->key();

    mapping_key.type = PDS_MAPPING_TYPE_L3;
    mapping_key.vpc = ep_ip_key->vpc;
    mapping_key.ip_addr = ep_ip_key->ip_addr ;
    return api::pds_local_mapping_delete(&mapping_key, bctxt);
}

static sdk_ret_t
delete_ip_entry (ep_ip_entry *ip_entry)
{
    ep_mac_entry *mac_entry = ip_entry->mac_entry();
    sdk_ret_t ret;

    event::timer_stop(ip_entry->timer());
    ip_entry->set_state(EP_STATE_DELETED);
    mac_entry->del_ip(ip_entry);

    ret = ip_entry->del_from_db();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete EP %s from db, error code %u",
                      ip_entry->key2str(), ret);
        return ret;
    }
    return ip_entry->delay_delete();
}

sdk_ret_t
delete_ip_from_ep (ep_ip_entry *ip_entry, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    ret = delete_ip_mapping(ip_entry, bctxt);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete IP mapping for EP %s, error code %u",
                      ip_entry->key2str(), ret);
        return ret;
    }

    PDS_TRACE_INFO("Deleting IP mapping %s", ip_entry->key2str());
    return delete_ip_entry(ip_entry);
}

static bool
delete_ip_mapping_cb (void *obj, void *ctxt)
{
    ep_ip_entry *ip_entry = (ep_ip_entry *)obj;
    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID;

    if (ctxt) {
        bctxt = *((pds_batch_ctxt_t *)ctxt);
    }
    return (delete_ip_mapping(ip_entry, bctxt) == SDK_RET_OK);
}

static bool
delete_ip_entry_cb (void *obj, void *ctxt)
{
    ep_ip_entry *ip_entry = (ep_ip_entry *)obj;

    return (delete_ip_entry(ip_entry) == SDK_RET_OK);
}

static sdk_ret_t
delete_vnic (ep_mac_entry *mac_entry, pds_batch_ctxt_t bctxt)
{
    pds_obj_key_t vnic_key;

    vnic_key = api::uuid_from_objid(mac_entry->vnic_obj_id());
    return pds_vnic_delete(&vnic_key, bctxt);
}

static sdk_ret_t
delete_mac_entry (ep_mac_entry *mac_entry)
{
    sdk_ret_t ret;

    timer_stop(mac_entry->timer());
    mac_entry->set_state(EP_STATE_DELETED);
    learn_db()->vnic_obj_id_free(mac_entry->vnic_obj_id());

    ret = mac_entry->del_from_db();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete EP %s from db, error code %u",
                      mac_entry->key2str(), ret);
        return ret;
    }
    return mac_entry->delay_delete();
}

// note: caller needs to check if it is expected that there be no IPs associated
// with the EP before deleting it
// note: if caller provides batch, we commit it here TODO: split delete api into
// hardware state delete and software state delete, this would help us batch all
// deletes into a single batch when deleting all endpoints under a subnet
sdk_ret_t
delete_ep (ep_mac_entry *mac_entry, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    // delete all IP mappings and then MAC mapping
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        // start a batch so that all IP mappings and vnic can be deleted
        // together, if successful, then only we will delete sw state
        // this way, we do not end up with inconsistent hw and sw states
        pds_batch_params_t batch_params {learn_db()->epoch_next(), false,
                                         nullptr, nullptr};
        bctxt = pds_batch_start(&batch_params);
        if (unlikely(bctxt == PDS_BATCH_CTXT_INVALID)) {
            PDS_TRACE_ERR("Failed to create api batch");
            return SDK_RET_ERR;
        }
    }

    mac_entry->walk_ip_list(delete_ip_mapping_cb, (void *)&bctxt);
    ret = delete_vnic(mac_entry, bctxt);
    if (ret != SDK_RET_OK) {
        pds_batch_destroy(bctxt);
        PDS_TRACE_ERR("Failed to delete EP %s, error code %u",
                      mac_entry->key2str(), ret);
        return ret;
    }

    ret = pds_batch_commit(bctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to commit API batch, error code %u", ret);
        return SDK_RET_ERR;
    }
    PDS_TRACE_INFO("Deleted EP %s", mac_entry->key2str());

    // delete sw state for all IP entries
    mac_entry->walk_ip_list(delete_ip_entry_cb, nullptr);
    return delete_mac_entry(mac_entry);
}

void
send_arp_probe (ep_ip_entry *ip_entry)
{
    //TODO: construct ARP probe packet
    // allocate mbuf on learn lif
    // get nexthop from vnic associated with this IP
    // add VLAN header if vnic has encap
    // create ARP request header
    // add arm to p4 tx header with nh_type = LEARN_NH_TYPE_VNIC
}

static void
broadcast_learn_event (event_id_t learn_event, vnic_entry *vnic,
                       ep_ip_entry *ip_entry)
{
    ::core::event_t event;
    ::core::learn_event_info_t *info = &event.learn;

    event.event_id = learn_event;
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
    sdk::ipc::broadcast(learn_event, &event, sizeof(event));
}

void
broadcast_mac_event (event_id_t event, ep_mac_entry *mac_entry)
{
    pds_obj_key_t vnic_key;
    vnic_entry *vnic;

    vnic_key = api::uuid_from_objid(mac_entry->vnic_obj_id());
    vnic = vnic_db()->find(&vnic_key);
    broadcast_learn_event(event, vnic, nullptr);
}

void
broadcast_ip_event (event_id_t event, ep_ip_entry *ip_entry)
{
    pds_obj_key_t vnic_key;
    vnic_entry *vnic;

    vnic_key = api::uuid_from_objid(ip_entry->vnic_obj_id());
    vnic = vnic_db()->find(&vnic_key);
    broadcast_learn_event(event, vnic, ip_entry);
}

}    // namespace learn
