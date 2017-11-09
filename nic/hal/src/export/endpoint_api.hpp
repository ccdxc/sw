/* 
 * ----------------------------------------------------------------------------
 *
 * endpoint_api.hpp
 *
 * Endpoint APIs exported by PI
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __ENDPOINT_API_HPP__
#define __ENDPOINT_API_HPP__

#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/hal/src/session.hpp"

namespace hal {


#define EP_FTE_EVENTS(ENTRY)                                                  \
     ENTRY(EP_FTE_EVENT_NONE,                   0, "ep_fte_event_none")       \
     ENTRY(EP_FTE_EVENT_IF_CHANGE,              1, "ep_fte_event_if_change")  \
     ENTRY(EP_FTE_EVENT_CONN_TRACK_CHANGE,      2, "ep_fte_event_if_change") 

 DEFINE_ENUM(ep_fte_event_type_t, EP_FTE_EVENTS)
 #undef EP_FTE_EVENTS

typedef struct ep_fte_event_s {
    ep_fte_event_type_t   type;

    union {
        // EP_FTE_EVENT_IF_CHANGE
        struct {
            hal_handle_t        old_if_handle;      // needed ?
            hal_handle_t        new_if_handle;
        } __PACK__;
    } __PACK__;
} __PACK__ ep_fte_event_t;


// EP APIs
l2seg_id_t ep_get_l2segid(ep_t *pi_ep);
mac_addr_t *ep_get_mac_addr(ep_t *pi_ep);
void ep_set_pd_ep(ep_t *pi_ep, pd::pd_ep_t *pd_ep);
if_t *ep_find_if_by_handle(ep_t *pi_ep);
void *ep_get_pd_ep(ep_t *pi_ep);
network_t *ep_get_nw(ep_t *pi_ep, l2seg_t *l2seg);
mac_addr_t *ep_get_rmac(ep_t *pi_ep, l2seg_t *l2seg);
if_t *ep_get_if(ep_t *pi_ep);
vrf_t *ep_get_vrf(ep_t *pi_ep);

// Adding sessions to EPs
hal_ret_t ep_add_session (ep_t *ep, session_t *session);
hal_ret_t ep_del_session (ep_t *ep, session_t *session);

} // namespace hal
#endif // __ENDPOINT_API_HPP__
