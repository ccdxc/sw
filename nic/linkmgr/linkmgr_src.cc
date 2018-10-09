//----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// PI implementation for port service
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/port_mac.hpp"
#include "nic/sdk/include/sdk/port_serdes.hpp"
#include "linkmgr_src.hpp"
#include "nic/linkmgr/utils.hpp"
#include "nic/linkmgr/linkmgr_utils.hpp"
#include "nic/sdk/include/sdk/asic/capri/cap_mx_api.h"

using hal::cfg_op_ctxt_t;
using hal::dhl_entry_t;
using sdk::lib::dllist_add;
using sdk::lib::dllist_reset;
using sdk::linkmgr::port_args_t;
using sdk::SDK_RET_OK;
using sdk::linkmgr::mac_info_t;
using grpc::ServerBuilder;
using sdk::linkmgr::linkmgr_thread_id_t;

namespace linkmgr {

class linkmgr_state *g_linkmgr_state;

sdk::lib::thread *
current_thread (void)
{
    return sdk::lib::thread::current_thread();
}

sdk::lib::catalog*
catalog (void)
{
    return g_linkmgr_state->catalog();
}

#define LINKMGR_CALLOC(var, ID, type, ...)  {  \
    void  *mem   = NULL;                       \
    mem = HAL_CALLOC(ID, sizeof(type));        \
    SDK_ABORT(mem != NULL);                    \
    var = new (mem) type(__VA_ARGS__);         \
}

static hal_ret_t
svc_reg (ServerBuilder *server_builder,
         bool process_mode)
{
    // register all services
    PortServiceImpl  *port_svc  = NULL;
    DebugServiceImpl *debug_svc = NULL;

    if (NULL == server_builder) {
        return HAL_RET_ERR;
    }

    LINKMGR_CALLOC(port_svc, hal::HAL_MEM_ALLOC_LINKMGR, PortServiceImpl);

    if (port_svc != NULL) {
        server_builder->RegisterService(port_svc);
    } else {
        HAL_TRACE_ERR("port_svc NULL");
    }

    if (process_mode == true) {
        LINKMGR_CALLOC(debug_svc, hal::HAL_MEM_ALLOC_LINKMGR, DebugServiceImpl);

        if (debug_svc != NULL) {
            server_builder->RegisterService(debug_svc);
        } else {
            HAL_TRACE_ERR("debug_svc NULL");
        }
    }

    return HAL_RET_OK;
}

hal_ret_t
linkmgr_init (sdk::linkmgr::linkmgr_cfg_t *sdk_cfg)
{
    hal_ret_t  ret_hal   = HAL_RET_OK;
    sdk_ret_t  sdk_ret   = SDK_RET_OK;

    g_linkmgr_state = linkmgr_state::factory();
    HAL_ASSERT_RETURN((g_linkmgr_state != NULL), HAL_RET_ERR);

    // store the catalog in global hal state
    g_linkmgr_state->set_catalog(sdk_cfg->catalog);

    sdk_ret = sdk::linkmgr::linkmgr_init(sdk_cfg);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("linkmgr init failed");
        return HAL_RET_ERR;
    }

    svc_reg((ServerBuilder *)sdk_cfg->server_builder,
            sdk_cfg->process_mode);

    return ret_hal;
}

/////////////////////////////////////////////////
// PORT CRUD APIs
/////////////////////////////////////////////////

void *
port_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    port_t                      *pi_p = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    pi_p = (port_t *)hal_handle_get_obj(ht_entry->handle_id);
    HAL_ASSERT(pi_p != NULL);
    return (void *)&(pi_p->port_num);
}

uint32_t
port_id_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(port_num_t)) % ht_size;
}

bool
port_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(port_num_t *)key1 == *(port_num_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a port to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
port_add_to_db (port_t *pi_p, hal_handle_t handle)
{
    hal_ret_t                 ret     = HAL_RET_OK;
    sdk_ret_t                 sdk_ret = SDK_RET_OK;
    hal_handle_id_ht_entry_t  *entry  = NULL;

    HAL_TRACE_DEBUG("Adding port: {} to port id hash table", pi_p->port_num);

    // allocate an entry to establish mapping from port id to its handle
    entry = (hal_handle_id_ht_entry_t *)g_linkmgr_state->
                        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        HAL_TRACE_ERR("Could not allocate ht entry for port: {}",
                      pi_p->port_num);
        return HAL_RET_OOM;
    }

    // add mapping from port num to its handle
    entry->handle_id = handle;
    sdk_ret = g_linkmgr_state->port_id_ht()->insert_with_key(
                                    &pi_p->port_num, entry, &entry->ht_ctxt);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add port num to handle mapping for port: {},"
                      " err: {}",  pi_p->port_num, sdk_ret);
        g_linkmgr_state->hal_handle_id_ht_entry_slab()->free(entry);
    }

    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    pi_p->hal_handle_id = handle;

    return ret;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
