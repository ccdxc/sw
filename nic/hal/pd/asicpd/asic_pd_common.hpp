// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_ASIC_PD_COMMON_HPP__
#define __HAL_PD_ASIC_PD_COMMON_HPP__

#include "nic/include/base.h"
#include <vector>
#include <tuple>
using std::vector;
using std::tuple;

namespace hal {
namespace pd {

int asicpd_table_entry_write(uint32_t tableid, uint32_t index,
                             uint8_t  *hwentry, uint16_t hwentry_bit_len);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_ASIC_RW_HPP__

