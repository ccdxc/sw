#include "nic/hal/src/endpoint.hpp"
#include "nic/include/endpoint_api.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/utils.hpp"

namespace hal {

l2seg_id_t
ep_get_l2segid(ep_t *pi_ep)
{
    return pi_ep->l2_key.l2_segid;
}

mac_addr_t *
ep_get_mac_addr(ep_t *pi_ep)
{
    return &(pi_ep->l2_key.mac_addr);
}

mac_addr_t *
ep_get_rmac(ep_t *pi_ep, l2seg_t *l2seg)
{
    network_t *nw;
    nw = ep_get_nw(pi_ep, l2seg);
    if (!nw) {
        HAL_ASSERT(0);
    }
    return (&nw->rmac_addr);
}

if_t *
ep_get_if(ep_t *pi_ep) 
{
    return (find_if_by_handle(pi_ep->if_handle));
}

tenant_t *
ep_get_tenant(ep_t *pi_ep) 
{
    return (tenant_lookup_by_handle(pi_ep->tenant_handle));
}

network_t *
ep_get_nw(ep_t *pi_ep, l2seg_t *l2seg) 
{
    network_t                   *nw = NULL;
    dllist_ctxt_t               *lnode = NULL, *nw_lnode = NULL;
    ep_ip_entry_t               *pi_ip_entry = NULL;
    hal_handle_id_list_entry_t  *entry = NULL;

    // Get the first IP
    if (dllist_empty(&pi_ep->ip_list_head)) {
        HAL_TRACE_ERR("{}:there are no ips for the ep",
                      __FUNCTION__);
        goto end;
    } else {
        lnode = pi_ep->ip_list_head.next;
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode - offsetof(ep_ip_entry_t, ep_ip_lentry));

#if 0
        if (dllist_empty(&l2seg->nw_list_head)) {
            goto end;
        }
#endif
        dllist_for_each(nw_lnode, &(l2seg->nw_list_head)) {
            entry = dllist_entry(nw_lnode, hal_handle_id_list_entry_t, dllist_ctxt);
            nw = find_network_by_handle(entry->handle_id);
#if 0
            nw = (network_t *)((char *)nw_lnode -
                    offsetof(network_t, l2seg_nw_lentry));
#endif
            // Check if ip is in prefix
            if (ip_addr_in_ip_pfx(&pi_ip_entry->ip_addr, &nw->nw_key.ip_pfx)) {
                return nw;
            }
        }
    }

end:
    return nw;
}

// ----------------------------------------------------------------------------
// EP API: Set PD EP in PI EP
// ----------------------------------------------------------------------------
void 
ep_set_pd_ep(ep_t *pi_ep, pd::pd_ep_t *pd_ep)
{
    pi_ep->pd = pd_ep;
}

if_t *
ep_find_if_by_handle(ep_t *pi_ep)
{
    // return (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&(pi_ep->if_handle));
    return find_if_by_handle(pi_ep->if_handle);
}

void *
ep_get_pd_ep(ep_t *pi_ep)
{
    return pi_ep->pd;
}


} // namespace hal


