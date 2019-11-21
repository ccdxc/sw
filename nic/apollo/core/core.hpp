// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains core helper functions
///
//----------------------------------------------------------------------------

#ifndef __CORE_CORE_HPP__
#define __CORE_CORE_HPP__

#include <signal.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/lib/periodic/periodic.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace core {

enum {
    THREAD_ID_NONE       = 0,
    THREAD_ID_CFG        = 1,
    THREAD_ID_API        = 2,
    THREAD_ID_PERIODIC   = 3,
    THREAD_ID_NICMGR     = 4,
    THREAD_ID_PCIEMGR    = 5,
    THREAD_ID_LEARN      = 6,
    THREAD_ID_MAX        = 48
};

enum {
    TIMER_ID_NONE         = 0,
    TIMER_ID_SESSION_AGE  = 1,
    TIMER_ID_SYSTEM_SCAN  = 2,
};

sdk_ret_t spawn_periodic_thread(pds_state *state);
sdk_ret_t spawn_nicmgr_thread(pds_state *state);
sdk_ret_t spawn_pciemgr_thread(pds_state *state);
sdk_ret_t spawn_api_thread(pds_state *state);
sdk_ret_t spawn_learn_thread(pds_state *state);
bool is_nicmgr_ready(void);
void threads_stop(void);
sdk::lib::thread *thread_get(uint32_t thread_id);
sdk_ret_t parse_global_config(string pipeline, string cfg_file,
                              pds_state *state);
sdk_ret_t parse_pipeline_config(string pipeline, pds_state *state);

typedef void (*sig_handler_t)(int sig, siginfo_t *info, void *ptr);
sdk_ret_t sig_init(int signal, sig_handler_t sig_handler);
sdk_ret_t schedule_timers(pds_state *state, sdk::lib::twheel_cb_t sysmon_cb);

}    // namespace core

#endif    // __CORE_CORE_HPP__