port_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    port_args_t   *port_args = NULL;
    dllist_ctxt_t *lnode     = NULL;
    dhl_entry_t   *dhl_entry = NULL;
    port_t        *pi_p      = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Invalid cfg_ctxt");
        return HAL_RET_INVALID_ARG;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    port_args = (port_args_t *)cfg_ctxt->app_ctxt;

    pi_p = (port_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("port_num: {}, create add CB.", pi_p->port_num);

    pi_p->pd_p = sdk::linkmgr::port_create(port_args);
    if (NULL == pi_p->pd_p) {
        HAL_TRACE_ERR("Failed to create pd for port: {}", pi_p->port_num);
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as port_create_add_cb() was a success
//------------------------------------------------------------------------------
hal_ret_t
port_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t      ret           = HAL_RET_OK;
    dllist_ctxt_t  *lnode        = NULL;
    dhl_entry_t    *dhl_entry    = NULL;
    port_t         *pi_p         = NULL;
    hal_handle_t   hal_handle_id = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Invalid cfg_ctxt");
        return HAL_RET_INVALID_ARG;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle_id = dhl_entry->handle;

    HAL_TRACE_DEBUG("port_num: {}, create commit CB.", pi_p->port_num);

    // Add to port id hash table
    ret = port_add_to_db(pi_p, hal_handle_id);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add port: {} to db, err: {}",
                      pi_p->port_num, ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// port_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI vrf
//------------------------------------------------------------------------------
hal_ret_t
port_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t      ret           = HAL_RET_OK;
    sdk_ret_t      sdk_ret       = SDK_RET_OK;
    dhl_entry_t    *dhl_entry    = NULL;
    port_t         *pi_p         = NULL;
    hal_handle_t   hal_handle_id = 0;
    dllist_ctxt_t  *lnode        = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Invalid cfg_ctxt");
        return HAL_RET_INVALID_ARG;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle_id = dhl_entry->handle;

    HAL_TRACE_DEBUG("port_num: {}, create abort CB", pi_p->port_num);

    // delete call to PD
    if (pi_p->pd_p) {
        sdk_ret = sdk::linkmgr::port_delete(pi_p->pd_p);
        if (sdk_ret != SDK_RET_OK) {
            HAL_TRACE_ERR("Failed to delete pd for port: {}, err: {}",
                          pi_p->port_num, sdk_ret);
        }
    }

    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // remove the object
    hal::hal_handle_free(hal_handle_id);

    // free PI port
    port_free(pi_p);

    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
port_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
port_prepare_rsp (PortResponse *rsp, hal_ret_t ret, hal_handle_t hal_handle_id)
{
    if (ret == HAL_RET_OK && hal_handle_id != 0) {
        //rsp->mutable_status()->set_port_handle(hal_handle_id);
    }
    rsp->set_api_status(hal::hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a port create request
//------------------------------------------------------------------------------
hal_ret_t
port_create (port_args_t *port_args, hal_handle_t *hal_handle)
{
    hal_ret_t      ret        = HAL_RET_OK;
    port_t         *pi_p      = NULL;
    dhl_entry_t    dhl_entry  = { 0 };
    cfg_op_ctxt_t  cfg_ctxt   = { 0 };

    HAL_TRACE_DEBUG("Port create with id: {}", port_args->port_num);

    *hal_handle = HAL_RET_HANDLE_INVALID;

    // check if port exists already, and reject if one is found
    if (find_port_by_id(port_args->port_num)) {
        HAL_TRACE_ERR("Failed to create a port, "
                      "port: {} exists already",
                      port_args->port_num);
        return HAL_RET_ENTRY_EXISTS;
    }

    // instantiate the port
    pi_p = port_alloc_init();
    if (pi_p == NULL) {
        HAL_TRACE_ERR("unable to allocate handle/memory for port: {}. ret: {}",
                       port_args->port_num, ret);
        return HAL_RET_OOM;
    }

    pi_p->hal_handle_id = hal_handle_alloc(hal::HAL_OBJ_ID_PORT);
    if (pi_p->hal_handle_id == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for port: {}",
                       port_args->port_num);
        port_free(pi_p);
        return HAL_RET_ERR;
    }

    pi_p->port_num = port_args->port_num;

    dhl_entry.handle  = pi_p->hal_handle_id;
    dhl_entry.obj     = pi_p;
    cfg_ctxt.app_ctxt = port_args;

    dllist_reset(&cfg_ctxt.dhl);
    dllist_reset(&dhl_entry.dllist_ctxt);
    dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);

    ret = hal_handle_add_obj(pi_p->hal_handle_id,
                             &cfg_ctxt,
                             port_create_add_cb,
                             port_create_commit_cb,
                             port_create_abort_cb,
                             port_create_cleanup_cb);

    // if there is an error, port will be freed in abort CB
    if (ret != HAL_RET_OK && pi_p != NULL) {
        pi_p = NULL;
    }

    *hal_handle = pi_p ? pi_p->hal_handle_id : HAL_HANDLE_INVALID;

    return ret;
}

//------------------------------------------------------------------------------
// get port from either id or handle
//------------------------------------------------------------------------------
port_t *
port_lookup_key_or_handle (const kh::PortKeyHandle& key_handle)
{
    if (key_handle.key_or_handle_case() ==
            kh::PortKeyHandle::kPortId) {
        return find_port_by_id(key_handle.port_id());
    }

    if (key_handle.key_or_handle_case() ==
            kh::PortKeyHandle::kPortHandle) {
        return find_port_by_handle(key_handle.port_handle());
    }

    return NULL;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
//------------------------------------------------------------------------------
hal_ret_t
port_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t      ret        = HAL_RET_OK;
    sdk_ret_t      sdk_ret    = SDK_RET_OK;
    port_args_t    *port_args = NULL;
    dllist_ctxt_t  *lnode     = NULL;
    dhl_entry_t    *dhl_entry = NULL;
    port_t         *pi_p      = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Invalid cfg_ctxt");
        return HAL_RET_INVALID_ARG;
    }

    port_args = (port_args_t *)cfg_ctxt->app_ctxt;

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // send the obj to PD for update
    pi_p = (port_t *)dhl_entry->cloned_obj;

    sdk_ret = sdk::linkmgr::port_update(pi_p->pd_p, port_args);

    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to update pd for port: {}, err: {}",
                       pi_p->port_num, sdk_ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD port.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
port_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    // port update doesn't create a clone obj
    return HAL_RET_OK;

# if 0
    hal_ret_t      ret         = HAL_RET_OK;
    sdk_ret_t      sdk_ret     = SDK_RET_OK;
    port_args_t    port_args   = { 0 };
    dllist_ctxt_t  *lnode      = NULL;
    dhl_entry_t    *dhl_entry  = NULL;
    port_t         *pi_p       = NULL;
    port_t         *pi_clone_p = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // send the original object to PD to free the memory
    pi_p = (port_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("update commit CB {}.",
                     pi_p->port_num);
    printf("Original: %p, Clone: %p\n", pi_p, pi_clone_p);

    // Free PD
    sdk::linkmgr::port_args_init(&port_args);
    sdk_ret = sdk::linkmgr::port_delete(pi_p->pd_p);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("failed to free original pd, err: {}",
                       ret);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // Free PI
    port_free(pi_p);

end:
    return ret;
#endif
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
port_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    // port update doesn't create a clone obj
    return HAL_RET_OK;

#if 0
    hal_ret_t        ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret     = SDK_RET_OK;
    port_args_t   port_args = { 0 };
    dllist_ctxt_t    *lnode = NULL;
    dhl_entry_t      *dhl_entry = NULL;
    port_t           *pi_p = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // send the cloned object to PD to be freed
    pi_p = (port_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update abort CB {}.",
                     pi_p->port_num);

    // Free PD
    sdk::linkmgr::port_args_init(&port_args);
    sdk_ret = sdk::linkmgr::port_delete(pi_p->pd_p);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("failed to delete pd, err: {}",
                       ret);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // Free PI
    port_free(pi_p);
end:
    return ret;
#endif
}

hal_ret_t
port_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a port update request
//------------------------------------------------------------------------------
hal_ret_t
port_update (port_args_t *port_args)
{
    port_t         *pi_p     = NULL;
    cfg_op_ctxt_t  cfg_ctxt  = { 0 };
    dhl_entry_t    dhl_entry = { 0 };

    pi_p = find_port_by_id(port_args->port_num);
    if (!pi_p) {
        HAL_TRACE_ERR("Failed to find port: {}", port_args->port_num);
        return HAL_RET_PORT_NOT_FOUND;
    }

    HAL_TRACE_DEBUG("port update for port: {}", port_args->port_num);

    // form ctxt and call infra update object
    dhl_entry.handle     = pi_p->hal_handle_id;
    dhl_entry.obj        = pi_p;
    dhl_entry.cloned_obj = pi_p;    // no clone for port obj
    cfg_ctxt.app_ctxt    = port_args;

    dllist_reset(&dhl_entry.dllist_ctxt);
    dllist_reset(&cfg_ctxt.dhl);
    dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);

    return hal_handle_upd_obj(pi_p->hal_handle_id,
                              &cfg_ctxt,
                              port_update_upd_cb,
                              port_update_commit_cb,
                              port_update_abort_cb,
                              port_update_cleanup_cb);
}

//------------------------------------------------------------------------------
// delete a port from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
port_del_from_db (port_t *pi_p)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing port: {} from port id hash table",
                    pi_p->port_num);

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)
            g_linkmgr_state->port_id_ht()->remove(&pi_p->port_num);

    // free up
    g_linkmgr_state->hal_handle_id_ht_entry_slab()->free(entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
port_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    sdk_ret_t      sdk_ret    = SDK_RET_OK;
    dllist_ctxt_t  *lnode     = NULL;
    dhl_entry_t    *dhl_entry = NULL;
    port_t         *pi_p      = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Invalid cfg_ctxt");
        return HAL_RET_INVALID_ARG;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("delete del CB fort port: {}", pi_p->port_num);

    sdk_ret = sdk::linkmgr::port_delete(pi_p->pd_p);

    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to delete pd for port: {}, err: {}",
                      pi_p->port_num, sdk_ret);
    }

    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

