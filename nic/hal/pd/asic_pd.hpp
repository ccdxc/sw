// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_ASIC_RW_HPP__
#define __HAL_PD_ASIC_RW_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/asic/asic.hpp"

namespace hal {
namespace pd {

// asic init
typedef struct pd_asic_init_args_s {
    asic_cfg_t   *cfg;
} __PACK__ pd_asic_init_args_t;

//------------------------------------------------------------------------------
// public API for saving cpu packet.
//------------------------------------------------------------------------------
sdk_ret_t asic_step_cpu_pkt(const uint8_t* pkt, size_t pkt_len);

// check if the current thread is hal-control thread or not
bool is_hal_ctrl_thread(void);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_ASIC_RW_HPP__

