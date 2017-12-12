// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

// ----------------------------------------------------------------------------
// PI implementation for port service
// ----------------------------------------------------------------------------

#include "grpc++/grpc++.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/hal_cfg.hpp"

#include "nic/utils/ht/ht.hpp"
#include "nic/utils/list/list.hpp"

#include "nic/hal/periodic/periodic.hpp"
#include "linkmgr_src.hpp"
#include "linkmgr_svc.hpp"
#include "linkmgr_state.hpp"
#include "linkmgr_pd.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using boost::property_tree::ptree;

using linkmgr::pd::port_args_pd_t;
using hal::cfg_op_ctxt_t;
using hal::dhl_entry_t;
using hal::utils::dllist_add;
using hal::utils::dllist_reset;

namespace linkmgr {

hal::utils::thread  *g_linkmgr_threads[hal::HAL_THREAD_ID_MAX];
class linkmgr_state *g_linkmgr_state;
linkmgr_cfg_t       linkmgr_cfg;

hal::utils::thread*
current_thread()
{
    return hal::utils::thread::current_thread();
}

bool hw_access_mock_mode()
{
    return g_linkmgr_state->catalog()->access_mock_mode();
}

hal::utils::platform_type_t platform_type()
{
    return g_linkmgr_state->catalog()->platform_type();
}

hal_ret_t
linkmgr_uplink_create(uint32_t uplink_port,
                      hal::utils::catalog *catalog_p)
{
    hal_ret_t       ret = HAL_RET_OK;
    PortSpec        spec;
    PortResponse    response;

    // TODO
    int vrf_id = catalog_p->vrf_id();

    hal::utils::catalog_uplink_port_t *catalog_uplink_port_p =
                                      catalog_p->uplink_port(uplink_port);

    spec.mutable_key_or_handle()->set_port_id(uplink_port);
    spec.mutable_meta()->set_vrf_id(vrf_id);
    spec.set_port_speed(catalog_uplink_port_p->speed);
    spec.set_num_lanes(catalog_uplink_port_p->num_lanes);
    spec.set_port_type(catalog_uplink_port_p->type);

    hal::utils::catalog_asic_port_t *catalog_asic_port_p =
                                    catalog_p->asic_port(uplink_port);

    spec.set_mac_id(catalog_asic_port_p->mac_id);
    spec.set_mac_ch(catalog_asic_port_p->mac_ch);

    if (catalog_uplink_port_p->enabled == true) {
        spec.set_admin_state(::port::PORT_ADMIN_STATE_UP);
    }

    HAL_TRACE_DEBUG("{}. creating uplink port {}", __FUNCTION__, uplink_port);

    linkmgr::g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);

    ret = linkmgr::port_create(spec, &response);

    linkmgr::g_linkmgr_state->cfg_db_close();

    return ret;
}

//------------------------------------------------------------------------------
// create uplink ports in the catalog file
//------------------------------------------------------------------------------
hal_ret_t
linkmgr_uplinks_create(hal::utils::catalog *catalog_p)
{
    uint32_t  uplink_port = 0;
    hal_ret_t ret = HAL_RET_OK;

    if (NULL == catalog_p) {
        HAL_TRACE_ERR("{}: catalog db NULL", __FUNCTION__);
        return HAL_RET_ERR;
    }

    for (uplink_port = 1; uplink_port <= catalog_p->num_uplink_ports();
                                                      ++uplink_port) {
        ret = linkmgr_uplink_create(uplink_port, catalog_p);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}: Error creating uplink port {}",
                          __FUNCTION__,  uplink_port);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// parse the catalog file and populate catalog DB
//------------------------------------------------------------------------------
hal::utils::catalog*
linkmgr_catalog_init(std::string catalog_file)
{
    char  *cfg_path = NULL;

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        catalog_file = std::string(cfg_path) + "/" + catalog_file;
    }

    return hal::utils::catalog::factory(catalog_file);
}

