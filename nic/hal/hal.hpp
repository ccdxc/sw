// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_HPP__
#define __HAL_HPP__

#include <string>
#include "nic/include/base.h"
#include "sdk/thread.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/lib/hal_handle.hpp"

namespace hal {

// TODO_CLEANUP - these don't belong here !!
class LIFManager;
extern LIFManager *g_lif_manager;

using sdk::lib::thread;

#define HAL_MAX_CORES                                4
#define HAL_CONTROL_CORE_ID                          0

extern thread *g_hal_threads[HAL_THREAD_ID_MAX];
thread *hal_get_current_thread(void);

//------------------------------------------------------------------------------
// parse HAL configuration
//------------------------------------------------------------------------------
extern hal_ret_t hal_parse_cfg(const char *cfgfile __IN__,
                               hal_cfg_t *hal_cfg __OUT__);

//------------------------------------------------------------------------------
// parse HAL initialization
//------------------------------------------------------------------------------
extern hal_ret_t hal_parse_ini(const char *inifile __IN__,
                               hal_cfg_t *hal_cfg __OUT__);

//------------------------------------------------------------------------------
// hal initialization main routine
//------------------------------------------------------------------------------
extern hal_ret_t hal_init(hal_cfg_t *cfg __IN__);

//------------------------------------------------------------------------------
// hal uninitialization main routine
//------------------------------------------------------------------------------
extern hal_ret_t hal_destroy(void);

//------------------------------------------------------------------------------
// hal wait-for-cleanup API
//------------------------------------------------------------------------------
extern hal_ret_t hal_wait(void);

//------------------------------------------------------------------------------
// one time memory related initializatino for HAL
//------------------------------------------------------------------------------
extern hal_ret_t hal_mem_init(void);

//------------------------------------------------------------------------------
// one time config related initializatino for HAL
//------------------------------------------------------------------------------
extern hal_ret_t hal_cfg_init(hal_cfg_t *hal_cfg);

}    // namespace hal

#endif    // __HAL_HPP__

