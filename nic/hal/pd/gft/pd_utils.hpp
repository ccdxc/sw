// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_IF_PD_UTILS_HPP__
#define __HAL_IF_PD_UTILS_HPP__

#include "nic/include/base.h"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/gft.hpp"

namespace hal {
namespace pd {

// Interface APIs
hal_ret_t if_get_hw_lif_id(if_t *pi_if, uint32_t *hw_lif_id);

// GFT APIs
bool gft_match_prof_is_ingress(gft_table_type_t type);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_IF_PD_UTILS_HPP__