//------------------------------------------------------------------------------
// Update PI DBs as port_delete_del_cb() was a succcess
//      a. Delete from port id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI port
//------------------------------------------------------------------------------
hal_ret_t
port_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t      ret        = HAL_RET_OK;
    dllist_ctxt_t  *lnode     = NULL;
    dhl_entry_t    *dhl_entry = NULL;
    port_t         *pi_p      = NULL;
    hal_handle_t   hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Invalid cfg_ctxt");
        return HAL_RET_INVALID_ARG;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit CB for port: {}", pi_p->port_num);

    // a. Remove from port id hash table
    ret = port_del_from_db(pi_p);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del port: {} from db, err: {}",
                       pi_p->port_num, ret);
        return ret;
    }

    // b. Remove object from handle id based hash table
    hal::hal_handle_free(hal_handle);

    // c. Free PI port
    port_free(pi_p);

    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
port_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
port_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a port delete request
//------------------------------------------------------------------------------
hal_ret_t
port_delete (port_args_t *port_args)
{
    port_t         *pi_p     = NULL;
    cfg_op_ctxt_t  cfg_ctxt  = { 0 };
    dhl_entry_t    dhl_entry = { 0 };

    pi_p = find_port_by_id(port_args->port_num);

    if (pi_p == NULL) {
        HAL_TRACE_ERR("Failed to find port: {}", port_args->port_num);
        return HAL_RET_PORT_NOT_FOUND;
    }

    HAL_TRACE_DEBUG("port delete for port: {}", pi_p->port_num);

    // form ctxt and call infra add
    dhl_entry.handle  = pi_p->hal_handle_id;
    dhl_entry.obj     = pi_p;
    cfg_ctxt.app_ctxt = NULL;

    dllist_reset(&cfg_ctxt.dhl);
    dllist_reset(&dhl_entry.dllist_ctxt);
    dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);

    return hal_handle_del_obj(pi_p->hal_handle_id,
                              &cfg_ctxt,
                              port_delete_del_cb,
                              port_delete_commit_cb,
                              port_delete_abort_cb,
                              port_delete_cleanup_cb);
}

