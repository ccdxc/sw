// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_HPP__
#define __HAL_HPP__

#include <string>
#include <base.h>
#include <thread.hpp>
#include <ht.hpp>
#include <hal_lock.hpp>

namespace hal {

// TODO_CLEANUP - these don't belong here !!
class LIFManager;
extern LIFManager *g_lif_manager;

using utils::thread;
using hal::utils::ht_ctxt_t;

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
thread *hal_get_current_thread(void);

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

class hal_handle {
public:
    static hal_handle *factory(void);
    ~hal_handle();
    // add an object to this handle .. even if object
    // is getting deleted add it with right version and NULL object
    hal_ret_t add_obj(void *obj);

    // get an object that has the highes version that is <= read-version
    // acquired by this thread
    void *get_obj(void);

    // get any valid object that is non-NULL from this handle (note that there
    // could be a valid entry but obj is NULL for objects that are deleted)
    void *get_any_obj(void);

private:
    bool init(void);
    hal_handle();

private:
    hal_spinlock_t    slock_;
    // max. number of objects per handle
    static const uint32_t k_max_objs_ = 4;
    struct {
        cfg_version_t    ver;        // version of this object
        void             *obj;       // object itself
        uint8_t          valid:1;    // TRUE if valid
    } __PACK__ objs_[k_max_objs_];
};
extern hal_handle_t hal_handle_alloc(void);
extern void hal_handle_free(uint64_t handle);

//------------------------------------------------------------------------------
// A HAL object can be indexed by several keys (e.g., like L2 key, L3 keys for
// endpoint), however all those lookups must give corresponding object's HAL
// handle as result (and never the object itself directly). The following
// structure is meant to be result of all such hash tables
//------------------------------------------------------------------------------
typedef struct hal_handle_ht_entry_s {
    hal_handle    *handle;
    ht_ctxt_t     ht_ctxt;
} __PACK__ hal_handle_ht_entry_t;

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

