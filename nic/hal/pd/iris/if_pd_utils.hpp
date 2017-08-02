#ifndef __HAL_IF_PD_UTILS_HPP__
#define __HAL_IF_PD_UTILS_HPP__

#include <base.h>
#include <pd.hpp>
#include "pd_api.hpp"

namespace hal {
namespace pd {

uint32_t if_get_hw_lif_id(if_t *pi_if);
uint32_t if_get_encap_vlan(if_t *pi_if, l2seg_t *pi_l2seg);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_IF_PD_UTILS_HPP__
