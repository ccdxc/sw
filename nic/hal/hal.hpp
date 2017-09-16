// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_HPP__
#define __HAL_HPP__

#include <string>
#include <base.h>
#include <list.hpp>
#include <thread.hpp>
#include <ht.hpp>
#include <hal_lock.hpp>
#include <hal_cfg.hpp>

namespace hal {

// TODO_CLEANUP - these don't belong here !!
class LIFManager;
extern LIFManager *g_lif_manager;

using utils::thread;
using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

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

class hal_handle {
    friend class hal_cfg_db;

public:
    static hal_handle *factory(void);
    ~hal_handle();

    // add an object to this handle
    // NOTE:
    // once an object is added to the handle, you waive all your rights on that
    // object and handle ... it can't be freed by the app anymore
    // and the only way to free such object (and allocated handle) is by calling
    // hal_handle_free() on that handle and infra will free both the object and
    // handle (when its appropriate)
    hal_ret_t add_obj(void *obj);

    // delete this object
    hal_ret_t del_obj(void *obj, hal_cfg_del_cb_t del_cb);

    // get an object that has the highes version that is <= read-version
    // acquired by this thread
    void *get_obj(void);

    // get any valid object that is non-NULL from this handle (note that there
    // could be a valid entry but obj is NULL for objects that are deleted)
    void *get_any_obj(void);
    void *get_any_obj_safe(void);    // thread safe version of get_any_obj()

private:
    bool init(void);
    hal_handle();

private:
    hal_spinlock_t    slock_;
    // max. number of objects per handle
    static const uint32_t k_max_objs_ = 4;
    // TODO: revisit and see if valid bit is needed !!
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

// TODO: deprecate these APIs eventually !!!
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

