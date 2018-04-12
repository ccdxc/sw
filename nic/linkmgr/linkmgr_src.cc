//----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// PI implementation for port service
//----------------------------------------------------------------------------

#include "grpc++/grpc++.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/hal_cfg.hpp"
#include "sdk/pal.hpp"
#include "sdk/ht.hpp"
#include "sdk/list.hpp"
#include "sdk/utils.hpp"
#include "linkmgr_src.hpp"
#include "linkmgr_svc.hpp"
#include "linkmgr_state.hpp"
#include "nic/linkmgr/utils.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using boost::property_tree::ptree;
using hal::cfg_op_ctxt_t;
using hal::dhl_entry_t;
using sdk::lib::dllist_add;
using sdk::lib::dllist_reset;
using sdk::types::platform_type_t;
using sdk::linkmgr::port_args_t;
using sdk::SDK_RET_OK;

namespace linkmgr {

class linkmgr_state *g_linkmgr_state;
linkmgr_cfg_t       linkmgr_cfg;

sdk::lib::thread *
current_thread (void)
{
    return sdk::lib::thread::current_thread();
}

static bool
hw_mock (void)
{
    return linkmgr_cfg.hw_mock;
}

static sdk::lib::catalog*
catalog (void)
{
    return g_linkmgr_state->catalog();
}

static uint32_t
num_uplink_ports (void)
{
    return catalog()->num_uplink_ports();
}

static port_speed_t
port_speed (uint32_t port)
{
    return catalog()->port_speed(port);
}

static port_type_t
port_type (uint32_t port)
{
    return catalog()->port_type(port);
}

static uint32_t
num_lanes (uint32_t port)
{
    return catalog()->num_lanes(port);
}

static bool
enabled (uint32_t port)
{
    return catalog()->enabled(port);
}

static uint32_t
mac_id (uint32_t port, uint32_t lane)
{
    return catalog()->mac_id(port, lane);
}

static uint32_t
mac_ch (uint32_t port, uint32_t lane)
{
    return catalog()->mac_ch(port, lane);
}

static uint32_t
sbus_addr (uint32_t port, uint32_t lane)
{
    return catalog()->sbus_addr(port, lane);
}

static platform_type_t
platform_type (void)
{
    return linkmgr_cfg.platform_type;
}

static hal_ret_t
linkmgr_uplink_create(uint32_t uplink_port)
{
    hal_ret_t     ret  = HAL_RET_OK;
    PortSpec      spec;
    PortResponse  response;

    spec.mutable_key_or_handle()->set_port_id(uplink_port);
    spec.set_port_speed(hal::sdk_port_speed_to_port_speed_spec(port_speed(uplink_port)));
    spec.set_num_lanes(num_lanes(uplink_port));
    spec.set_port_type(hal::sdk_port_type_to_port_type_spec(port_type(uplink_port)));
    spec.set_mac_id(mac_id(uplink_port, 0));
    spec.set_mac_ch(mac_ch(uplink_port, 0));

    if (enabled(uplink_port) == true) {
        spec.set_admin_state(::port::PORT_ADMIN_STATE_UP);
    }

    HAL_TRACE_DEBUG("creating uplink port {}",  uplink_port);

    linkmgr::g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);

    ret = linkmgr::port_create(spec, &response);

    linkmgr::g_linkmgr_state->cfg_db_close();

    return ret;
}

//------------------------------------------------------------------------------
// create uplink ports in the catalog file
//------------------------------------------------------------------------------
static hal_ret_t
linkmgr_uplinks_create()
{
    uint32_t  uplink_port = 0;
    hal_ret_t ret = HAL_RET_OK;

    for (uplink_port = 1; uplink_port <= num_uplink_ports();
                                             ++uplink_port) {
        ret = linkmgr_uplink_create(uplink_port);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error creating uplink port {}",
                            uplink_port);
        }
    }

    return ret;
}

static void
svc_reg (const std::string& server_addr)
{
    PortServiceImpl   port_svc;
    ServerBuilder     server_builder;

    // listen on the given address (no authentication)
    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    // register all services
    server_builder.RegisterService(&port_svc);

    HAL_TRACE_DEBUG("gRPC server listening on ... {}", server_addr.c_str());

    // assemble the server
    std::unique_ptr<Server> server(server_builder.BuildAndStart());

    hal::utils::hal_logger()->flush();

    // wait for server to shutdown (some other thread must be responsible for
    // shutting down the server or else this call won't return)
    server->Wait();
}