static bool
port_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_ret_t   hal_ret   = HAL_RET_OK;
    sdk_ret_t   sdk_ret   = SDK_RET_OK;
    port_t      *port     = NULL;
    port_args_t port_args = { 0 };

    uint64_t    stats_data[MAX_MAC_STATS];

    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;

    port = (port_t *)hal_handle_get_obj(entry->handle_id);

    sdk::linkmgr::port_args_init(&port_args);

    port_args.port_num   = port->port_num;
    port_args.stats_data = stats_data;

    sdk_ret = sdk::linkmgr::port_get(port->pd_p, &port_args);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to get pd for port: {}, err: {}",
                      port->port_num, sdk_ret);
    }

    hal_ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    port_ht_cb_ctxt_t *ht_cb_ctxt = (port_ht_cb_ctxt_t*) ctxt;

    // Invoke svc cb
    ht_cb_ctxt->cb(&port_args, ht_cb_ctxt->ctxt, hal_ret);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

hal_ret_t
port_get_all (port_get_cb_t port_get_cb, void *ctxt)
{
    port_ht_cb_ctxt_t ht_cb_ctxt;

    ht_cb_ctxt.cb   = port_get_cb;
    ht_cb_ctxt.ctxt = ctxt;

    // Walk the objects and invoke ht cb
    g_linkmgr_state->port_id_ht()->walk(port_get_ht_cb, &ht_cb_ctxt);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a port get request
//------------------------------------------------------------------------------
hal_ret_t
port_get (port_args_t *port_args)
{
    hal_ret_t hal_ret = HAL_RET_OK;
    sdk_ret_t sdk_ret = SDK_RET_OK;
    port_t    *pi_p   = NULL;

    pi_p = find_port_by_id(port_args->port_num);
    if (!pi_p) {
        return HAL_RET_PORT_NOT_FOUND;
    }

    sdk_ret = sdk::linkmgr::port_get(pi_p->pd_p, port_args);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to get pd for port: {}, err: {}",
                      pi_p->port_num, sdk_ret);
    }

    hal_ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return hal_ret;
}

