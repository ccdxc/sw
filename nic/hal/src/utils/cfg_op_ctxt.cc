//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// handlers for cfg_op_ctxt_t
//-----------------------------------------------------------------------------

#include "cfg_op_ctxt.hpp"
#include "nic/hal/lib/hal_handle.hpp"

using sdk::lib::dllist_ctxt_t;

namespace hal {

//-----------------------------------------------------------------------------
// HAL handle routines
//-----------------------------------------------------------------------------

static inline hal_handle_t
cfg_op_ctxt_hal_hdl_alloc (hal_obj_id_t obj_id)
{
    return hal_handle_alloc(obj_id);
}

static inline void
cfg_op_ctxt_hal_hdl_free (hal_handle_t hal_hdl)
{
    hal_handle_free(hal_hdl);
}

static inline hal_handle_t
cfg_op_ctxt_hal_hdl_alloc_init (hal_obj_id_t obj_id)
{
    hal_handle_t hal_hdl;

    if ((hal_hdl = cfg_op_ctxt_hal_hdl_alloc(obj_id)) == HAL_HANDLE_INVALID)
        return hal_hdl;

    return hal_hdl;
}

static inline void
cfg_op_ctxt_hal_hdl_uninit_free (hal_handle_t hal_hdl)
{
    if (hal_hdl != HAL_HANDLE_INVALID) {
        cfg_op_ctxt_hal_hdl_free(hal_hdl);
    }
}

static inline hal_ret_t 
cfg_op_ctxt_hal_hdl_db_add (cfg_op_ctxt_t *cfg_ctxt, hal_handle_t hal_hdl,
                            hal_cfg_op_cb_t add_cb,
                            hal_cfg_commit_cb_t commit_cb,
                            hal_cfg_abort_cb_t abort_cb,
                            hal_cfg_cleanup_cb_t cleanup_cb)
{
    return hal_handle_add_obj(hal_hdl, cfg_ctxt, add_cb, commit_cb,
                              abort_cb, cleanup_cb);
}

#if 0
static hal_ret_t
cfg_op_ctxt_hal_hdl_add_handle (cfg_op_ctxt_t *cfg_ctxt, hal_obj_id_t obj_id,
                                hal_cfg_op_cb_t add_cb,
                                hal_cfg_commit_cb_t commit_cb,
                                hal_cfg_abort_cb_t abort_cb,
                                hal_cfg_cleanup_cb_t cleanup_cb,
                                hal_handle_t *out_hal_hdl)
{
    hal_ret_t ret;
    hal_handle_t hal_hdl;

    if ((hal_hdl = cfg_op_ctxt_hal_hdl_alloc_init(obj_id)) ==
            HAL_HANDLE_INVALID)
        return HAL_RET_HANDLE_INVALID;

    if ((ret = cfg_op_ctxt_hal_hdl_db_add(cfg_ctxt, hal_hdl,
            add_cb, commit_cb, abort_cb, cleanup_cb)) != HAL_RET_OK)
        return ret;

    *out_hal_hdl = hal_hdl;
    return HAL_RET_OK;
}
#endif

//-----------------------------------------------------------------------------
// DHL Entry routines
//-----------------------------------------------------------------------------

static inline void
cfg_op_ctxt_dhl_entry_init (dhl_entry_t *dhl_entry, hal_handle_t hal_hdl,
                            void *obj)
{
    dllist_reset(&dhl_entry->dllist_ctxt);
    dhl_entry->handle = hal_hdl;
    dhl_entry->obj = obj;
}

static inline void
cfg_op_ctxt_dhl_entry_db_add (cfg_op_ctxt_t *cfg_ctxt, dhl_entry_t *dhl_entry)
{
    dllist_add(&cfg_ctxt->dhl, &dhl_entry->dllist_ctxt);
}

static inline hal_ret_t
cfg_op_ctxt_dhl_entry_handle (cfg_op_ctxt_t *cfg_ctxt, dhl_entry_t *dhl_entry,
                              hal_handle_t hal_hdl, void *obj)
{
    cfg_op_ctxt_dhl_entry_init(dhl_entry, hal_hdl, obj);
    cfg_op_ctxt_dhl_entry_db_add(cfg_ctxt, dhl_entry);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// cfg_op_ctxt routines
//
// The flow of operations : 
//     - Add/Delete a hal handle for the given object (along with callbacks)
//     - Allocate a DHL entry with the above HAL handle and the object
//     - Add the DHL entry to a list maintained in cfg_op_ctxt
//     - Add the hal handle to the database and call the callbacks
//-----------------------------------------------------------------------------

static inline void
cfg_op_ctxt_init (cfg_op_ctxt_t *cfg_ctxt, void *app_ctxt)
{
    cfg_ctxt->app_ctxt = app_ctxt;
    dllist_reset(&cfg_ctxt->dhl);
}

static inline void
cfg_op_ctxt_uninit (cfg_op_ctxt_t *cfg_ctxt)
{
}

hal_ret_t
cfg_ctxt_op_create_handle (hal_obj_id_t obj_id, void *obj, void *app_ctxt,
                           hal_cfg_op_cb_t add_cb,
                           hal_cfg_commit_cb_t commit_cb,
                           hal_cfg_abort_cb_t abort_cb,
                           hal_cfg_cleanup_cb_t cleanup_cb,
                           hal_handle_t *hal_hdl)
{
    hal_ret_t ret;
    dhl_entry_t dhl_entry = { 0 };
    cfg_op_ctxt_t cfg_ctxt = { 0 };

    cfg_op_ctxt_init(&cfg_ctxt, app_ctxt);

    if ((*hal_hdl = cfg_op_ctxt_hal_hdl_alloc_init(obj_id)) ==
            HAL_HANDLE_INVALID)
        return HAL_RET_HANDLE_INVALID;

    if ((ret = cfg_op_ctxt_dhl_entry_handle(&cfg_ctxt, &dhl_entry, *hal_hdl,
            obj)) != HAL_RET_OK)
        return ret;

    if ((ret = cfg_op_ctxt_hal_hdl_db_add(&cfg_ctxt, *hal_hdl,
            add_cb, commit_cb, abort_cb, cleanup_cb)) != HAL_RET_OK)
        return ret;

    return ret;
}

}  // namespace hal