//------------------------------------------------------------------------------
// parse configuration
//------------------------------------------------------------------------------
hal_ret_t
linkmgr_parse_cfg (const char *cfgfile, linkmgr_cfg_t *linkmgr_cfg)
{
    ptree             pt;
    std::string       sparam;

    if (!cfgfile) {
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("cfg file {}",  cfgfile);

    std::ifstream json_cfg(cfgfile);

    read_json(json_cfg, pt);

    try {
		std::string platform_type = pt.get<std::string>("platform_type");

        linkmgr_cfg->platform_type =
            sdk::lib::catalog::catalog_platform_type_to_platform_type(platform_type);

        linkmgr_cfg->grpc_port = pt.get<std::string>("sw.grpc_port");

        linkmgr_cfg->hw_mock = pt.get<bool>("hw_mock", false);

        if (getenv("HAL_GRPC_PORT")) {
            linkmgr_cfg->grpc_port = getenv("HAL_GRPC_PORT");
            HAL_TRACE_DEBUG("Overriding GRPC Port to {}", linkmgr_cfg->grpc_port);
        }
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

hal_ret_t
linkmgr_init (void)
{
    hal_ret_t                     ret_hal   = HAL_RET_OK;
    sdk_ret_t                     sdk_ret   = SDK_RET_OK;
    std::string                   cfg_file  = "linkmgr.json";
    char                          *cfg_path = NULL;
    sdk::lib::catalog             *catalog;
    sdk::linkmgr::linkmgr_cfg_t   sdk_cfg;

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        cfg_file = std::string(cfg_path) + "/" + cfg_file;
    } else {
        HAL_ASSERT(FALSE);
    }

    linkmgr_parse_cfg(cfg_file.c_str(), &linkmgr_cfg);

    g_linkmgr_state = linkmgr_state::factory();
    HAL_ASSERT_RETURN((g_linkmgr_state != NULL), HAL_RET_ERR);

    catalog =
        sdk::lib::catalog::factory(std::string(cfg_path) + "/catalog.json");
    HAL_ASSERT_RETURN((catalog != NULL), HAL_RET_ERR);

    // store the catalog in global hal state
    g_linkmgr_state->set_catalog(catalog);

    sdk_cfg.platform_type = linkmgr_cfg.platform_type;
    sdk_cfg.hw_mock = linkmgr_cfg.hw_mock;

    sdk_ret = sdk::linkmgr::linkmgr_init(&sdk_cfg);
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("linkmgr init failed");
        return HAL_RET_ERR;
    }

    if (sdk::lib::pal_init(platform_type()) != sdk::lib::PAL_RET_OK) {
        HAL_TRACE_ERR("pal init failed");
        return HAL_RET_ERR;
    }

    // must be done after sdk linkmgr init
    ret_hal = linkmgr_uplinks_create();
    if (ret_hal != HAL_RET_OK) {
        HAL_TRACE_ERR("uplinks create failed");
    }

    // start the linkmgr control thread
    sdk::linkmgr::linkmgr_event_wait();

    // register for all gRPC services
    svc_reg(std::string("localhost:") + linkmgr_cfg.grpc_port);

    return ret_hal;
}

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

    port_args.port_type   =
            hal::port_type_spec_to_sdk_port_type(app_ctxt->port_type);
    port_args.admin_state =
            hal::port_admin_st_spec_to_sdk_port_admin_st(app_ctxt->admin_state);
    port_args.port_speed  =
            hal::port_speed_spec_to_sdk_port_speed(app_ctxt->port_speed);
    port_args.fec_type =
            hal::port_fec_type_spec_to_sdk_port_fec_type(app_ctxt->fec_type);
    port_args.mac_id      = app_ctxt->mac_id;
    port_args.mac_ch      = app_ctxt->mac_ch;
    port_args.num_lanes   = app_ctxt->num_lanes;
    port_args.auto_neg_enable = app_ctxt->auto_neg_enable;
    port_args.debounce_time   = app_ctxt->debounce_time;
    memcpy(port_args.sbus_addr, app_ctxt->sbus_addr,
                                PORT_MAX_LANES * sizeof(uint32_t));

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
    app_ctxt.port_type   = spec.port_type();
    app_ctxt.admin_state = spec.admin_state();
    app_ctxt.port_speed  = spec.port_speed();
    app_ctxt.mac_id      = spec.mac_id();
    app_ctxt.mac_ch      = spec.mac_ch();
    app_ctxt.num_lanes   = spec.num_lanes();
    app_ctxt.fec_type    = spec.fec_type();
    app_ctxt.auto_neg_enable = spec.auto_neg_enable();
    app_ctxt.debounce_time   = spec.debounce_time();

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
                hal::port_admin_st_spec_to_sdk_port_admin_st(app_ctxt->admin_state);
    port_args.port_speed  =
                hal::port_speed_spec_to_sdk_port_speed(app_ctxt->port_speed);
    port_args.fec_type =
                hal::port_fec_type_spec_to_sdk_port_fec_type(app_ctxt->fec_type);
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

//------------------------------------------------------------------------------
// process a port get request
//------------------------------------------------------------------------------
hal_ret_t
port_get (PortGetRequest& req, PortGetResponse *rsp)
{
    port_t       *pi_p     = NULL;
    PortSpec     *spec     = NULL;
    hal_ret_t    ret       = HAL_RET_OK;
    sdk_ret_t    sdk_ret   = SDK_RET_OK;
    port_args_t  port_args = { 0 };

    hal::hal_api_trace(" API Begin: port get ");

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_PORT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    pi_p = port_lookup_key_or_handle(req.key_or_handle());
    if (!pi_p) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_PORT_NOT_FOUND;
    }

    // fill in the config spec of this port
    spec = rsp->mutable_spec();
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
                (hal::sdk_port_type_to_port_type_spec(port_args.port_type));
        spec->set_port_speed
                (hal::sdk_port_speed_to_port_speed_spec(port_args.port_speed));
        spec->set_admin_state
                (hal::sdk_port_admin_st_to_port_admin_st_spec
                                        (port_args.admin_state));
        spec->set_fec_type
                (hal::sdk_port_fec_type_to_port_fec_type_spec
                                            (port_args.fec_type));
        rsp->mutable_status()->set_oper_status(
                (hal::sdk_port_oper_st_to_port_oper_st_spec
                                        (port_args.oper_status)));
        spec->set_mac_id    (port_args.mac_id);
        spec->set_mac_ch    (port_args.mac_ch);
        spec->set_num_lanes (port_args.num_lanes);
        spec->set_auto_neg_enable (port_args.auto_neg_enable);
        spec->set_debounce_time   (port_args.debounce_time);
    }

    rsp->set_api_status(hal::hal_prepare_rsp(ret));

    hal::hal_api_trace(" API End: port get ");

    return HAL_RET_OK;
}

}    // namespace linkmgr
