// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_IF_PD_UTILS_HPP__
#define __HAL_IF_PD_UTILS_HPP__

#include "nic/include/base.h"
#include "nic/hal/src/interface.hpp"

namespace hal {
namespace pd {

// Interface APIs
hal_ret_t if_get_hw_lif_id(if_t *pi_if, uint32_t *hw_lif_id);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_IF_PD_UTILS_HPP__
