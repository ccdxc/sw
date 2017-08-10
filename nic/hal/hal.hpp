#ifndef __HAL_HPP__
#define __HAL_HPP__

#include <string>
#include <base.h>
#include <thread.hpp>

namespace hal {

class LIFManager;
extern LIFManager *g_lif_manager;

using utils::thread;

//------------------------------------------------------------------------------
// TODO - following should come from cfg file or should be derived from platform
//        type/cfg
//------------------------------------------------------------------------------
enum {
    HAL_THREAD_ID_CFG        = 0,
    HAL_THREAD_ID_PERIODIC   = 1,
    HAL_THREAD_ID_FTE_MIN    = 2,
    HAL_THREAD_ID_FTE_MAX    = 4,
    HAL_THREAD_ID_ASIC_RW    = 5,
    HAL_THREAD_ID_MAX        = 6,
};

#define HAL_MAX_CORES                                4
#define HAL_CONTROL_CORE_ID                          0

extern thread *g_hal_threads[HAL_THREAD_ID_MAX];

#define HAL_MAX_NAME_STR         16
typedef struct hal_cfg_s {
    bool             sim;
    char             asic_name[HAL_MAX_NAME_STR];
    std::string      grpc_port;
    char             feature_set[HAL_MAX_NAME_STR];
} hal_cfg_t;

//------------------------------------------------------------------------------
// parse HAL configuration
//------------------------------------------------------------------------------
extern hal_ret_t hal_parse_cfg(const char *cfgfile __IN__,
                               hal_cfg_t *hal_cfg __OUT__);

//------------------------------------------------------------------------------
// hal initialization main routine
//------------------------------------------------------------------------------
extern hal_ret_t hal_init(hal_cfg_t *cfg __IN__);

//------------------------------------------------------------------------------
// hal wait-for-cleanup API
//------------------------------------------------------------------------------
extern hal_ret_t hal_wait(void);

//------------------------------------------------------------------------------
// one time memory related initializatino for HAL
//------------------------------------------------------------------------------
extern hal_ret_t hal_mem_init(void);

extern uint64_t hal_handle;
//------------------------------------------------------------------------------
// HAL internal api to allocate handle for an object
//------------------------------------------------------------------------------
extern hal_handle_t hal_alloc_handle(void);

//------------------------------------------------------------------------------
// HAL internal api to free handle of an object back
//------------------------------------------------------------------------------
extern void hal_free_handle(uint64_t handle);

}    // namespace hal

#endif    // __HAL_HPP__