//------------------------------------------------------------------------------
// given a port object, disable the port
//------------------------------------------------------------------------------
static hal_ret_t
port_disable (port_t *port_p)
{
    sdk_ret_t      sdk_ret;
    port_args_t    port_args;

    sdk::linkmgr::port_args_init(&port_args);
    sdk_ret = sdk::linkmgr::port_get(port_p->pd_p, &port_args);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to get pd for port: {}, err: {}",
                      port_p->port_num, sdk_ret);
        return hal_sdk_ret_to_hal_ret(sdk_ret);
    }

    port_args.port_type = port_type_t::PORT_TYPE_NONE;
    port_args.num_lanes = 0;
    port_args.admin_state = port_admin_state_t::PORT_ADMIN_STATE_DOWN;
    sdk_ret = sdk::linkmgr::port_update(port_p->pd_p, &port_args);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to disable port {}, err : {}",
                      port_p->port_num, sdk_ret);
        return hal_sdk_ret_to_hal_ret(sdk_ret);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// callback to disable port
//------------------------------------------------------------------------------
static bool
port_disable_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t    *entry;
    port_t                      *port_p;

    entry = (hal_handle_id_ht_entry_t *)ht_entry;
    port_p = (port_t *)hal_handle_get_obj(entry->handle_id);
    port_disable(port_p);
    return false;    // continue the walk
}

//------------------------------------------------------------------------------
// administratively bring all ports down
// port_num is 0 implies all ports should be brought down
//------------------------------------------------------------------------------
hal_ret_t
port_disable (uint32_t port_num)
{
    kh::PortKeyHandle    kh;
    port_t               *port_p;

    if (port_num == 0) {
        g_linkmgr_state->port_id_ht()->walk(port_disable_cb, NULL); 
    } else {
        kh.set_port_id(port_num);
        port_p = port_lookup_key_or_handle(kh);
        if (port_p == NULL) {
            HAL_TRACE_ERR("Failed to find port {}", port_num);
            return HAL_RET_ERR;
        }
        port_disable(port_p);
    }
    return HAL_RET_OK;
}

static void*
linkmgr_aacs_start (void* ctxt)
{
    sdk::linkmgr::serdes_fns.serdes_aacs_start(*(int*)ctxt);
    return NULL;
}

static hal_ret_t
start_aacs_server (int port)
{
    int    thread_prio = 0, thread_id = 0;

    thread_prio = sched_get_priority_max(SCHED_OTHER);
    if (thread_prio < 0) {
        return HAL_RET_ERR;
    }

    thread_id = linkmgr_thread_id_t::LINKMGR_THREAD_ID_AACS_SERVER;
    sdk::lib::thread *thread = 
        sdk::lib::thread::factory(
                        std::string("linkmgr-aacs-server").c_str(),
                        thread_id,
                        sdk::lib::THREAD_ROLE_CONTROL,
                        0x0 /* use all control cores */,
                        linkmgr_aacs_start,
                        thread_prio - 1,
                        SCHED_OTHER,
                        true);
    if (thread == NULL) {
        SDK_TRACE_ERR("Failed to create linkmgr aacs server thread");
        return HAL_RET_ERR;
    }

    int *int_port = NULL;

    LINKMGR_CALLOC(int_port, hal::HAL_MEM_ALLOC_LINKMGR, int);

    *int_port = port;

    thread->start(int_port);

    return HAL_RET_OK;
}

static void
stop_aacs_server (void)
{
}

