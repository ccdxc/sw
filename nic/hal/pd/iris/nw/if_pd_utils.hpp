#ifndef __HAL_IF_PD_UTILS_HPP__
#define __HAL_IF_PD_UTILS_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
namespace pd {

#define OPER_TYPES(ENTRY)                         \
    ENTRY(TABLE_OPER_INSERT,    0, "INSERT")      \
    ENTRY(TABLE_OPER_UPDATE,    1, "UPDATE")      \
    ENTRY(TABLE_OPER_REMOVE,    2, "REMOVE")

DEFINE_ENUM(table_oper_t, OPER_TYPES)
#undef OPER_TYPES

// Interface APIs
bool if_is_cpu_if(if_t *pi_if);
bool if_is_tunnel_if(if_t *pi_if);
uint32_t if_get_lport_id(if_t *pi_if);
uint32_t if_get_hw_lif_id(if_t *pi_if);
uint32_t if_get_uplink_ifpc_id(if_t *pi_if);
uint32_t if_get_uplink_lport_id(if_t *pi_if);
vrf_t *if_get_pi_vrf(if_t *pi_if);
if_t *if_get_if_from_ep(ep_t *ep);
int tunnelif_get_rw_idx(pd_tunnelif_t *pd_tif);
uint32_t pd_uplinkif_pdif_label(pd_uplinkif_t *upif);
uint32_t pd_uplinkif_if_label(if_t *hal_if);

// Lif APIs
hal_ret_t pd_lif_get_hw_lif_id(lif_t *lif, uint32_t *hw_lif_id);

// L2Seg APIs
vrf_t *l2seg_get_pi_vrf(l2seg_t *pi_l2seg);

// Vrf APIs
uint32_t ten_get_nwsec_prof_hw_id(vrf_t *pi_ten);
hal_ret_t pd_get_l2seg_ten_masks(uint16_t *l2seg_mask, uint16_t *ten_mask,
                                 uint8_t *ten_shift);

// Nwsec APIs
uint32_t nwsec_get_nwsec_prof_hw_id(nwsec_profile_t *pi_nwsec);

// Utils
uint8_t *memrev(uint8_t *block, size_t elnum);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_IF_PD_UTILS_HPP__
