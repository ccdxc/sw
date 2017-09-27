// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_HPP__
#define __HAL_HPP__

#include <string>
#include "nic/include/base.h"
#include "nic/utils/list/list.hpp"
#include "nic/utils/thread/thread.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_cfg.hpp"

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
    //friend class hal_cfg_db;

public:
    hal_handle_t    handle_id_;     // TODO: make this private

public:
    static hal_handle *factory(hal_obj_id_t obj_id);
    ~hal_handle();
    hal_obj_id_t obj_id(void) const { return obj_id_; }
    void set_handle_id(hal_handle_t handle) { handle_id_ = handle; }
    //hal_handle_t handle_id(void) const { return handle_id_; }

#if 0
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
#endif

    // start adding object(s) to cfg db and drive h/w programming, if any
    // NOTE: add can block the caller until all the readers of the cfg database
    // finish db_close() operation on the cfg db and once the all readers
    // release the db, add operation starts blocking the readers (aka. FTEs)
    hal_ret_t add_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t add_cb,
                       hal_cfg_commit_cb_t commit_cb,
                       hal_cfg_abort_cb_t abort_cb,
                       hal_cfg_cleanup_cb_t cleanup_cb);

    // start modifying object(s) in cfg db and drive h/w programming, if any
    // NOTE: upd can block the caller until all the readers of the cfg database
    // finish db_close() operation on the cfg db and once the all readers
    // release the db, add operation starts blocking the readers (aka. FTEs)
    hal_ret_t upd_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t upd_cb,
                      hal_cfg_commit_cb_t commit_cb,
                      hal_cfg_abort_cb_t abort_cb,
                      hal_cfg_cleanup_cb_t cleanup_cb);

    // start deletion of object(s) from the cfg db and drive h/w programming, if
    // any
    // NOTE: del can block the caller until all the readers of the cfg database
    // finish db_close() operation on the cfg db and once the all readers
    // release the db, add operation starts blocking the readers (aka. FTEs)
    hal_ret_t del_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t del_cb,
                      hal_cfg_commit_cb_t commit_cb,
                      hal_cfg_abort_cb_t abort_cb,
                      hal_cfg_cleanup_cb_t cleanup_cb);

    void *get_obj(void) const { return obj_; }

private:
    hal_handle();
    bool init(hal_obj_id_t obj_id);
    hal_ret_t act_on_obj_(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t cb,
                          hal_cfg_commit_cb_t commit_cb,
                          hal_cfg_abort_cb_t abort_cb);
    void replace_(void *obj, void *cloned_obj);

private:
    //hal_handle_t    handle_id_;
    hal_obj_id_t    obj_id_;
    void            *obj_;
#if 0
    hal_spinlock_t    slock_;
    // max. number of objects per handle
    static const uint32_t k_max_objs_ = 4;
    // TODO: revisit and see if valid bit is needed !!
    struct {
        cfg_version_t    ver;        // version of this object
        void             *obj;       // object itself
        uint8_t          valid:1;    // TRUE if valid
    } __PACK__ objs_[k_max_objs_];
#endif
};
#define HAL_MAX_HANDLES                              8192
extern hal_handle_t hal_handle_alloc(void);
extern hal_handle_t hal_handle_alloc(hal_obj_id_t obj_id);
extern void hal_handle_free(hal_handle_t handle_id);
extern hal_ret_t hal_handle_add_obj(hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                                    hal_cfg_op_cb_t add_cb,
                                    hal_cfg_commit_cb_t commit_cb,
                                    hal_cfg_abort_cb_t abort_cb,
                                    hal_cfg_cleanup_cb_t cleanup_cb);
extern hal_ret_t hal_handle_del_obj(hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                                    hal_cfg_op_cb_t del_cb,
                                    hal_cfg_commit_cb_t commit_cb,
                                    hal_cfg_abort_cb_t abort_cb,
                                    hal_cfg_cleanup_cb_t cleanup_cb);
extern hal_ret_t hal_handle_upd_obj(hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                                    hal_cfg_op_cb_t del_cb,
                                    hal_cfg_commit_cb_t commit_cb,
                                    hal_cfg_abort_cb_t abort_cb,
                                    hal_cfg_cleanup_cb_t cleanup_cb);
extern void *hal_handle_get_obj(hal_handle_t handle_id);

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

static inline void *
hal_handle_id_get_key_func (void *entry)
{
    hal_handle_ht_entry_t    *ht_entry;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_ht_entry_t *)entry;
    return (void *)&(((hal_handle *)ht_entry->handle)->handle_id_);
}

static inline uint32_t
hal_handle_id_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

static inline bool
hal_handle_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

typedef struct hal_handle_list_entry_s {
    hal_handle       *handle;
    dllist_ctxt_t    dllist_ctxt;
} __PACK__ hal_handle_list_entry_t;

typedef struct hal_handle_id_list_entry_s {
    hal_handle_t     handle_id;
    dllist_ctxt_t    dllist_ctxt;
} __PACK__ hal_handle_id_list_entry_t;

typedef struct hal_handle_id_ht_entry_s {
    hal_handle_t     handle_id;
    ht_ctxt_t        ht_ctxt;
} __PACK__ hal_handle_id_ht_entry_t;
// TODO: deprecate these APIs eventually !!!
//------------------------------------------------------------------------------
// HAL internal api to allocate handle for an object
//------------------------------------------------------------------------------
extern hal_handle_t hal_alloc_handle(void);

//------------------------------------------------------------------------------
// HAL internal api to free handle of an object back
//------------------------------------------------------------------------------
extern void hal_free_handle(hal_handle_t handle);

}    // namespace hal

#endif    // __HAL_HPP__