hal_ret_t
linkmgr_generic_debug_opn (GenericOpnRequest& req, GenericOpnResponse *resp)
{
    port_t        *pi_p        = NULL;
    port_args_t   port_args    = { 0 };
    sdk_ret_t     sdk_ret      = SDK_RET_OK;
    serdes_info_t *serdes_info = NULL;
    std::string   filename     = "serdes.rom";

    uint32_t    port_id       = 0;
    uint32_t    mac_port_num  = 0;
    uint32_t    speed         = 0;
    uint32_t    num_lanes     = 0;
    bool        enable        = false;
    bool        reset         = false;
    uint32_t    sbus_addr     = 0x0;
    uint32_t    sbus_data     = 0x0;
    uint8_t     reg_addr      = 0x0;
    uint8_t     cmd           = 0x0;
    uint32_t    cable_type    = 0x0;
    int         eye_type      = 0;
    int         hard          = 0;
    int         mac_inst      = 0;
    int         mac_ch        = 0;
    int         int_code      = 0;
    int         int_data      = 0;
    int         aacs_server_port = 0;
    uint64_t    stats_data[MAX_MAC_STATS];

    sdk::linkmgr::port_args_init(&port_args);
    kh::PortKeyHandle key_handle;

    switch (req.opn()) {
        case 0:
            // port disable

            port_id = req.val1();
            key_handle.set_port_id(port_id);

            HAL_TRACE_DEBUG("port_disable for port: {}", port_id);

            pi_p = port_lookup_key_or_handle(key_handle);
            if (!pi_p) {
                HAL_TRACE_ERR("Failed to find port: {}",
                               port_id);
                return HAL_RET_PORT_NOT_FOUND;
            }

            sdk_ret = sdk::linkmgr::port_get(pi_p->pd_p, &port_args);
            if (sdk_ret != SDK_RET_OK) {
                HAL_TRACE_ERR("Failed to get pd for port: {}, err: {}",
                               pi_p->port_num, sdk_ret);
                return HAL_RET_ERR;
            }

            // reset port_type and num_lanes for update
            port_args.port_type   = port_type_t::PORT_TYPE_NONE;
            port_args.num_lanes   = 0;

            port_args.admin_state = port_admin_state_t::PORT_ADMIN_STATE_DOWN;

            sdk_ret = sdk::linkmgr::port_update(pi_p->pd_p, &port_args);
            if (sdk_ret != SDK_RET_OK) {
                HAL_TRACE_ERR("Failed to update pd for port: {}, err: {}",
                               pi_p->port_num, sdk_ret);
                return HAL_RET_ERR;
            }

            break;

        case 1:
            // port enable

            port_id = req.val1();
            key_handle.set_port_id(port_id);

            HAL_TRACE_DEBUG("port_enable for port: {}", port_id);

            pi_p = port_lookup_key_or_handle(key_handle);
            if (!pi_p) {
                HAL_TRACE_ERR("Failed to find port: {}",
                               port_id);
                return HAL_RET_PORT_NOT_FOUND;
            }

            sdk_ret = sdk::linkmgr::port_get(pi_p->pd_p, &port_args);
            if (sdk_ret != SDK_RET_OK) {
                HAL_TRACE_ERR("Failed to get pd for port: {}, err: {}",
                               pi_p->port_num, sdk_ret);
                return HAL_RET_ERR;
            }

            // reset port_type and num_lanes for update
            port_args.port_type   = port_type_t::PORT_TYPE_NONE;
            port_args.num_lanes   = 0;

            port_args.admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;

            sdk_ret = sdk::linkmgr::port_update(pi_p->pd_p, &port_args);
            if (sdk_ret != SDK_RET_OK) {
                HAL_TRACE_ERR("Failed to update pd for port: {}, err: {}",
                               pi_p->port_num, sdk_ret);
                return HAL_RET_ERR;
            }

            break;

        case 2:
            port_id = req.val1();

            if (req.val2() == 1) {
                enable = true;
            }

            key_handle.set_port_id(port_id);

            pi_p = port_lookup_key_or_handle(key_handle);
            if (!pi_p) {
                HAL_TRACE_ERR("Failed to find port: {}",
                               port_id);
                return HAL_RET_PORT_NOT_FOUND;
            }

            port_args_t args;
            memset(&args, 0, sizeof(port_args_t));

            if (sdk::linkmgr::port_get(pi_p->pd_p, &args) != SDK_RET_OK) {
                HAL_TRACE_ERR("Failed to get port info for port: {}", port_id);
                return HAL_RET_ERR;
            }

            mac_info_t mac_info;
            memset(&mac_info, 0, sizeof(mac_info_t));

            mac_info.mac_id    = args.mac_id;
            mac_info.mac_ch    = args.mac_ch;
            mac_info.speed     = static_cast<uint32_t>(args.port_speed);

            mac_info.mtu       = args.mtu;
            mac_info.num_lanes = args.num_lanes;

            // 0: disable fec, 1: enable fec
            // TODO current FEC type is determined by global mode in mx api
            mac_info.fec       = static_cast<uint32_t>(args.fec_type);

            // Enable Tx padding. Disable Rx padding
            mac_info.tx_pad_enable = 1;
            mac_info.rx_pad_enable = 0;

            mac_info.force_global_init = enable;

            HAL_TRACE_DEBUG("mac_cfg mac_id: {}, mac_ch: {}, speed: {},"
                            " mtu: {}, num_lanes: {}, fec: {},"
                            " force_global_init: {}",
                            mac_info.mac_id, mac_info.mac_ch, mac_info.speed,
                            mac_info.mtu, mac_info.num_lanes, mac_info.fec,
                            mac_info.force_global_init);

            sdk::linkmgr::mac_fns.mac_cfg(&mac_info);
            break;

        case 3:
            mac_port_num = req.val1();
            speed        = req.val2();
            num_lanes    = req.val3();

            if (req.val4() == 1) {
                enable = true;
            }

            HAL_TRACE_DEBUG("mac_enable mac_port: {}, speed: {}, num_lanes: {}"
                            ", enable: {}",
                            mac_port_num, speed, num_lanes, enable);

            sdk::linkmgr::mac_fns.mac_enable(
                    mac_port_num, speed, num_lanes, enable);
            break;

        case 4:
            mac_port_num = req.val1();
            speed        = req.val2();
            num_lanes    = req.val3();

            if (req.val4() == 1) {
                reset = true;
            }

            HAL_TRACE_DEBUG("mac_reset mac_port: {}, speed: {}, num_lanes: {}"
                            ", reset: {}",
                            mac_port_num, speed, num_lanes, reset);

            sdk::linkmgr::mac_fns.mac_soft_reset(
                    mac_port_num, speed, num_lanes, reset);
            break;

        case 5:
            mac_port_num = req.val1();
            HAL_TRACE_DEBUG("mac_faults mac_port: {}, faults: {}",
                            mac_port_num,
                            sdk::linkmgr::mac_fns.mac_faults_get(mac_port_num));
            break;

        case 6:
            mac_port_num = req.val1();
            HAL_TRACE_DEBUG("mac_sync mac_port: {}, sync: {}",
                            mac_port_num,
                            sdk::linkmgr::mac_fns.mac_sync_get(mac_port_num));
            break;

        case 7:
            sbus_addr  = req.val1();
            speed      = req.val2();
            cable_type = req.val3();

            serdes_info =
                    catalog()->serdes_info_get(sbus_addr, speed, cable_type);

            HAL_TRACE_DEBUG("serdes_cfg sbus_addr: {}, speed: {}, cable: {}",
                            sbus_addr, speed, cable_type);
            sdk::linkmgr::serdes_fns.serdes_cfg(sbus_addr, serdes_info);
            break;

        case 8:
            sbus_addr = req.val1();

            if (req.val2() == 1) {
                enable = true;
            }

            HAL_TRACE_DEBUG("serdes_output_enable sbus_addr: {}, enable: {}",
                            sbus_addr, enable);
            sdk::linkmgr::serdes_fns.serdes_output_enable(sbus_addr, enable);
            break;

        case 9:
            sbus_addr = req.val1();

            HAL_TRACE_DEBUG("serdes_ical_start sbus_addr: {}",
                            sbus_addr);
            sdk::linkmgr::serdes_fns.serdes_ical_start(sbus_addr);
            break;

        case 10:
            sbus_addr = req.val1();

            HAL_TRACE_DEBUG("serdes_pcal_start sbus_addr: {}",
                            sbus_addr);
            sdk::linkmgr::serdes_fns.serdes_pcal_start(sbus_addr);
            break;

        case 11:
            sbus_addr = req.val1();

            HAL_TRACE_DEBUG("serdes_pcal_continuous_start sbus_addr: {}",
                            sbus_addr);
            sdk::linkmgr::serdes_fns.serdes_pcal_continuous_start(sbus_addr);
            break;

        case 12:
            sbus_addr = req.val1();

            HAL_TRACE_DEBUG(
                    "serdes_dfe_status sbus_addr: {}, status: {}",
                    sbus_addr,
                    sdk::linkmgr::serdes_fns.serdes_dfe_status(sbus_addr));
            break;

        case 13:
            sbus_addr = req.val1();
            eye_type  = req.val2();

            HAL_TRACE_DEBUG("serdes_eye_get sbus_addr: {}, eye_type: {}",
                            sbus_addr, eye_type);
            sdk::linkmgr::serdes_fns.serdes_eye_get(sbus_addr, eye_type);
            break;

        case 14:
            sbus_addr = req.val1();

            if (req.val2() == 1) {
                enable = true;
            }

            HAL_TRACE_DEBUG("serdes_rx_lpbk sbus_addr: {}, enable: {}",
                            sbus_addr, enable);
            sdk::linkmgr::serdes_fns.serdes_rx_lpbk(sbus_addr, enable);
            break;

        case 15:
            sbus_addr = req.val1();
            hard      = req.val2();
            HAL_TRACE_DEBUG("serdes_sbus_reset sbus_addr: {}, hard: {}",
                            sbus_addr, hard);
            sdk::linkmgr::serdes_fns.serdes_sbus_reset(sbus_addr, hard);
            break;

        case 16:
            sbus_addr = req.val1();
            HAL_TRACE_DEBUG("serdes_spico_reset sbus_addr: {}",
                            sbus_addr);
            sdk::linkmgr::serdes_fns.serdes_spico_reset(sbus_addr);
            break;

        case 17:
            sbus_addr = req.val1();
            HAL_TRACE_DEBUG("serdes_spico_upload sbus_addr: {}",
                            sbus_addr);
            sdk::linkmgr::serdes_fns.serdes_spico_upload(
                                        sbus_addr, filename.c_str());
            break;

        case 18:
            mac_inst = req.val1();
            mac_ch   = req.val2();

            HAL_TRACE_DEBUG("mac_stats mac_inst: {}, mac_ch: {}",
                            mac_inst, mac_ch);

            memset(stats_data, 0, sizeof(uint64_t) * MAX_MAC_STATS);
            cap_mx_mac_stat(0 /*chip_id*/, mac_inst, mac_ch, 0, stats_data);
            break;

        case 19:
            mac_inst = req.val1();
            mac_ch   = req.val2();

            HAL_TRACE_DEBUG("mac_serdes_lpbk mac_inst: {}, mac_ch: {},"
                            " enable: {}",
                            mac_inst, mac_ch, req.val3());

            cap_mx_serdes_lpbk_set(0 /*chip_id*/, mac_inst, mac_ch, req.val3());

            break;

        case 20:
            sbus_addr = req.val1();
            reg_addr  = req.val2();
            cmd       = req.val3();
            sbus_data = req.val4();

            HAL_TRACE_DEBUG("sbus_access sbus_addr: {}, reg_addr: {}, cmd: {},"
                            " data: {}, ret: {}",
                            sbus_addr, sbus_addr, cmd, sbus_data,
                            sdk::linkmgr::sbus_access(
                                        sbus_addr, reg_addr, cmd, &sbus_data));
            break;

        case 21:
            sbus_addr = req.val1();
            HAL_TRACE_DEBUG("spico_status sbus_addr: {}", sbus_addr);
            sdk::linkmgr::serdes_fns.serdes_spico_status(sbus_addr);
            break;

        case 22:
            sbus_addr = req.val1();
            HAL_TRACE_DEBUG(
                    "serdes_rev sbus_addr: {}, rev: {}",
                    sbus_addr,
                    sdk::linkmgr::serdes_fns.serdes_get_rev(sbus_addr));
            break;

        case 23:
            sbus_addr = req.val1();
            HAL_TRACE_DEBUG(
                    "serdes_build_id sbus_addr: {}, build_id: {}",
                    sbus_addr,
                    sdk::linkmgr::serdes_fns.serdes_get_build_id(sbus_addr));
            break;

        case 24:
            sbus_addr = req.val1();
            HAL_TRACE_DEBUG(
                    "serdes_spico_crc sbus_addr: {}, spico_crc: {}",
                    sbus_addr,
                    sdk::linkmgr::serdes_fns.serdes_spico_crc(sbus_addr));
            break;

        case 25:
            sbus_addr = req.val1();
            HAL_TRACE_DEBUG(
                    "serdes_get_eng_id sbus_addr: {}, eng_id: {}",
                    sbus_addr,
                    sdk::linkmgr::serdes_fns.serdes_get_eng_id(sbus_addr));
            break;

        case 26:
            aacs_server_port = req.val1();
            start_aacs_server(aacs_server_port);
            HAL_TRACE_DEBUG("AACS server started");
            break;

        case 27:
            stop_aacs_server();
            HAL_TRACE_DEBUG("AACS server stopped");
            break;

        case 28:
            sbus_addr = req.val1();
            int_code  = req.val2();
            int_data  = req.val3();

            HAL_TRACE_DEBUG("spico_int sbus_addr: {}, int_code: {},"
                            " int_data: {}, result: {}",
                            sbus_addr, int_code, int_data,
                            sdk::linkmgr::serdes_fns.serdes_spico_int(
                                    sbus_addr, int_code, int_data));
            break;

        case 29:
            sbus_addr = req.val1();

            if (req.val2() == 1) {
                reset = true;
            }

            HAL_TRACE_DEBUG("serdes_get_errors sbus_addr: {}, clear: {},"
                            " result: {}",
                            sbus_addr, reset,
                            sdk::linkmgr::serdes_fns.serdes_get_errors(
                                sbus_addr, reset));
            break;

        case 30:
            if (req.val1() == 1) {
                enable = true;
            }

            HAL_TRACE_DEBUG("port_link_poll {}", enable);

            sdk::linkmgr::linkmgr_set_link_poll_enable(enable);
            break;

        case 31:
            sbus_addr = req.val1();

            HAL_TRACE_DEBUG("serdes signal detect sbus_addr: {}, signal: {}",
                            sbus_addr,
                            sdk::linkmgr::serdes_fns.serdes_signal_detect(
                                                            sbus_addr));
            break;

        case 32:
            sbus_addr  = req.val1();
            speed      = req.val2();
            cable_type = req.val3();

            serdes_info =
                    catalog()->serdes_info_get(sbus_addr, speed, cable_type);

            HAL_TRACE_DEBUG("serdes_prbs31 sbus_addr: {}, speed: {}, cable: {}",
                            sbus_addr, speed, cable_type);

            sdk::linkmgr::serdes_fns.serdes_prbs_start(sbus_addr, serdes_info);

            break;

        case 33:
            mac_inst  = req.val2();
            sbus_addr = req.val3(); // mac reg addr
            sbus_data = req.val4(); // mac reg data

            if (req.val1() == 0) {
                // READ
                HAL_TRACE_DEBUG (
                        "mac_read mac_inst: {}, addr: {}, data: {}",
                        mac_inst, sbus_addr,
                        cap_mx_apb_read(0 /* chip_id */, mac_inst, sbus_addr));
            } else {
                // WRITE
                HAL_TRACE_DEBUG ("mac_write mac_inst: {}, addr: {}, data: {}",
                                 mac_inst, sbus_addr, sbus_data);
                cap_mx_apb_write(0 /* chip_id */, mac_inst,
                                 sbus_addr, sbus_data);
            }

            break;

        default:
            break;
    }

    return HAL_RET_OK;
}

}    // namespace linkmgr

extern "C" {

uint32_t jtag_wr (unsigned char chip,
                  unsigned long long int reg_addr,
                  uint32_t *sbus_data,
                  unsigned long long int flag)
{
    return 0;
}

uint32_t jtag_rd (unsigned char chip,
                  unsigned long long int reg_addr,
                  uint32_t sbus_data,
                  unsigned long long int flag)
{
    return 0;
}

}   // extern "C"
