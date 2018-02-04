// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_HPP__
#define __HAL_PD_HPP__

#include "nic/include/hal.hpp"
#include "nic/hal/pd/hal_pd_error.hpp"

namespace hal {
namespace pd {

typedef struct p4pd_def_cfg_s {
    uint32_t admin_cos;
} __PACK__ p4pd_def_cfg_t;

extern hal_ret_t hal_pd_init(hal::hal_cfg_t *hal_cfg);
extern hal_ret_t p4pd_table_defaults_init(p4pd_def_cfg_t *p4pd_def_cfg);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_HPP__

