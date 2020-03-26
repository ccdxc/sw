// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ASIC_INIT_HPP__
#define __ASIC_INIT_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "asic/asic.hpp"

namespace sdk  {
namespace asic {

sdk_ret_t asic_pgm_init(asic_cfg_t *cfg);
sdk_ret_t asic_asm_init (asic_cfg_t *cfg);
sdk_ret_t asic_hbm_regions_init (asic_cfg_t *cfg);

}    // namespace asic
}    // namespace sdk

#endif    // __ASIC_INIT_HPP__
