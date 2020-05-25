//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// utilities to handle EP MAC and IP entries
///
//----------------------------------------------------------------------------

#ifndef __LEARN_EP_LEARN_UTILS_HPP__
#define __LEARN_EP_LEARN_UTILS_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/learn/ep_ip_state.hpp"
#include "nic/apollo/learn/ep_mac_state.hpp"

using core::event_t;

namespace learn {

/// \brief delete local IP mapping
sdk_ret_t delete_local_ip_mapping(ep_ip_entry *ip_entry, pds_batch_ctxt_t bctxt);

/// \brief delete IP entry state
sdk_ret_t delete_ip_entry(ep_ip_entry *ip_entry, ep_mac_entry *mac_entry);

/// \brief delete IP mapping and entry from endpoint
sdk_ret_t delete_ip_from_ep(ep_ip_entry *ip_entry, ep_mac_entry *mac_entry);

/// \brief delete VNIC
sdk_ret_t delete_vnic(ep_mac_entry *mac_entry, pds_batch_ctxt_t bctxt);

/// \brief delete  MAC entry state
sdk_ret_t delete_mac_entry(ep_mac_entry *mac_entry);

/// \brief delete MAC
/// \brief delete endpoint
/// this deletes MAC entry and all associated IP entries of the endpoint
sdk_ret_t delete_ep(ep_mac_entry *mac_entry);

/// \bried delete all VNICs and IP mappings belonging to a subnet
sdk_ret_t delete_all_eps_in_subnet(pds_obj_key_t key);

/// \brief send ARP probe for associated IP
void send_arp_probe(ep_ip_entry *ip_entry);

/// \brief send ARP probe for given v4 addr towards given VNIC
void send_arp_probe(vnic_entry *vnic, ipv4_addr_t v4_addr);

/// \brief get vnic key from endpoint
static inline pds_obj_key_t
ep_vnic_key (ep_mac_entry *mac_entry)
{
    return (api::uuid_from_objid(mac_entry->vnic_obj_id()));
}

/// \brief broadcast learn event to subscribers
static inline void
broadcast_learn_event (event_t *event)
{
    sdk::ipc::broadcast(event->event_id, event, sizeof(*event));
}

/// \brief utility function to get l2 mapping key from ep key
static inline void
ep_mac_to_pds_mapping_key (ep_mac_key_t *ep_mac_key, pds_mapping_key_t *mkey)
{
    mkey->type = PDS_MAPPING_TYPE_L2;
    mkey->subnet = ep_mac_key->subnet;
    MAC_ADDR_COPY(mkey->mac_addr, ep_mac_key->mac_addr);
}

/// \brief utility function to get l3 mapping key from ep key
static inline void
ep_ip_to_pds_mapping_key (ep_ip_key_t *ep_ip_key, pds_mapping_key_t *mkey)
{
    mkey->type = PDS_MAPPING_TYPE_L3;
    mkey->vpc = ep_ip_key->vpc;
    mkey->ip_addr = ep_ip_key->ip_addr;
}

/// \brief ageout MAC entry
sdk_ret_t mac_ageout(ep_mac_entry *mac_entry);

/// \brief ageout IP entry
sdk_ret_t ip_ageout(ep_ip_entry *ip_entry);

/// \brief clear IP entry
sdk_ret_t ep_ip_entry_clear(ep_ip_key_t *ip_key);

/// \brief clear all IP entries
sdk_ret_t ep_ip_entry_clear_all(void);

/// \brief clear a MAC entry and all associated IP entries
sdk_ret_t ep_mac_entry_clear(ep_mac_key_t *mac_key);

/// \brief clear all MAC entries and their associated IP entries
sdk_ret_t ep_mac_entry_clear_all(void);

/// \brief clear all MAC and IP entries belonging to a subnet
sdk_ret_t clear_all_eps_in_subnet(pds_obj_key_t key);

/// \brief clear all MAC and IP entries attached to any of the given lifs
sdk_ret_t clear_all_eps_on_lifs(pds_obj_key_t *lifs, uint8_t num_lifs);

}    // namespace learn

#endif    // __LEARN_EP_LEARN_UTILS_HPP__
