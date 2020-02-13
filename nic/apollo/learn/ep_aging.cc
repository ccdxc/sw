//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// endpoint ageout handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/learn/ep_aging.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn_state.hpp"

namespace learn {

namespace event = sdk::event_thread;

static void
learn_ep_mac_aging_cb (event::timer_t *timer)
{
    ep_mac_entry *mac_entry = (ep_mac_entry *) timer->ctx;
    sdk_ret_t ret;

    if (unlikely(mac_entry->state() != EP_STATE_CREATED)) {
        // timeout cannot be active in any other state
        SDK_TRACE_ERR("Failed to ageout EP %s, invalid state %u at timeout",
                      mac_entry->key2str(), mac_entry->state());
        return;
    }

    // before MAC entry ages out, all the IP entries must have aged out
    if (unlikely(mac_entry->ip_count() != 0)) {
        SDK_TRACE_ERR("Failed to ageout EP %s, IP count %u",
                      mac_entry->key2str(), mac_entry->ip_count());
        return;
    }

    ret = delete_ep(mac_entry);
    if (unlikely(ret != SDK_RET_OK)) {
        SDK_TRACE_ERR("Failed to delete EP %s, error code %u",
                      mac_entry->key2str(), ret);
    }
}

static void
learn_ep_ip_aging_cb (event::timer_t *timer)
{
    ep_ip_entry *ip_entry = (ep_ip_entry *) timer->ctx;

    if ((ip_entry->state() != EP_STATE_CREATED) ||
        (ip_entry->state() != EP_STATE_PROBING)) {
        SDK_TRACE_ERR("Failed to ageout EP %s state at timeout %u",
                      ip_entry->key2str(), ip_entry->state());
        return;
    }

    if (ip_entry->state() == EP_STATE_PROBING) {
        if (ip_entry->arp_probe_count() == MAX_NUM_ARP_PROBES) {
            // we did not receive reply to ARP probe, despite retries,
            // delete IP mapping
            if (delete_ip_from_ep(ip_entry) == SDK_RET_OK) {
                // TODO: send EVPN event
            }
            return;
        }
    } else {
        // enter probing state, reset ARP probe count
        ip_entry->arp_probe_count_reset();
        ip_entry->set_state(EP_STATE_PROBING);
    }

    // send arp probe
    send_arp_probe(ip_entry);
    ip_entry->arp_probe_count_incr();

    // start timer for arp probe response
    event::timer_set(timer, (float) learn_db()->arp_probe_timeout(), 0.0);
    event::timer_start(timer);
}

void
learn_ep_aging_timer_init (event::timer_t *timer, void *ctx,
                           pds_mapping_type_t type)
{
    // note: we have one event timer per MAC and IP entry that expires
    // independently, instead of a global timer as the scale of local endpoints
    // and IPs is low and libev timers scale well

    if (type == PDS_MAPPING_TYPE_L2) {
        event::timer_init(timer, learn_ep_mac_aging_cb,
                          (float)learn_db()->ep_timeout(), 0.0);
    } else {
        event::timer_init(timer, learn_ep_ip_aging_cb,
                          (float)learn_db()->ep_timeout(), 0.0);
    }
    timer->ctx = ctx;
}

}    // namespace learn
