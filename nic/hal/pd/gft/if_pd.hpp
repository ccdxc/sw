// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_IF_HPP__
#define __HAL_PD_IF_HPP__


#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "gen/p4gen/gft/include/p4pd.h"

namespace hal {
namespace pd {

struct pd_if_s {
} __PACK__;

// Utils
uint8_t *memrev(uint8_t *block, size_t elnum);
}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_IF_HPP__
