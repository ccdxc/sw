// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <iostream>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nic/hal/hal.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/hal/src/rdma.hpp"
#include "nic/hal/src/oif_list_mgr.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/include/fte.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/hal/plugins/plugins.hpp"

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/hal/src/proxy.hpp"

#include "nic/utils/catalog/catalog.hpp"

using port::PortSpec;
using port::PortResponse;

namespace hal {

// process globals
thread    *g_hal_threads[HAL_THREAD_ID_MAX];
uint64_t  g_hal_handle = 1;
bool      gl_super_user = false;

// TODO_CLEANUP: THIS DOESN'T BELONG HERE !!
LIFManager *g_lif_manager = nullptr;

// thread local variables
thread_local cfg_db_ctxt_t t_cfg_db_ctxt;

using boost::property_tree::ptree;

static void *
fte_pkt_loop_start (void *ctxt)
{
    HAL_THREAD_INIT(ctxt);

    thread *curr_thread = hal::utils::thread::current_thread();
    fte::fte_start(curr_thread->thread_id() - HAL_THREAD_ID_FTE_MIN);
    return NULL;
}

//------------------------------------------------------------------------------
// return current thread pointer, for gRPC threads curr_thread is not set,
// however, they are considered as cfg threads
//------------------------------------------------------------------------------
thread *
hal_get_current_thread (void)
{
    return  hal::utils::thread::current_thread();
}

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
    mem = g_hal_state->hal_handle_slab()->alloc();
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    handle = new(mem) hal_handle();
    // initialize the handle instance
    if (handle->init(obj_id) == false) {
        handle->~hal_handle();
        g_hal_state->hal_handle_slab()->free(handle);
        return NULL;
    }

