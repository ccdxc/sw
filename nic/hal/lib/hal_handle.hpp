// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_LIB_HAL_HANDLE_HPP__
#define __HAL_LIB_HAL_HANDLE_HPP__

#include <string>

#include "nic/include/base.h"
#include "nic/include/hal_cfg.hpp"

#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "sdk/slab.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;
using sdk::lib::slab;
using sdk::lib::ht;

namespace hal {

class hal_handle {
public:
    hal_handle_t    handle_id_;     // TODO: make this private

public:
    static hal_handle *factory(hal_obj_id_t obj_id);
    ~hal_handle();
    hal_obj_id_t obj_id(void) const { return obj_id_; }
    void set_handle_id(hal_handle_t handle) { handle_id_ = handle; }

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
    hal_obj_id_t    obj_id_;
    void            *obj_;
};

#define HAL_MAX_HANDLES                              8192
extern hal_handle_t hal_handle_alloc(hal_obj_id_t obj_id,
                                     hal_handle_t handle_id = HAL_HANDLE_INVALID);
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
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
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
extern hal_handle *hal_handle_get_from_handle_id(hal_handle_t handle_id);
slab *hal_handle_slab(void);
slab *hal_handle_ht_entry_slab(void);
ht *hal_handle_id_ht(void);
void hal_handle_cfg_db_lock(bool readlock, bool lock);

}   /* namespace hal */

#endif /* __HAL_LIB_HAL_HANDLE_HPP__ */
