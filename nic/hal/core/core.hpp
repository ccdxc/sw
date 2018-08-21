//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_CORE_HPP__
#define __HAL_CORE_HPP__

#include <signal.h>
#include "sdk/thread.hpp"
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
hal_ret_t hal_thread_init(hal_cfg_t *hal_cfg);
void hal_thread_start(uint32_t thread_id, void *ctxt);
sdk::lib::thread *hal_thread_get(uint32_t thread_id);
sdk::lib::thread *hal_thread_create(const char *name, uint32_t thread_id,
                                    sdk::lib::thread_role_t thread_role,
                                    uint64_t cores_mask,
                                    sdk::lib::thread_entry_func_t entry_func,
                                    uint32_t prio, int sched_policy,
                                    void *data);
sdk::lib::thread *hal_get_current_thread(void);
sdk::lib::thread *hal_get_thread(uint32_t thread_id);
hal_ret_t hal_wait(void);
hal_ret_t hal_thread_destroy(void);

//------------------------------------------------------------------------------
// system or infra timers
// reserve first 16 timer ids for the system
// TODO: move HAL_TIMER_ID_CLOCK_SYNC into infra range once asicpd is sorted out
//------------------------------------------------------------------------------
enum {
    HAL_TIMER_ID_NONE                     = 0,
    HAL_TIMER_ID_MIN                      = 1,
    HAL_TIMER_ID_HEARTBEAT                = HAL_TIMER_ID_MIN,
    HAL_TIMER_ID_STATS,
    HAL_TIMER_ID_INFRA_MAX                = 16,
};

}    // namespace hal

#endif    // __HAL_CORE_HPP__