//------------------------------------------------------------------------------
//  spawn and setup all the HAL threads
//------------------------------------------------------------------------------
static hal_ret_t
linkmgr_thread_init (void)
{
    int                  rv, thread_prio;
    struct sched_param   sched_param = { 0 };
    pthread_attr_t       attr;
    cpu_set_t            cpus;
    int                  thread_id = 0;

    // spawn data core threads and pin them to their cores
    thread_prio = sched_get_priority_max(SCHED_FIFO);
    assert(thread_prio >= 0);

    // spawn periodic thread that does background tasks
    thread_id = hal::HAL_THREAD_ID_PERIODIC;
    g_linkmgr_threads[thread_id] =
        hal::utils::thread::factory(
                        std::string("periodic-thread").c_str(),
                        thread_id,
                        HAL_CONTROL_CORE_ID,
                        hal::periodic::periodic_thread_start,
                        thread_prio - 1, SCHED_RR, true);
    HAL_ABORT(g_linkmgr_threads[thread_id] != NULL);
    g_linkmgr_threads[thread_id]->start(g_linkmgr_threads[thread_id]);

    // make the current thread, main hal config thread (also a real-time thread)
    rv = pthread_attr_init(&attr);
    if (rv != 0) {
        HAL_TRACE_ERR("pthread_attr_init failure, err : {}", rv);
        return HAL_RET_ERR;
    }

    // set core affinity
    CPU_ZERO(&cpus);
    CPU_SET(HAL_CONTROL_CORE_ID, &cpus);
    rv = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
    if (rv != 0) {
        HAL_TRACE_ERR("pthread_attr_setaffinity_np failure, err : {}", rv);
        return HAL_RET_ERR;
    }

    // create a thread object for this main thread
    thread_id = hal::HAL_THREAD_ID_CFG;
    g_linkmgr_threads[thread_id] =
        hal::utils::thread::factory(
                        std::string("cfg-thread").c_str(),
                        thread_id,
                        HAL_CONTROL_CORE_ID,
                        hal::utils::thread::dummy_entry_func,
                        sched_param.sched_priority, SCHED_RR, true);
    g_linkmgr_threads[thread_id]->set_data(g_linkmgr_threads[thread_id]);
    g_linkmgr_threads[thread_id]->set_pthread_id(pthread_self());
    g_linkmgr_threads[thread_id]->set_running(true);

    return HAL_RET_OK;
}

void
svc_reg (const std::string& server_addr)
{
    PortServiceImpl   port_svc;
    ServerBuilder     server_builder;

    HAL_TRACE_DEBUG("Bringing gRPC server for all API services ...");

    // listen on the given address (no authentication)
    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    // register all services
    server_builder.RegisterService(&port_svc);

    HAL_TRACE_DEBUG("gRPC server listening on ... {}", server_addr.c_str());

    // assemble the server
    std::unique_ptr<Server> server(server_builder.BuildAndStart());

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

    HAL_TRACE_DEBUG("{}: cfg file {}", __FUNCTION__, cfgfile);

    std::ifstream json_cfg(cfgfile);

    read_json(json_cfg, pt);

    try {
		std::string platform_type = pt.get<std::string>("platform_type");

        linkmgr_cfg->platform_type =
            hal::utils::catalog::catalog_platform_type_to_platform_type(platform_type);

        linkmgr_cfg->grpc_port = pt.get<std::string>("sw.grpc_port");

        if (getenv("HAL_GRPC_PORT")) {
            linkmgr_cfg->grpc_port = getenv("HAL_GRPC_PORT");
            HAL_TRACE_DEBUG("Overriding GRPC Port to : {}", linkmgr_cfg->grpc_port);
        }
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

hal_ret_t
linkmgr_init()
{
    hal_ret_t    ret          = HAL_RET_OK;
    std::string  catalog_file = "catalog.json";
    std::string  cfg_file     = "linkmgr.json";
    char         *cfg_path    = NULL;

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        cfg_file = std::string(cfg_path) + "/" + cfg_file;
    }

    linkmgr_parse_cfg(cfg_file.c_str(), &linkmgr_cfg);

    g_linkmgr_state = linkmgr_state::factory();
    HAL_ASSERT_RETURN((g_linkmgr_state != NULL), HAL_RET_ERR);

    hal::utils::catalog *catalog = linkmgr_catalog_init(catalog_file);
    HAL_ASSERT_RETURN((catalog != NULL), HAL_RET_ERR);

    // store the catalog in global hal state
    g_linkmgr_state->set_catalog(catalog);

    ret = linkmgr::pd::linkmgr_init_pd();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} pd init failed", __FUNCTION__);
        return ret;
    }

    ret = linkmgr_thread_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} thread init failed", __FUNCTION__);
        return ret;
    }

    // must be done after pd init
    ret = linkmgr_uplinks_create(catalog);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} uplinks create failed", __FUNCTION__);
    }

    // register for all gRPC services
    svc_reg(std::string("localhost:") + linkmgr_cfg.grpc_port);

    return ret;
}

