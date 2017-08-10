#ifndef __HAL_IF_PD_UTILS_HPP__
#define __HAL_IF_PD_UTILS_HPP__

#include <base.h>
#include <pd.hpp>
#include "pd_api.hpp"

namespace hal {
namespace pd {

// Interface APIs
uint32_t if_get_hw_lif_id(if_t *pi_if);
uint32_t if_get_encap_vlan(if_t *pi_if, l2seg_t *pi_l2seg);
uint32_t if_get_uplink_ifpc_id(if_t *pi_if);
hal_ret_t if_get_qid_qoff(if_t *pi_if, intf::LifQType qtype, 
                          uint8_t *q_off, uint32_t *qid);
tenant_t *if_get_pi_tenant(if_t *pi_if);

// L2Seg APIs
tenant_t *l2seg_get_pi_tenant(l2seg_t *pi_l2seg);

// Tenant APIs
uint32_t ten_get_nwsec_prof_hw_id(tenant_t *pi_ten);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_IF_PD_UTILS_HPP__
