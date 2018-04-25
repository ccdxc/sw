//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hal_handle.hpp"
#include "nic/include/hal_state.hpp"

namespace hal {

bool g_delay_delete = true;

// process globals
uint64_t  g_hal_handle = 1;

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
hal_handle::hal_handle()
{
}

//------------------------------------------------------------------------------
// initialize hal handle
//------------------------------------------------------------------------------
bool
hal_handle::init(hal_obj_id_t obj_id)
{
    if ((obj_id <= HAL_OBJ_ID_NONE) || (obj_id >= HAL_OBJ_ID_MAX)) {
        return false;
    }
    handle_id_ = HAL_HANDLE_INVALID;
    obj_id_ = obj_id;
    obj_ = NULL;

    return true;
}

//------------------------------------------------------------------------------
// factory method to allocate a handle
//------------------------------------------------------------------------------
hal_handle *
hal_handle::factory(hal_obj_id_t obj_id)
{
    void          *mem;
    hal_handle    *handle;

    // allocate from the handle slab
    mem = hal_handle_slab()->alloc();
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    handle = new(mem) hal_handle();
    // initialize the handle instance
    if (handle->init(obj_id) == false) {
        handle->~hal_handle();
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE, handle);
        return NULL;
    }

    return handle;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_handle::~hal_handle()
{
}

//------------------------------------------------------------------------------
// private helper function that calls operation specific callback and then calls
// either commit or abort callback based on whether operation succeeded or
// failed
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::act_on_obj_(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t cb,
                        hal_cfg_commit_cb_t commit_cb,
                        hal_cfg_abort_cb_t abort_cb)
{
    hal_ret_t    ret;

    // invoke modify callback
    ret = cb(ctxt);
    if (ret == HAL_RET_OK) {
        // call commit callback if successful
        commit_cb(ctxt);
    } else {
        // call abort callback in case of failure
        abort_cb(ctxt);
    }

    return ret;
}

//------------------------------------------------------------------------------
// add an object to this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::add_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t add_cb,
                    hal_cfg_commit_cb_t commit_cb, hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_ret_t      ret;
    dhl_entry_t    *entry;

    // release the read lock
    hal_handle_cfg_db_lock(true, false);

    // acquire the write lock
    hal_handle_cfg_db_lock(false, true);

    // invoke the callbacks passed
    ret = act_on_obj_(ctxt, add_cb, commit_cb, abort_cb);
    if (ret == HAL_RET_OK) {
        // if commit was successful, populate the handle (HACK ??)
        entry = dllist_entry(ctxt->dhl.next,
                             dhl_entry_t, dllist_ctxt);
        this->obj_ = entry->obj;
    }

    // call cleanup callback to either free the software state of original
    // object of cloned object (based on whether modify was successful or not)
    cleanup_cb(ctxt);

    // release the write lock
    hal_handle_cfg_db_lock(false, false);

    // reacquire the read lock again
    hal_handle_cfg_db_lock(true, true);

    return ret;
}

//------------------------------------------------------------------------------
// helper function to overwrite current instance of the object with cloned obj,
// cloned obj is supposed to have all updated changes by now
//------------------------------------------------------------------------------
void
hal_handle::replace_(void *obj, void *cloned_obj)
{
    if (cloned_obj) {
        this->obj_ = cloned_obj;
    }
}

//------------------------------------------------------------------------------
// modify an object in this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::upd_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t upd_cb,
                    hal_cfg_commit_cb_t commit_cb, hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_ret_t        ret;
    dllist_ctxt_t    *curr, *next;
    dhl_entry_t      *entry;

    // release the read lock
    hal_handle_cfg_db_lock(true, false);

    // acquire the write lock
    hal_handle_cfg_db_lock(false, true);

    // invoke the callbacks passed
    ret = act_on_obj_(ctxt, upd_cb, commit_cb, abort_cb);

    // if commit was successful, copy over old object with cloned object's
    // contents
    if (ret == HAL_RET_OK) {
        dllist_for_each_safe(curr, next, &ctxt->dhl) {
            entry = dllist_entry(curr, dhl_entry_t, dllist_ctxt);
            this->replace_(entry->obj, entry->cloned_obj);
        }
    }

    // call cleanup callback to either free the software state of original
    // object of cloned object (based on whether modify was successful or not)
    cleanup_cb(ctxt);

    // release the write lock
    hal_handle_cfg_db_lock(false, false);

    // reacquire the read lock again
    hal_handle_cfg_db_lock(true, true);

    return ret;
}

//------------------------------------------------------------------------------
// delete object from this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::del_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t del_cb,
                    hal_cfg_commit_cb_t commit_cb, hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_ret_t      ret = HAL_RET_OK;

    // release the read lock
    hal_handle_cfg_db_lock(true, false);

    // acquire the write lock
    hal_handle_cfg_db_lock(false, true);

    // invoke the callbacks passed
    ret = act_on_obj_(ctxt, del_cb, commit_cb, abort_cb);

    // call cleanup callback to either free the software state of original
    // object of cloned object (based on whether modify was successful or not)
    cleanup_cb(ctxt);

    // release the write lock
    hal_handle_cfg_db_lock(false, false);

    // reacquire the read lock again
    hal_handle_cfg_db_lock(true, true);

    return ret;
}