hal_ret_t
port_event_timer(void *ctxt)
{
    return linkmgr::pd::port_event_timer_pd(ctxt);
}

hal_ret_t
port_event_enable(void *ctxt)
{
    return linkmgr::pd::port_event_enable_pd(ctxt);
}

hal_ret_t
port_event_disable(void *ctxt)
{
    return linkmgr::pd::port_event_disable_pd(ctxt);
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
    return hal::utils::hash_algo::fnv_hash(key, sizeof(port_num_t)) % ht_size;
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
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("{}:adding to port id hash table",
                    __FUNCTION__);
    // allocate an entry to establish mapping from port id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_linkmgr_state->
                        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from port num to its handle
    entry->handle_id = handle;
    ret = g_linkmgr_state->port_id_ht()->insert_with_key(&pi_p->port_num,
                                                       entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to add port num to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_linkmgr_state->hal_handle_id_ht_entry_slab()->free(entry);
    }

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
    if (!spec.has_meta()) {
        HAL_TRACE_ERR("{}:no meta",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    std::string str;

    // must have key-handle set
    str = "key_or_handle";
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}:{} not set in request",
                      __FUNCTION__, str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // port type
    str = "port type";
    if (spec.port_type() == ::port::PORT_TYPE_NONE) {
        HAL_TRACE_ERR("{}:{} not set in request",
                      __FUNCTION__, str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // port speed
    str = "port speed";
    if (spec.port_speed() == ::port::PORT_SPEED_NONE) {
        HAL_TRACE_ERR("{}:{} not set in request",
                      __FUNCTION__, str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_SPEED_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // number of lanes for the port
    str = "number of lanes";
    if (spec.num_lanes() == 0) {
        HAL_TRACE_ERR("{}:{} not set in request",
                      __FUNCTION__, str.c_str());
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
    hal_ret_t                ret = HAL_RET_OK;
    port_args_pd_t           port_args_pd = { 0 };
    dllist_ctxt_t            *lnode = NULL;
    dhl_entry_t              *dhl_entry = NULL;
    port_t                   *pi_p = NULL;
    port_create_app_ctxt_t   *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (port_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    pi_p = (port_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("{}:port_num:{}:create add CB.",
                    __FUNCTION__, pi_p->port_num);

    // PD Call to allocate PD resources and HW programming
    pd::port_args_init_pd(&port_args_pd);

    port_args_pd.port_type   = app_ctxt->port_type;
    port_args_pd.admin_state = app_ctxt->admin_state;
    port_args_pd.port_speed  = app_ctxt->port_speed;
    port_args_pd.mac_id      = app_ctxt->mac_id;
    port_args_pd.mac_ch      = app_ctxt->mac_ch;
    port_args_pd.num_lanes   = app_ctxt->num_lanes;

    pi_p->pd_p = pd::port_create_pd(&port_args_pd);
    if (NULL == pi_p->pd_p) {
        HAL_TRACE_ERR("{}:failed to create port pd, err : {}",
                __FUNCTION__, ret);
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
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle_id = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:port_num:{}:create commit CB.",
                    __FUNCTION__, pi_p->port_num);

    // Add to port id hash table
    ret = port_add_to_db(pi_p, hal_handle_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to add port {} to db, err : {}",
                __FUNCTION__, pi_p->port_num, ret);
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
    hal_ret_t        ret = HAL_RET_OK;
    port_args_pd_t   port_args_pd = { 0 };
    dhl_entry_t      *dhl_entry = NULL;
    port_t           *pi_p = NULL;
    hal_handle_t     hal_handle_id = 0;
    dllist_ctxt_t    *lnode = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle_id = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:port_num:{}:create abort CB",
                    __FUNCTION__, pi_p->port_num);

    // delete call to PD
    if (pi_p->pd_p) {
        pd::port_args_init_pd(&port_args_pd);
        port_args_pd.pd_p = pi_p->pd_p;
        ret = pd::port_delete_pd(&port_args_pd);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}:failed to delete port pd, err : {}",
                          __FUNCTION__, ret);
        }
    }

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
    hal_ret_t      ret = HAL_RET_OK;
    port_t         *pi_p = NULL;
    port_t         *pi_p1 = NULL;
    dhl_entry_t    dhl_entry = { 0 };
    cfg_op_ctxt_t  cfg_ctxt = { 0 };
    port_create_app_ctxt_t   app_ctxt;
    memset (&app_ctxt, 0, sizeof(port_create_app_ctxt_t));

    hal::hal_api_trace(" API Begin: port create ");
    HAL_TRACE_DEBUG("{}:port create with id:{}", __FUNCTION__,
                    spec.key_or_handle().port_id());

    // check if port exists already, and reject if one is found
    if (find_port_by_id(spec.key_or_handle().port_id())) {
        HAL_TRACE_ERR("{}:failed to create a port, "
                      "port {} exists already", __FUNCTION__,
                      spec.key_or_handle().port_id());
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_ENTRY_EXISTS;
    }

    // validate the request message
    ret = validate_port_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("{}:validation Failed. ret: {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // instantiate the port
    pi_p = port_alloc_init();
    if (pi_p == NULL) {
        HAL_TRACE_ERR("{}:unable to allocate handle/memory ret: {}",
                      __FUNCTION__, ret);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    pi_p->hal_handle_id = hal_handle_alloc(hal::HAL_OBJ_ID_PORT);

    if (pi_p->hal_handle_id == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: failed to alloc handle for port id {}",
                      __FUNCTION__, spec.key_or_handle().port_id());
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
        HAL_TRACE_ERR("{}:{} not set in update request",
                      __FUNCTION__, str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // port type must not be set in update
    str = "port type";
    if (spec.port_type() != ::port::PORT_TYPE_NONE) {
        HAL_TRACE_ERR("{}:{} set in update request",
                      __FUNCTION__, str.c_str());
        rsp->set_api_status(types::API_STATUS_PORT_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // number of lanes must not be set in update
    str = "number of lanes";
    if (spec.num_lanes() != 0) {
        HAL_TRACE_ERR("{}:{} set in update request",
                      __FUNCTION__, str.c_str());
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
// check for changes in port update
//------------------------------------------------------------------------------
hal_ret_t
port_update_check_for_change (PortSpec& spec,
                              port_t *pi_p,
                              port_update_app_ctxt_t *app_ctxt,
                              bool *has_changed)
{
    hal_ret_t       ret = HAL_RET_OK;
    port_args_pd_t  port_args_pd = { 0 };

    *has_changed = false;

    pd::port_args_init_pd(&port_args_pd);
    port_args_pd.pd_p = pi_p->pd_p;

    // check if port speed is set in update request
    if (spec.port_speed() != ::port::PORT_SPEED_NONE) {
        port_args_pd.port_speed  = spec.port_speed();
        *has_changed = pd::port_has_speed_changed_pd(&port_args_pd);
    }

    // check if admin state is set in update request
    if (spec.admin_state() != ::port::PORT_ADMIN_STATE_NONE) {
        port_args_pd.admin_state  = spec.admin_state();
        *has_changed = pd::port_has_admin_state_changed_pd(&port_args_pd);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
port_make_clone (port_t *pi_p, port_t **pi_clone_p)
{

    *pi_clone_p = port_alloc_init();

    memcpy(*pi_clone_p, pi_p, sizeof(port_t));

    (*pi_clone_p)->pd_p = pd::port_make_clone_pd(pi_p->pd_p);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
//------------------------------------------------------------------------------
hal_ret_t
port_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                ret = HAL_RET_OK;
    port_args_pd_t           port_args_pd = { 0 };
    dllist_ctxt_t            *lnode = NULL;
    dhl_entry_t              *dhl_entry = NULL;
    port_t                   *pi_p = NULL;
    port_update_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    app_ctxt = (port_create_app_ctxt_t *)cfg_ctxt->app_ctxt;
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // send the cloned obj to PD for update
    pi_p = (port_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("{}: update upd cb {}.",
                    __FUNCTION__, pi_p->port_num);

    // 1. PD Call to allocate PD resources and HW programming
    pd::port_args_init_pd(&port_args_pd);

    port_args_pd.pd_p        = pi_p->pd_p;
    port_args_pd.port_type   = app_ctxt->port_type;
    port_args_pd.admin_state = app_ctxt->admin_state;
    port_args_pd.port_speed  = app_ctxt->port_speed;
    port_args_pd.mac_id      = app_ctxt->mac_id;
    port_args_pd.mac_ch      = app_ctxt->mac_ch;
    port_args_pd.num_lanes   = app_ctxt->num_lanes;

    ret = pd::port_update_pd(&port_args_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to update port pd, err : {}",
                      __FUNCTION__, ret);
    }

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
    hal_ret_t        ret = HAL_RET_OK;
    port_args_pd_t   port_args_pd = { 0 };
    dllist_ctxt_t    *lnode = NULL;
    dhl_entry_t      *dhl_entry = NULL;
    port_t           *pi_p = NULL, *pi_clone_p = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // send the original object to PD to free the memory
    pi_p = (port_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("{}:update commit CB {}.",
                    __FUNCTION__, pi_p->port_num);
    printf("Original: %p, Clone: %p\n", pi_p, pi_clone_p);

    // Free PD
    pd::port_args_init_pd(&port_args_pd);
    port_args_pd.pd_p = pi_p->pd_p;
    ret = pd::port_mem_free_pd(&port_args_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to free original port pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    port_free(pi_p);
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
    port_args_pd_t   port_args_pd = { 0 };
    dllist_ctxt_t    *lnode = NULL;
    dhl_entry_t      *dhl_entry = NULL;
    port_t           *pi_p = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // send the cloned object to PD to be freed
    pi_p = (port_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("{}:update abort CB {}.",
                    __FUNCTION__, pi_p->port_num);

    // Free PD
    pd::port_args_init_pd(&port_args_pd);
    port_args_pd.pd_p = pi_p->pd_p;
    ret = pd::port_mem_free_pd(&port_args_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to delete port pd, err : {}",
                      __FUNCTION__, ret);
    }

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
    bool                       has_changed = false;
    port_update_app_ctxt_t     app_ctxt;

    hal::hal_api_trace(" API Begin: port update ");

    memset (&app_ctxt, 0, sizeof(port_create_app_ctxt_t));

    // validate the request message
    ret = validate_port_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:port update validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    pi_p = port_lookup_key_or_handle(kh);
    if (!pi_p) {
        HAL_TRACE_ERR("{}:failed to find port id {}, handle {}",
                      __FUNCTION__, kh.port_id(), kh.port_handle());
        ret = HAL_RET_PORT_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("{}: port update for port id {}, handle {}",
                    __FUNCTION__, kh.port_id(), kh.port_handle());

    // form ctxt and call infra add
    app_ctxt.port_type   = spec.port_type();
    app_ctxt.admin_state = spec.admin_state();
    app_ctxt.port_speed  = spec.port_speed();
    app_ctxt.mac_id      = spec.mac_id();
    app_ctxt.mac_ch      = spec.mac_ch();
    app_ctxt.num_lanes   = spec.num_lanes();

    // Check for changes
    ret = port_update_check_for_change(spec, pi_p, &app_ctxt, &has_changed);
    if (ret != HAL_RET_OK || !has_changed) {
        HAL_TRACE_ERR("{}:no change in port update: noop", __FUNCTION__);
        goto end;
    }

    port_make_clone(pi_p, (port_t **)&dhl_entry.cloned_obj);

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
        HAL_TRACE_ERR("{}:spec has no key or handle", __FUNCTION__);
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

    HAL_TRACE_DEBUG("{}:removing from port id hash table", __FUNCTION__);

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
    port_args_pd_t   port_args_pd = { 0 };
    dllist_ctxt_t    *lnode = NULL;
    dhl_entry_t      *dhl_entry = NULL;
    port_t           *pi_p = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("{}:delete del CB {}",
                    __FUNCTION__, pi_p->port_num);

    // 1. PD Call to allocate PD resources and HW programming
    pd::port_args_init_pd(&port_args_pd);

    port_args_pd.pd_p = pi_p->pd_p;

    ret = pd::port_delete_pd(&port_args_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to delete port pd, err : {}", 
                      __FUNCTION__, ret);
    }

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
        HAL_TRACE_ERR("{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    pi_p = (port_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:delete commit CB {}",
                    __FUNCTION__, pi_p->port_num);

    // a. Remove from port id hash table
    ret = port_del_from_db(pi_p);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to del port {} from db, err : {}", 
                      __FUNCTION__, pi_p->port_num, ret);
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
        HAL_TRACE_ERR("{}:port delete request validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    pi_p = port_lookup_key_or_handle(kh);
    if (pi_p == NULL) {
        HAL_TRACE_ERR("{}:failed to find port, id {}, handle {}",
                      __FUNCTION__, kh.port_id(), kh.port_handle());
        ret = HAL_RET_PORT_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("{}: port delete for id {}",
                    __FUNCTION__, pi_p->port_num);

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
    rsp->add_api_status(hal::hal_prepare_rsp(ret));

    hal::hal_api_trace(" API End: port delete ");
    return ret;
}

//------------------------------------------------------------------------------
// process a port get request
//------------------------------------------------------------------------------
hal_ret_t
port_get (PortGetRequest& req, PortGetResponse *rsp)
{
    port_t          *pi_p = NULL;
    PortSpec        *spec = NULL;
    hal_ret_t       ret = HAL_RET_OK;
    port_args_pd_t  port_args_pd = { 0 };

    hal::hal_api_trace(" API Begin: port get ");

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_PORT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    pi_p = port_lookup_key_or_handle(req.key_or_handle());
    if (!pi_p) {
        rsp->set_api_status(types::API_STATUS_PORT_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    // fill in the config spec of this port
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_port_id(pi_p->port_num);

    // 1. PD Call to get PD resources
    pd::port_args_init_pd(&port_args_pd);

    port_args_pd.pd_p = pi_p->pd_p;

    ret = pd::port_get_pd(&port_args_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to get port pd, err : {}",
                      __FUNCTION__, ret);
    }

    if (ret == HAL_RET_OK) {
        spec->set_port_type   (port_args_pd.port_type);
        spec->set_port_speed  (port_args_pd.port_speed);
        spec->set_admin_state (port_args_pd.admin_state);
        spec->set_mac_id      (port_args_pd.mac_id);
        spec->set_mac_ch      (port_args_pd.mac_ch);
        spec->set_num_lanes   (port_args_pd.num_lanes);

        rsp->set_status       (port_args_pd.oper_status);
    }

    rsp->set_api_status(hal::hal_prepare_rsp(ret));

    hal::hal_api_trace(" API End: port get ");

    return HAL_RET_OK;
}

}    // namespace linkmgr
