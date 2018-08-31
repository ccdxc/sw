//----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// PI implementation for port service
//----------------------------------------------------------------------------

#include "sdk/port_mac.hpp"
#include "sdk/port_serdes.hpp"
#include "linkmgr_src.hpp"
#include "nic/linkmgr/utils.hpp"
#include "nic/linkmgr/linkmgr_utils.hpp"
#include "cap_mx_api.h"

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

static uint32_t
num_fp_lanes (uint32_t port)
{
    return catalog()->num_fp_lanes(port);
}

static uint32_t
breakout_modes (uint32_t port)
{
    return catalog()->breakout_modes(port);
}

static uint32_t
sbus_addr (uint32_t port, uint32_t lane)
{
    return catalog()->sbus_addr(port, lane);
}

static uint32_t
num_fp_ports (void)
{
    return catalog()->num_fp_ports();
}

#define LINKMGR_CALLOC(var, ID, type, ...)  {  \
    void  *mem   = NULL;                       \
    mem = HAL_CALLOC(ID, sizeof(type));        \
    SDK_ABORT(mem != NULL);                    \
    var = new (mem) type(__VA_ARGS__);         \
}

static hal_ret_t
svc_reg(ServerBuilder *server_builder,
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
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("adding to port id hash table");
    // allocate an entry to establish mapping from port id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_linkmgr_state->
                        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from port num to its handle
    entry->handle_id = handle;
    sdk_ret = g_linkmgr_state->port_id_ht()->insert_with_key(&pi_p->port_num,
                                                       entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("failed to add port num to handle mapping, "
                      "err: {}",  sdk_ret);
        g_linkmgr_state->hal_handle_id_ht_entry_slab()->free(entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // TODO: Check if this is the right place
    pi_p->hal_handle_id = handle;

    return ret;
}

//------------------------------------------------------------------------------
// validate an incoming port create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_port_create (PortSpec& spec, PortResponse *rsp)
{
#if 0
    if (!spec.has_meta()) {
        HAL_TRACE_ERR("no meta");
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
#endif

    std::string str;

    // must have key-handle set
    str = "key_or_handle";
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{} not set in request",
                       str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // port type
    str = "port type";
    if (spec.port_type() == ::port::PORT_TYPE_NONE) {
        HAL_TRACE_ERR("not set in request {}",
                       str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // port speed
    str = "port speed";
    if (spec.port_speed() == ::port::PORT_SPEED_NONE) {
        HAL_TRACE_ERR("not set in request {}",
                       str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_SPEED_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // number of lanes for the port
    str = "number of lanes";
    if (spec.num_lanes() == 0) {
        HAL_TRACE_ERR("not set in request {}",
                       str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_NUM_LANES_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
port_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t               ret        = HAL_RET_OK;
    port_args_t             port_args  = { 0 };
    dllist_ctxt_t           *lnode     = NULL;
    dhl_entry_t             *dhl_entry = NULL;
    port_t                  *pi_p      = NULL;
    port_create_app_ctxt_t  *app_ctxt  = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (port_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    pi_p = (port_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("port_num: {}, create add CB.",
                     pi_p->port_num);

    // PD Call to allocate PD resources and HW programming
    sdk::linkmgr::port_args_init(&port_args);

    port_args.port_num = app_ctxt->port_num;
    port_args.port_type   =
            linkmgr::port_type_spec_to_sdk_port_type(app_ctxt->port_type);
    port_args.admin_state =
            linkmgr::port_admin_st_spec_to_sdk_port_admin_st(
                                                    app_ctxt->admin_state);
    port_args.port_speed  =
            linkmgr::port_speed_spec_to_sdk_port_speed(app_ctxt->port_speed);
    port_args.fec_type =
            linkmgr::port_fec_type_spec_to_sdk_port_fec_type(
                                                        app_ctxt->fec_type);
    port_args.mac_id      = app_ctxt->mac_id;
    port_args.mac_ch      = app_ctxt->mac_ch;
    port_args.num_lanes   = app_ctxt->num_lanes;
    port_args.mtu         = app_ctxt->mtu;
    port_args.auto_neg_enable = app_ctxt->auto_neg_enable;
    port_args.debounce_time   = app_ctxt->debounce_time;
    memcpy(port_args.sbus_addr, app_ctxt->sbus_addr,
                                MAX_PORT_LANES * sizeof(uint32_t));

    pi_p->pd_p = sdk::linkmgr::port_create(&port_args);
    if (NULL == pi_p->pd_p) {
        HAL_TRACE_ERR("failed to create port pd, err: {}",
                       ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as port_create_add_cb() was a success
//------------------------------------------------------------------------------
hal_ret_t
port_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                ret = HAL_RET_OK;
    dllist_ctxt_t            *lnode = NULL;
    dhl_entry_t              *dhl_entry = NULL;
    port_t                   *pi_p = NULL;
    hal_handle_t             hal_handle_id = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle_id = dhl_entry->handle;

    HAL_TRACE_DEBUG("port_num: {}, create commit CB.",
                     pi_p->port_num);

    // Add to port id hash table
    ret = port_add_to_db(pi_p, hal_handle_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to add port {} to db, err: {}",
                      pi_p->port_num, ret);
        goto end;
    }

end:
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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle_id = dhl_entry->handle;

    HAL_TRACE_DEBUG("port_num: {}, create abort CB",
                     pi_p->port_num);

    // delete call to PD
    if (pi_p->pd_p) {
        sdk_ret = sdk::linkmgr::port_delete(pi_p->pd_p);
        if (sdk_ret != SDK_RET_OK) {
            HAL_TRACE_ERR("failed to delete port pd, err: {}",
                           ret);
        }
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // remove the object
    hal::hal_handle_free(hal_handle_id);

    // free PI port
    port_free(pi_p);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
port_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
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
port_create (PortSpec& spec, PortResponse *rsp)
{
    hal_ret_t               ret        = HAL_RET_OK;
    port_t                  *pi_p      = NULL;
    port_t                  *pi_p1     = NULL;
    dhl_entry_t             dhl_entry  = { 0 };
    cfg_op_ctxt_t           cfg_ctxt   = { 0 };
    port_create_app_ctxt_t  app_ctxt;

    memset(&app_ctxt, 0, sizeof(port_create_app_ctxt_t));

    hal::hal_api_trace(" API Begin: port create ");
    HAL_TRACE_DEBUG("port create with id: {}",
                    spec.key_or_handle().port_id());

    // check if port exists already, and reject if one is found
    if (find_port_by_id(spec.key_or_handle().port_id())) {
        HAL_TRACE_ERR("failed to create a port, "
                      "port {} exists already",
                      spec.key_or_handle().port_id());
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_ENTRY_EXISTS;
    }

    // validate the request message
    ret = validate_port_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("validation Failed. ret: {}",
                       ret);
        goto end;
    }

    // instantiate the port
    pi_p = port_alloc_init();
    if (pi_p == NULL) {
        HAL_TRACE_ERR("unable to allocate handle/memory ret: {}",
                       ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    pi_p->hal_handle_id = hal_handle_alloc(hal::HAL_OBJ_ID_PORT);

    if (pi_p->hal_handle_id == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("failed to alloc handle for port id {}",
                       spec.key_or_handle().port_id());
        port_free(pi_p);
        pi_p = NULL;
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    pi_p->port_num = spec.key_or_handle().port_id();

    // form ctxt and call infra add
    app_ctxt.port_num    = spec.key_or_handle().port_id();
    app_ctxt.port_type   = spec.port_type();
    app_ctxt.admin_state = spec.admin_state();
    app_ctxt.port_speed  = spec.port_speed();
    app_ctxt.mac_id      = spec.mac_id();
    app_ctxt.mac_ch      = spec.mac_ch();
    app_ctxt.num_lanes   = spec.num_lanes();
    app_ctxt.fec_type    = spec.fec_type();
    app_ctxt.auto_neg_enable = spec.auto_neg_enable();
    app_ctxt.debounce_time   = spec.debounce_time();

    if (spec.mtu() == 0) {
        // TODO define?
        app_ctxt.mtu = 9216;
    } else {
        app_ctxt.mtu = spec.mtu();
    }

    for (uint32_t i = 0; i < spec.num_lanes(); ++i) {
        app_ctxt.sbus_addr[i] = sbus_addr(spec.key_or_handle().port_id(), i);
    }

    dhl_entry.handle = pi_p->hal_handle_id;
    dhl_entry.obj = pi_p;
    cfg_ctxt.app_ctxt = &app_ctxt;
    dllist_reset(&cfg_ctxt.dhl);
    dllist_reset(&dhl_entry.dllist_ctxt);
    dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = HAL_RET_OK;
    ret = hal_handle_add_obj(pi_p->hal_handle_id,
                             &cfg_ctxt,
                             port_create_add_cb,
                             port_create_commit_cb,
                             port_create_abort_cb,
                             port_create_cleanup_cb);

    // TODO needed?
    if (ret == HAL_RET_OK) {
        pi_p1 = find_port_by_handle(pi_p->hal_handle_id);
        HAL_ASSERT(pi_p == pi_p1);
    }

end:
    if (ret != HAL_RET_OK && pi_p != NULL) {
        // if there is an error, port will be freed in abort CB
        pi_p = NULL;
    }

    port_prepare_rsp(rsp, ret, pi_p ? pi_p->hal_handle_id : HAL_HANDLE_INVALID);

    hal::hal_api_trace(" API End: port create ");
    return ret;
}

//------------------------------------------------------------------------------
// validate port update request
//------------------------------------------------------------------------------
hal_ret_t
validate_port_update (PortSpec& spec, PortResponse*rsp)
{
    // TODO no meta check?

    std::string str;

    // must have key-handle set
    str = "key_or_handle";
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{} not set in update request",
                       str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // port type must not be set in update
    str = "port type";
    if (spec.port_type() != ::port::PORT_TYPE_NONE) {
        HAL_TRACE_ERR("{} set in update request",
                       str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // number of lanes must not be set in update
    str = "number of lanes";
    if (spec.num_lanes() != 0) {
        HAL_TRACE_ERR("{} set in update request",
                       str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_NUM_LANES_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
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
    hal_ret_t               ret = HAL_RET_OK;
    sdk_ret_t               sdk_ret = SDK_RET_OK;
    port_args_t             port_args = { 0 };
    dllist_ctxt_t           *lnode = NULL;
    dhl_entry_t             *dhl_entry = NULL;
    port_t                  *pi_p = NULL;
    port_update_app_ctxt_t  *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    app_ctxt = (port_create_app_ctxt_t *)cfg_ctxt->app_ctxt;
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // send the cloned obj to PD for update
    pi_p = (port_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("update upd cb {}.",
                     pi_p->port_num);

    // 1. PD Call to allocate PD resources and HW programming
    sdk::linkmgr::port_args_init(&port_args);

    port_args.admin_state =
              linkmgr::port_admin_st_spec_to_sdk_port_admin_st(
                                                        app_ctxt->admin_state);
    port_args.port_speed  =
              linkmgr::port_speed_spec_to_sdk_port_speed(app_ctxt->port_speed);
    port_args.fec_type =
              linkmgr::port_fec_type_spec_to_sdk_port_fec_type(
                                                            app_ctxt->fec_type);
    port_args.auto_neg_enable = app_ctxt->auto_neg_enable;
    port_args.debounce_time = app_ctxt->debounce_time;

    sdk_ret = sdk::linkmgr::port_update(pi_p->pd_p, &port_args);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("failed to update port pd, err: {}",
                       ret);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

end:
    return ret;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD port.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
port_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t      ret         = HAL_RET_OK;
    //sdk_ret_t      sdk_ret     = SDK_RET_OK;
    //port_args_t    port_args   = { 0 };
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

#if 0
    // Free PD
    sdk::linkmgr::port_args_init(&port_args);
    sdk_ret = sdk::linkmgr::port_delete(pi_p->pd_p);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("failed to free original port pd, err: {}",
                       ret);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // Free PI
    port_free(pi_p);
#endif

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
port_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
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
        HAL_TRACE_ERR("failed to delete port pd, err: {}",
                       ret);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    // Free PI
    port_free(pi_p);
end:
    return ret;
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
port_update (PortSpec& spec, PortResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    port_t                     *pi_p = NULL;
    const kh::PortKeyHandle  &kh = spec.key_or_handle();
    cfg_op_ctxt_t              cfg_ctxt = { 0 };
    dhl_entry_t                dhl_entry = { 0 };
    port_update_app_ctxt_t     app_ctxt;

    hal::hal_api_trace(" API Begin: port update ");

    memset (&app_ctxt, 0, sizeof(port_create_app_ctxt_t));

    // validate the request message
    ret = validate_port_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("port update validation failed, ret : {}",
                       ret);
        goto end;
    }

    pi_p = port_lookup_key_or_handle(kh);
    if (!pi_p) {
        HAL_TRACE_ERR("failed to find port id {}, handle {}",
                       kh.port_id(), kh.port_handle());
        ret = HAL_RET_PORT_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("port update for port id {}, handle {}",
                     kh.port_id(), kh.port_handle());

    // form ctxt and call infra add
    app_ctxt.admin_state = spec.admin_state();
    app_ctxt.port_speed  = spec.port_speed();
    app_ctxt.fec_type    = spec.fec_type();
    app_ctxt.auto_neg_enable = spec.auto_neg_enable();
    app_ctxt.debounce_time   = spec.debounce_time();

    dhl_entry.cloned_obj = pi_p;

    // form ctxt and call infra update object
    dhl_entry.handle = pi_p->hal_handle_id;
    dhl_entry.obj = pi_p;
    dllist_reset(&dhl_entry.dllist_ctxt);

    cfg_ctxt.app_ctxt = &app_ctxt;
    dllist_reset(&cfg_ctxt.dhl);
    dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);

    /* upd_cb:    Update the cloned object to reflect new config
     * commit_cb: Free the original object. Infra swaps orignal obj with clone
     * abort_cb:  Free the cloned object
     */
    ret = hal_handle_upd_obj(pi_p->hal_handle_id,
                             &cfg_ctxt,
                             port_update_upd_cb,
                             port_update_commit_cb,
                             port_update_abort_cb,
                             port_update_cleanup_cb);
end:
    port_prepare_rsp(rsp, ret, pi_p->hal_handle_id);

    hal::hal_api_trace(" API End: port update ");
    return ret;
}

//------------------------------------------------------------------------------
// validate port delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_port_delete_req (PortDeleteRequest& req, PortDeleteResponseMsg *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("spec has no key or handle");
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// delete a port from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
port_del_from_db (port_t *pi_p)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("removing from port id hash table");

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_linkmgr_state->port_id_ht()->
                                            remove(&pi_p->port_num);
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
    hal_ret_t        ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret     = SDK_RET_OK;
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

    pi_p = (port_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("delete del CB {}",
                     pi_p->port_num);

    sdk_ret = sdk::linkmgr::port_delete(pi_p->pd_p);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("failed to delete port pd, err: {}",
                       ret);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

end:
    return ret;
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
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode = NULL;
    dhl_entry_t     *dhl_entry = NULL;
    port_t          *pi_p = NULL;
    hal_handle_t    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit CB {}",
                     pi_p->port_num);

    // a. Remove from port id hash table
    ret = port_del_from_db(pi_p);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to del port {} from db, err: {}",
                       pi_p->port_num, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal::hal_handle_free(hal_handle);

    // c. Free PI port
    port_free(pi_p);

end:
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
port_delete (PortDeleteRequest& req, PortDeleteResponseMsg *rsp)
{
    hal_ret_t                  ret = HAL_RET_OK;
    port_t                     *pi_p = NULL;
    cfg_op_ctxt_t              cfg_ctxt = { 0 };
    dhl_entry_t                dhl_entry = { 0 };
    const kh::PortKeyHandle  &kh = req.key_or_handle();

    hal::hal_api_trace(" API Begin: port delete ");

    // validate the request message
    ret = validate_port_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("port delete request validation failed, ret : {}",
                       ret);
        goto end;
    }

    pi_p = port_lookup_key_or_handle(kh);
    if (pi_p == NULL) {
        HAL_TRACE_ERR("failed to find port, id {}, handle {}",
                       kh.port_id(), kh.port_handle());
        ret = HAL_RET_PORT_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("port delete for id {}",
                     pi_p->port_num);

    // form ctxt and call infra add
    dhl_entry.handle = pi_p->hal_handle_id;
    dhl_entry.obj = pi_p;
    cfg_ctxt.app_ctxt = NULL;
    dllist_reset(&cfg_ctxt.dhl);
    dllist_reset(&dhl_entry.dllist_ctxt);
    dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);

    ret = hal_handle_del_obj(pi_p->hal_handle_id,
                             &cfg_ctxt,
                             port_delete_del_cb,
                             port_delete_commit_cb,
                             port_delete_abort_cb,
                             port_delete_cleanup_cb);
end:
    rsp->add_response()->set_api_status(hal::hal_prepare_rsp(ret));

    hal::hal_api_trace(" API End: port delete ");
    return ret;
}

static void
port_populate_get_response (port_t *pi_p, PortGetResponse *response)
{
    PortSpec    *spec     = NULL;
    port_args_t port_args = { 0 };
    hal_ret_t   ret       = HAL_RET_OK;
    sdk_ret_t   sdk_ret   = SDK_RET_OK;

    // fill in the config spec of this port
    spec = response->mutable_spec();
    spec->mutable_key_or_handle()->set_port_id(pi_p->port_num);

    // 1. PD Call to get PD resources
    sdk::linkmgr::port_args_init(&port_args);

    sdk_ret = sdk::linkmgr::port_get(pi_p->pd_p, &port_args);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("failed to get port pd, err: {}",
                       ret);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret == HAL_RET_OK) {
        spec->set_port_type
                (linkmgr::sdk_port_type_to_port_type_spec(port_args.port_type));
        spec->set_port_speed
                (linkmgr::sdk_port_speed_to_port_speed_spec(port_args.port_speed));
        spec->set_admin_state
                (linkmgr::sdk_port_admin_st_to_port_admin_st_spec
                                        (port_args.admin_state));
        spec->set_fec_type
                (linkmgr::sdk_port_fec_type_to_port_fec_type_spec
                                            (port_args.fec_type));
        response->mutable_status()->set_oper_status(
                (linkmgr::sdk_port_oper_st_to_port_oper_st_spec
                                        (port_args.oper_status)));
        spec->set_mac_id    (port_args.mac_id);
        spec->set_mac_ch    (port_args.mac_ch);
        spec->set_num_lanes (port_args.num_lanes);
        spec->set_auto_neg_enable (port_args.auto_neg_enable);
        spec->set_debounce_time   (port_args.debounce_time);
    }

    response->set_api_status(hal::hal_prepare_rsp(ret));
}

static bool
port_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry    = (hal_handle_id_ht_entry_t *)ht_entry;
    PortGetResponseMsg       *rsp      = (PortGetResponseMsg *)ctxt;
    port_t                   *port     = NULL;
    PortGetResponse          *response = NULL;

    port = (port_t *)hal_handle_get_obj(entry->handle_id);

    response = rsp->add_response();

    port_populate_get_response(port, response);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}

//------------------------------------------------------------------------------
// process a port get request
//------------------------------------------------------------------------------
hal_ret_t
port_get (PortGetRequest& req, PortGetResponseMsg *rsp)
{
    port_t          *pi_p     = NULL;
    PortGetResponse *response = NULL;

    hal::hal_api_trace(" API Begin: port get ");

    if (!req.has_key_or_handle()) {
        g_linkmgr_state->port_id_ht()->walk(port_get_ht_cb, rsp);
        return HAL_RET_OK;
    }

    response = rsp->add_response();

    pi_p = port_lookup_key_or_handle(req.key_or_handle());
    if (!pi_p) {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_PORT_NOT_FOUND;
    }

    port_populate_get_response (pi_p, response);

    hal::hal_api_trace(" API End: port get ");

    return HAL_RET_OK;
}

hal_ret_t
populate_port_info(uint32_t fp_port, PortInfoGetResponse *response)
{
    response->mutable_spec()->mutable_key_or_handle()->set_port_id(fp_port);
    response->mutable_spec()->set_num_lanes(num_fp_lanes(fp_port));

    uint32_t breakout_mask = breakout_modes(fp_port);

    while (breakout_mask != 0) {
        // find the position (0 based) of the first set bit
        uint32_t             bit  = ffs(breakout_mask) - 1;

        port_breakout_mode_t mode = static_cast<port_breakout_mode_t>(bit);

        response->mutable_spec()->add_breakout_modes(
                sdk_port_breakout_mode_to_port_breakout_mode_spec(mode));

        // clear the bit
        breakout_mask = breakout_mask & ~(1 << bit);
    }

    return HAL_RET_OK;
}

hal_ret_t
populate_port_info_response(uint32_t fp_port,
                            PortInfoGetResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;

    PortInfoGetResponse *response = rsp->add_response();

    ret = populate_port_info(fp_port, response);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting port info {}", fp_port);
        response->set_api_status(types::API_STATUS_NOT_FOUND);
    } else {
        response->set_api_status(types::API_STATUS_OK);
    }

    return ret;
}

hal_ret_t
port_info_get (PortInfoGetRequest& req, PortInfoGetResponseMsg *rsp)
{
    uint32_t  fp_port = 0;
    hal_ret_t ret         = HAL_RET_OK;

    if (!req.has_key_or_handle()) {
        for (fp_port = 1; fp_port <= num_fp_ports();
                              ++fp_port) {
            populate_port_info_response(fp_port, rsp);
        }
        return ret;
    }

    // TODO handle case?
    fp_port = req.key_or_handle().port_id();

    populate_port_info_response(fp_port, rsp);

    return ret;
}

static void*
linkmgr_aacs_start (void* ctxt)
{
    sdk::linkmgr::serdes_fns.serdes_aacs_start(*(int*)ctxt);
    return NULL;
}

static hal_ret_t
start_aacs_server(int port)
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
stop_aacs_server(void)
{
}

hal_ret_t
linkmgr_generic_debug_opn(GenericOpnRequest& req, GenericOpnResponse *resp)
{
    port_t        *pi_p        = NULL;
    port_args_t   port_args    = { 0 };
    hal_ret_t     ret          = HAL_RET_OK;
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
    int         aacs_server_port = 0;

    sdk::linkmgr::port_args_init(&port_args);
    kh::PortKeyHandle key_handle;

    switch (req.opn()) {
        case 0:
            // port disable

            port_id = req.val1();
            key_handle.set_port_id(port_id);

            HAL_TRACE_DEBUG("port_disable for port {}", port_id);

            pi_p = port_lookup_key_or_handle(key_handle);
            if (!pi_p) {
                HAL_TRACE_ERR("failed to find port id {}",
                               port_id);
                return HAL_RET_PORT_NOT_FOUND;
            }

            sdk_ret = sdk::linkmgr::port_get(pi_p->pd_p, &port_args);
            if (sdk_ret != SDK_RET_OK) {
                HAL_TRACE_ERR("failed to get port pd, err: {}",
                               ret);
                return HAL_RET_ERR;
            }

            port_args.admin_state = port_admin_state_t::PORT_ADMIN_STATE_DOWN;

            sdk_ret = sdk::linkmgr::port_update(pi_p->pd_p, &port_args);
            if (sdk_ret != SDK_RET_OK) {
                HAL_TRACE_ERR("failed to update port pd, err: {}",
                               ret);
                return HAL_RET_ERR;
            }

            break;

        case 1:
            // port enable

            port_id = req.val1();
            key_handle.set_port_id(port_id);

            HAL_TRACE_DEBUG("port_enable for port {}", port_id);

            pi_p = port_lookup_key_or_handle(key_handle);
            if (!pi_p) {
                HAL_TRACE_ERR("failed to find port id {}",
                               port_id);
                return HAL_RET_PORT_NOT_FOUND;
            }

            sdk_ret = sdk::linkmgr::port_get(pi_p->pd_p, &port_args);
            if (sdk_ret != SDK_RET_OK) {
                HAL_TRACE_ERR("failed to get port pd, err: {}",
                               ret);
                return HAL_RET_ERR;
            }

            port_args.admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;

            sdk_ret = sdk::linkmgr::port_update(pi_p->pd_p, &port_args);
            if (sdk_ret != SDK_RET_OK) {
                HAL_TRACE_ERR("failed to update port pd, err: {}",
                               ret);
                return HAL_RET_ERR;
            }

            break;

        case 2:
            mac_port_num = req.val1();
            speed        = req.val2();
            num_lanes    = req.val3();

            HAL_TRACE_DEBUG("mac_cfg mac_port: {}, speed: {}, num_lanes: {}",
                            mac_port_num, speed, num_lanes);

            mac_info_t mac_info;
            memset(&mac_info, 0, sizeof(mac_info_t));

            // TODO
            mac_info.mac_id    = mac_port_num / 4;
            mac_info.mac_ch    = mac_port_num % 4;
            mac_info.speed     = speed;
            mac_info.num_lanes = num_lanes;

            sdk::linkmgr::mac_fns.mac_cfg(&mac_info);
            break;

        case 3:
            mac_port_num = req.val1();
            speed        = req.val2();
            num_lanes    = req.val3();
            enable       = req.val4();

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
            reset        = req.val4();

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
            enable    = req.val2();

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
            enable    = req.val2();
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

            cap_mx_mac_stat(0 /*chip_id*/, mac_inst, mac_ch, 0);
            break;

        case 19:
            mac_inst = req.val1();
            mac_ch   = req.val2();
            enable   = req.val3();

            HAL_TRACE_DEBUG("mac_lpbk mac_inst: {}, mac_ch: {}, enable: {}",
                            mac_inst, mac_ch, enable);

            // TODO
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