//------------------------------------------------------------------------------
// allocate a handle for an object instance, if handle is provided use it or
// else allocate one
//------------------------------------------------------------------------------
hal_handle_t
hal_handle_alloc (hal_obj_id_t obj_id, hal_handle_t handle_id,
                  hal_handle **handle_ret)
{
    sdk_ret_t                sdk_ret;
    hal_handle               *handle;
    hal_handle_ht_entry_t    *entry;

    // allocate an entry to create mapping from handle-id to its handle obj
    entry =
        (hal_handle_ht_entry_t *)hal_handle_ht_entry_slab()->alloc();
    if (entry == NULL) {
        HAL_TRACE_ERR("Failed to allocate hal handle ht entry, obj id {}",
                      obj_id);
        return HAL_HANDLE_INVALID;
    }

    // allocate hal handle object itself
    handle = hal_handle::factory(obj_id);
    if (handle == NULL) {
        HAL_TRACE_ERR("Failed to allocate handle, obj id {}", obj_id);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_HT_ENTRY, entry);
        return HAL_HANDLE_INVALID;
    }
    if (handle_ret) {
        *handle_ret = handle;
    }
    // allocate unique handle id, if not provided
    if (handle_id == HAL_HANDLE_INVALID) {
        handle_id = HAL_ATOMIC_INC_UINT32(&g_hal_handle, 1);
    }
    handle->set_handle_id(handle_id);

    // prepare the entry to be inserted
    entry->handle = handle;
    entry->ht_ctxt.reset();
    sdk_ret = hal_handle_id_ht()->insert_with_key(&handle_id,
                                                  entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add handle id {} to handle db, obj id {}",
                      handle_id, obj_id);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_HT_ENTRY, entry);
        handle->~hal_handle();
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE, handle);
        return HAL_HANDLE_INVALID;
    }

    HAL_TRACE_DEBUG("Assigned hal_handle {} for obj id {}", handle_id, obj_id);
    return handle_id;
}

//------------------------------------------------------------------------------
// allocates the handle object instance, populates the obj in that instance
// and insert into hash-table
//------------------------------------------------------------------------------
hal_ret_t
hal_handle_insert (hal_obj_id_t obj_id, hal_handle_t handle_id, void *obj)
{
    hal_ret_t   ret = HAL_RET_OK;
    hal_handle  *handle = NULL;

    hal_handle_alloc(obj_id, handle_id, &handle);
    handle->set_obj(obj);

    return ret;
}

//------------------------------------------------------------------------------
// return a hal handle back so it can be reallocated for another object
//------------------------------------------------------------------------------
void
hal_handle_free (hal_handle_t handle_id)
{
    hal_handle_ht_entry_t    *entry;

    entry = (hal_handle_ht_entry_t *)
                hal_handle_id_ht()->remove(&handle_id);
    if (entry == NULL) {
        return;
    }
    if (entry->handle) {
        entry->handle->~hal_handle();
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE, entry->handle);
    }
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_HT_ENTRY, entry);

    return;
}

//------------------------------------------------------------------------------
// given a handle id, return the corresponding handle object
//------------------------------------------------------------------------------
hal_handle *
hal_handle_get_from_handle_id (hal_handle_t handle_id)
{
    hal_handle_ht_entry_t    *entry;

    if (handle_id == HAL_HANDLE_INVALID) {
        return NULL;
    }
    // lookup by handle id and get the handle class
    entry =
        (hal_handle_ht_entry_t *)hal_handle_id_ht()->lookup(&handle_id);
    if (entry) {
        return entry->handle;
    }
    HAL_TRACE_ERR("Failed to get handle obj from handle id {}", handle_id);
    return NULL;
}

//------------------------------------------------------------------------------
// given a handle id, return the corresponding config object in the db
//------------------------------------------------------------------------------
void *
hal_handle_get_obj (hal_handle_t handle_id)
{
    hal_handle_ht_entry_t    *entry;

    if (handle_id == HAL_HANDLE_INVALID) {
        return NULL;
    }
    // lookup by handle id and get the handle class
    entry =
        (hal_handle_ht_entry_t *)hal_handle_id_ht()->lookup(&handle_id);
    if (entry && entry->handle) {
        return entry->handle->obj();
    }
    return NULL;
}

//------------------------------------------------------------------------------
// given a handle id, add the object to the cfg db with this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle_add_obj (hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                    hal_cfg_op_cb_t add_cb,
                    hal_cfg_commit_cb_t commit_cb,
                    hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_handle    *handle;

    handle = hal_handle_get_from_handle_id(handle_id);
    return handle->add_obj(ctxt, add_cb, commit_cb, abort_cb, cleanup_cb);
}

//------------------------------------------------------------------------------
// given a handle id, delete the object to the cfg db with this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle_del_obj (hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                    hal_cfg_op_cb_t del_cb,
                    hal_cfg_commit_cb_t commit_cb,
                    hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_handle    *handle;

    handle = hal_handle_get_from_handle_id(handle_id);
    return handle->del_obj(ctxt, del_cb, commit_cb, abort_cb, cleanup_cb);
}

//------------------------------------------------------------------------------
// given a handle id, update the corresponding config object in the db
//------------------------------------------------------------------------------
hal_ret_t
hal_handle_upd_obj (hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                    hal_cfg_op_cb_t upd_cb,
                    hal_cfg_commit_cb_t commit_cb,
                    hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_handle    *handle;

    handle = hal_handle_get_from_handle_id(handle_id);
    return handle->upd_obj(ctxt, upd_cb, commit_cb, abort_cb, cleanup_cb);
}

// TODO: cleanup these two APIs once all objects move to new APIs
//------------------------------------------------------------------------------
// allocate a handle for an object instance
// TODO: if this can be called from FTE, we need atomic increments
//------------------------------------------------------------------------------
hal_handle_t
hal_alloc_handle (void)
{
    return g_hal_handle++;
}

//------------------------------------------------------------------------------
// return a hal handle back so it can be reallocated for another object
//------------------------------------------------------------------------------
void
hal_free_handle (uint64_t handle)
{
    return;
}

}    // namespace hal
