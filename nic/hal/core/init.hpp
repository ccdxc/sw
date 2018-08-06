//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_CORE_HPP__
#define __HAL_CORE_HPP__

#include <signal.h>
#include "nic/include/base.hpp"
#include "nic/include/hal_cfg.hpp"

namespace hal {

hal_ret_t hal_parse_cfg(const char *cfgfile, hal_cfg_t *hal_cfg);
hal_ret_t hal_sdk_init(void);
hal_ret_t hal_cores_validate(uint64_t sys_core,
                             uint64_t control_core, uint64_t data_core);
hal_ret_t hal_logger_init(hal_cfg_t *hal_cfg);

// signal handler type
typedef void (*hal_sig_handler_t)(int sig, siginfo_t *info, void *ptr);
hal_ret_t hal_sig_init(hal_sig_handler_t sig_handler);

void *periodic_thread_start(void *ctxt);

}    // namespace hal

#endif    // __HAL_CORE_HPP__

