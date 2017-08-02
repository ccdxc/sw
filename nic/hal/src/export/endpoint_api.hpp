/* 
 * ----------------------------------------------------------------------------
 *
 * interface_api.hpp
 *
 * Interface APIs exported by PI to PD.
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __ENDPOINT_API_HPP__
#define __ENDPOINT_API_HPP__

#include <l2segment.hpp>

namespace hal {

// EP APIs

l2seg_id_t ep_get_l2segid(ep_t *pi_ep);

mac_addr_t *ep_get_mac_addr(ep_t *pi_ep);

void ep_set_pd_ep(ep_t *pi_ep, pd::pd_ep_t *pd_ep);
if_t *ep_find_if_by_handle(ep_t *pi_ep);
void *ep_get_pd_ep(ep_t *pi_ep);

} // namespace hal
#endif // __ENDPOINT_API_HPP__
