//
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
    THREAD_ID_CONTROL    = 1,
    THREAD_ID_PERIODIC   = 2,
    THREAD_ID_NICMGR     = 3,
    THREAD_ID_DATA_START = 16,
    THREAD_ID_DATA_END   = 47,
    THREAD_ID_MAX        = 38
};

enum {
    TIMER_ID_NONE         = 0,
    TIMER_ID_SESSION_AGE  = 1,
    TIMER_ID_SYSTEM_SCAN  = 2,
};

sdk_ret_t thread_periodic_spawn(pds_state *state);
sdk_ret_t thread_nicmgr_spawn(pds_state *state);
void threads_stop();
sdk::lib::thread *thread_get(uint32_t thread_id);
sdk_ret_t parse_global_config(string pipeline, string cfg_file,
                              pds_state *state);
sdk_ret_t parse_pipeline_config(string pipeline, pds_state *state);

typedef void (*sig_handler_t)(int sig, siginfo_t *info, void *ptr);
sdk_ret_t sig_init(int signal, sig_handler_t sig_handler);
sdk_ret_t schedule_timers(pds_state *state, sdk::lib::twheel_cb_t sysmon_cb);

}    // namespace core

#endif    // __CORE_CORE_HPP__
