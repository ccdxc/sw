#ifndef __HAL_IF_PD_UTILS_HPP__
#define __HAL_IF_PD_UTILS_HPP__

#include <base.h>
#include <pd.hpp>
#include "pd_api.hpp"

namespace hal {
namespace pd {

// Interface APIs
uint32_t if_get_lport_id(if_t *pi_if);
uint32_t if_get_hw_lif_id(if_t *pi_if);
uint32_t if_get_uplink_ifpc_id(if_t *pi_if);
uint32_t if_get_uplink_lport_id(if_t *pi_if);
tenant_t *if_get_pi_tenant(if_t *pi_if);
ep_t *if_get_tunnelif_remote_tep_ep(if_t *pi_if, bool *v4_valid);
if_t *if_get_if_from_ep(ep_t *ep);
int pd_tunnelif_get_rw_idx(pd_tunnelif_t *pd_tif);

// Lif APIs
uint32_t lif_get_lport_id(lif_t *pi_lif);

// L2Seg APIs
tenant_t *l2seg_get_pi_tenant(l2seg_t *pi_l2seg);

// Tenant APIs
uint32_t ten_get_nwsec_prof_hw_id(tenant_t *pi_ten);

// Utils
uint8_t *memrev(uint8_t *block, size_t elnum);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_IF_PD_UTILS_HPP__