    return handle;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_handle::~hal_handle()
{
    //HAL_SPINLOCK_DESTROY(&slock_);
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
    g_hal_state->cfg_db()->runlock();

    // acquire the write lock
    g_hal_state->cfg_db()->wlock();

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
    g_hal_state->cfg_db()->wunlock();

    // reacquire the read lock again
    g_hal_state->cfg_db()->rlock();

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
    g_hal_state->cfg_db()->runlock();

    // acquire the write lock
    g_hal_state->cfg_db()->wlock();

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
    g_hal_state->cfg_db()->wunlock();

    // reacquire the read lock again
    g_hal_state->cfg_db()->rlock();

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
    g_hal_state->cfg_db()->runlock();

    // acquire the write lock
    g_hal_state->cfg_db()->wlock();

    // invoke the callbacks passed
    ret = act_on_obj_(ctxt, del_cb, commit_cb, abort_cb);

    // call cleanup callback to either free the software state of original
    // object of cloned object (based on whether modify was successful or not)
    cleanup_cb(ctxt);

    // release the write lock
    g_hal_state->cfg_db()->wunlock();

    // reacquire the read lock again
    g_hal_state->cfg_db()->rlock();

    return ret;
}

//------------------------------------------------------------------------------
// allocate a handle for an object instance
//------------------------------------------------------------------------------
hal_handle_t
hal_handle_alloc (hal_obj_id_t obj_id)
{
    hal_ret_t                ret;
    hal_handle_t             handle_id;
    hal_handle               *handle;
    hal_handle_ht_entry_t    *entry;

    // allocate an entry to create mapping from handle-id to its handle obj
    entry =
        (hal_handle_ht_entry_t *)g_hal_state->hal_handle_ht_entry_slab()->alloc();
    if (entry == NULL) {
        HAL_TRACE_ERR("Failed to allocate hal handle ht entry");
        return HAL_HANDLE_INVALID;
    }

    // allocate hal handle object itself
    handle = hal_handle::factory(obj_id);
    if (handle == NULL) {
        HAL_TRACE_ERR("Failed to allocate handle");
        g_hal_state->hal_handle_ht_entry_slab()->free(entry);
        return HAL_HANDLE_INVALID;
    }
    // allocate unique handle id
    handle_id = HAL_ATOMIC_INC_UINT32(&g_hal_handle, 1);
    handle->set_handle_id(handle_id);

    // prepare the entry to be inserted
    entry->handle = handle;
    entry->ht_ctxt.reset();
    ret = g_hal_state->hal_handle_id_ht()->insert_with_key(&handle_id,
                                                           entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add handle id {} to handle obj", handle_id);
        g_hal_state->hal_handle_ht_entry_slab()->free(entry);
        handle->~hal_handle();
        g_hal_state->hal_handle_slab()->free(handle);
        return HAL_HANDLE_INVALID;
    }

    HAL_TRACE_DEBUG("Assigned hal_handle:{} for an object of obj_id:{}", 
                    handle_id, obj_id);
    return handle_id;
}

//------------------------------------------------------------------------------
// return a hal handle back so it can be reallocated for another object
//------------------------------------------------------------------------------
void
hal_handle_free (hal_handle_t handle_id)
{
    hal_handle_ht_entry_t    *entry;

    entry = (hal_handle_ht_entry_t *)
                g_hal_state->hal_handle_id_ht()->remove(&handle_id);
    if (entry == NULL) {
        return;
    }
    if (entry->handle) {
        entry->handle->~hal_handle();
        g_hal_state->hal_handle_slab()->free(entry->handle);
    }
    g_hal_state->hal_handle_ht_entry_slab()->free(entry);

    return;
}

//------------------------------------------------------------------------------
// given a handle id, return the corresponding handle object
//------------------------------------------------------------------------------
hal_handle *
hal_handle_get_from_handle_id (hal_handle_t handle_id)
{
    hal_handle_ht_entry_t    *entry;

    // lookup by handle id and get the handle class
    entry =
        (hal_handle_ht_entry_t *)g_hal_state->hal_handle_id_ht()->lookup(&handle_id);
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

    // lookup by handle id and get the handle class
    entry =
        (hal_handle_ht_entry_t *)g_hal_state->hal_handle_id_ht()->lookup(&handle_id);
    if (entry && entry->handle) {
        return entry->handle->get_obj();
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

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
static void
hal_sig_handler (int sig, siginfo_t *info, void *ptr)
{
    HAL_TRACE_DEBUG("HAL received signal {}", sig);

    switch (sig) {
    case SIGINT:
        HAL_GCOV_FLUSH();
        utils::hal_logger().flush();
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        utils::hal_logger().flush();
        break;

    case SIGHUP:
    case SIGQUIT:
    case SIGCHLD:
    case SIGURG:
    case SIGTERM:
    default:
        utils::hal_logger().flush();
        break;
    }
}

//------------------------------------------------------------------------------
// initialize all the signal handlers
// TODO: save old handlers and restore when signal happened
//------------------------------------------------------------------------------
static hal_ret_t
hal_sig_init (void)
{
    struct sigaction    act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = hal_sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGURG, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
//  spawn and setup all the HAL threads - both config and packet loop threads
//------------------------------------------------------------------------------
static hal_ret_t
hal_thread_init (void)
{
    uint32_t              tid, core_id;
    int                   rv, thread_prio;
    char                  thread_name[16];
    struct sched_param    sched_param = { 0 };
    pthread_attr_t        attr;
    cpu_set_t             cpus;

    // spawn data core threads and pin them to their cores
    thread_prio = sched_get_priority_max(SCHED_FIFO);
    assert(thread_prio >= 0);
    for (tid = HAL_THREAD_ID_FTE_MIN, core_id = 1;
         tid <= HAL_THREAD_ID_FTE_MAX;       // TODO: fix the env !!
         tid++, core_id++) {
        HAL_TRACE_DEBUG("Spawning FTE thread {}", tid);
        snprintf(thread_name, sizeof(thread_name), "fte-core-%u", core_id);
        g_hal_threads[tid] =
            thread::factory(static_cast<const char *>(thread_name), tid,
                            core_id, fte_pkt_loop_start,
                            thread_prio, SCHED_FIFO, false);
        HAL_ABORT(g_hal_threads[tid] != NULL);
    }

    // spawn periodic thread that does background tasks
    g_hal_threads[HAL_THREAD_ID_PERIODIC] =
        thread::factory(std::string("periodic-thread").c_str(),
                        HAL_THREAD_ID_PERIODIC,
                        HAL_CONTROL_CORE_ID,
                        hal::periodic::periodic_thread_start,
                        thread_prio - 1, SCHED_RR, true);
    HAL_ABORT(g_hal_threads[HAL_THREAD_ID_PERIODIC] != NULL);
    g_hal_threads[HAL_THREAD_ID_PERIODIC]->start(g_hal_threads[HAL_THREAD_ID_PERIODIC]);

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

    if (gl_super_user) {
        HAL_TRACE_DEBUG("Started by root, switching to real-time scheduling");
        sched_param.sched_priority = sched_get_priority_max(SCHED_RR);
        rv = sched_setscheduler(0, SCHED_RR, &sched_param);
        if (rv != 0) {
            HAL_TRACE_ERR("sched_setscheduler failure, err : {}", rv);
            return HAL_RET_ERR;
        }
    }

    // create a thread object for this main thread
    g_hal_threads[HAL_THREAD_ID_CFG] =
        thread::factory(std::string("cfg-thread").c_str(),
                        HAL_THREAD_ID_CFG,
                        HAL_CONTROL_CORE_ID,
                        thread::dummy_entry_func,
                        sched_param.sched_priority, SCHED_RR, true);
    g_hal_threads[HAL_THREAD_ID_CFG]->set_data(g_hal_threads[HAL_THREAD_ID_CFG]);
    g_hal_threads[HAL_THREAD_ID_CFG]->set_pthread_id(pthread_self());
    g_hal_threads[HAL_THREAD_ID_CFG]->set_running(true);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
//  uninit all  the HAL threads - both config and packet loop threads.
//------------------------------------------------------------------------------
static hal_ret_t
hal_thread_destroy (void)
{
    HAL_ABORT(g_hal_threads[HAL_THREAD_ID_PERIODIC] != NULL);
    g_hal_threads[HAL_THREAD_ID_PERIODIC]->stop();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// wait for all the HAL threads to be terminated and any other background
// activities
//------------------------------------------------------------------------------
hal_ret_t
hal_wait (void)
{
    int         rv;
    uint32_t    tid;

    for (tid = HAL_THREAD_ID_PERIODIC; tid < HAL_THREAD_ID_MAX; tid++) {
        if (g_hal_threads[tid]) {
            rv = pthread_join(g_hal_threads[tid]->pthread_id(), NULL);
            if (rv != 0) {
                HAL_TRACE_ERR("pthread_join failure, thread {}, err : {}",
                              g_hal_threads[tid]->name(), rv);
                return HAL_RET_ERR;
            }
        }
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// parse HAL .ini file
//------------------------------------------------------------------------------
hal_ret_t
hal_parse_ini (const char *inifile, hal_cfg_t *hal_cfg) 
{
    hal_ret_t           ret = HAL_RET_OK;
    std::string         line;
    std::ifstream       in(inifile);     

    if (!in) {
        HAL_TRACE_ERR("unable to open ini file ... "
                      "setting default forwarding mode");
        hal_cfg->forwarding_mode = "default";
        return HAL_RET_OK;
    }

    while (std::getline(in, line)) {
        std::string key = line.substr(0, line.find("="));
        std::string val = line.substr(line.find("=")+1, line.length()-1);
        // HAL_TRACE_DEBUG("key:{}, val:{}", key, val);

        if (key == "forwarding_mode") {
            if (val != "default" && val != "host-pinned" && val != "classic") {
                HAL_TRACE_ERR("Invalid forwarding mode: aborting ...");
                HAL_ABORT(0);
            }
            hal_cfg->forwarding_mode = val;
            HAL_TRACE_DEBUG("NIC forwarding mode: {}", val);
        }
    }

    in.close();

    return ret;
}

//------------------------------------------------------------------------------
// parse HAL configuration
//------------------------------------------------------------------------------
hal_ret_t
hal_parse_cfg (const char *cfgfile, hal_cfg_t *hal_cfg)
{
    ptree             pt;
    std::string       sparam;

    if (!cfgfile || !hal_cfg) {
        return HAL_RET_INVALID_ARG;
    }

    std::ifstream json_cfg(cfgfile);
    read_json(json_cfg, pt);
    try {
		std::string mode = pt.get<std::string>("mode");
        if (mode == "sim") {
            hal_cfg->sim = true;
        } else {
            hal_cfg->sim = false;
        }

        sparam = pt.get<std::string>("asic.name");
        strncpy(hal_cfg->asic_name, sparam.c_str(), HAL_MAX_NAME_STR);
        hal_cfg->loader_info_file =
                pt.get<std::string>("asic.loader_info_file");

        hal_cfg->grpc_port = pt.get<std::string>("sw.grpc_port");
        if (getenv("HAL_GRPC_PORT")) {
            hal_cfg->grpc_port = getenv("HAL_GRPC_PORT");
            HAL_TRACE_DEBUG("Overriding GRPC Port to : {}", hal_cfg->grpc_port);
        }
        sparam = pt.get<std::string>("sw.feature_set");
        strncpy(hal_cfg->feature_set, sparam.c_str(), HAL_MAX_NAME_STR);

#if 0
        hal_cfg->forwarding_mode = pt.get<std::string>("sw.forwarding_mode");
        HAL_TRACE_INFO("HAL Forwarding Mode: {}", hal_cfg->forwarding_mode);
        if (hal_cfg->forwarding_mode != "default" &&
            hal_cfg->forwarding_mode != "host-pinned" &&
            hal_cfg->forwarding_mode != "classic") {
            HAL_TRACE_ERR("Invalid Forwarding Mode: aborting...");
            HAL_ABORT(0);
        }
#endif
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

hal_ret_t
hal_uplink_create(uint32_t uplink_port,
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

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    ret = hal::port_create(spec, &response);

    hal::hal_cfg_db_close();

    return ret;
}

//------------------------------------------------------------------------------
// create uplink ports in the catalog file
//------------------------------------------------------------------------------
hal_ret_t
hal_uplinks_create(hal::utils::catalog *catalog_p)
{
    uint32_t  uplink_port = 0;
    hal_ret_t ret = HAL_RET_OK;

    if (NULL == catalog_p) {
        HAL_TRACE_ERR("{}: catalog db NULL", __FUNCTION__);
        return HAL_RET_ERR;
    }

    for (uplink_port = 1; uplink_port <= catalog_p->num_uplink_ports();
                                                      ++uplink_port) {
        ret = hal_uplink_create(uplink_port, catalog_p);
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
hal_catalog_init(std::string catalog_file)
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
// init function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_init (hal_cfg_t *hal_cfg)
{
    char         *user = NULL;
    std::string  catalog_file = "catalog.json";

    HAL_TRACE_DEBUG("Initializing HAL ...");

    // check to see if HAL is running with root permissions
    user = getenv("USER");
    if (user && !strcmp(user, "root")) {
        gl_super_user = true;
    }

    // install signal handlers
    hal_sig_init();

    // do memory related initialization
    HAL_ABORT(hal_mem_init() == HAL_RET_OK);

    // Initialize config parameters from the JSON file.
    HAL_ABORT(hal_cfg_init(hal_cfg) == HAL_RET_OK);

    // init fte and hal plugins
    hal::init_plugins(hal_cfg->forwarding_mode == "classic");

    // spawn all necessary PI threads
    HAL_ABORT(hal_thread_init() == HAL_RET_OK);
    HAL_TRACE_DEBUG("Spawned all HAL threads");

    // parse the hal catalog file
    hal::utils::catalog *catalog_p = hal_catalog_init(catalog_file);
    if (NULL == catalog_p) {
        HAL_TRACE_ERR("{}: Error creating catalog db",
                      __FUNCTION__);
    }

    // store the catalog in global hal state
    g_hal_state->set_catalog(catalog_p);

    // do platform dependent init
    HAL_ABORT(hal::pd::hal_pd_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Platform initialization done");

    // create uplink ports after PD init since HW needs to be programmed
    hal_uplinks_create(catalog_p);

    // TODO_CLEANUP: this doesn't belong here, why is this outside
    // hal_state ??? how it this special compared to other global state ??
    g_lif_manager = new LIFManager();

    // do rdma init
    HAL_ABORT(rdma_hal_init() == HAL_RET_OK);

    if(!getenv("CAPRI_MOCK_MODE")) {
        // start fte threads
        for (int tid = HAL_THREAD_ID_FTE_MIN; tid <= HAL_THREAD_ID_FTE_MAX; tid++) {
            g_hal_threads[tid]->start(g_hal_threads[tid]);
            break; // TODO(goli) only one FTE thread until driver supports multiple ARQs
        }
    }

    hal_proxy_svc_init();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// un init function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_destroy (void)
{
    // cancel  all necessary PI threads
    HAL_ABORT(hal_thread_destroy() == HAL_RET_OK);
    HAL_TRACE_DEBUG("Cancelled  all HAL threads");

    return HAL_RET_OK;

}

}    // namespace hal
