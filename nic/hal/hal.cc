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
#include "nic/hal/lib/hal_handle.hpp"

using port::PortSpec;
using port::PortResponse;

namespace hal {

namespace utils {
hal::utils::mem_mgr     g_hal_mem_mgr;
}

// process globals
thread   *g_hal_threads[HAL_THREAD_ID_MAX];
bool     gl_super_user = false;

// TODO_CLEANUP: THIS DOESN'T BELONG HERE !!
LIFManager *g_lif_manager = nullptr;

// thread local variables
thread_local cfg_db_ctxt_t t_cfg_db_ctxt;

using boost::property_tree::ptree;

thread*
current_thread()
{
    return hal::utils::thread::current_thread() ?
           hal::utils::thread::current_thread() :
           g_hal_threads[HAL_THREAD_ID_CFG];
}

static void *
fte_pkt_loop_start (void *ctxt)
{
    HAL_THREAD_INIT(ctxt);

    thread *curr_thread = hal::current_thread();
    thread_init_plugins(curr_thread->thread_id());
    fte::fte_start(curr_thread->thread_id() - HAL_THREAD_ID_FTE_MIN);
    thread_exit_plugins(curr_thread->thread_id());
    return NULL;
}

static void *
hal_periodic_loop_start (void *ctxt)
{
    // initialize timer wheel
    hal::periodic::periodic_thread_init(ctxt);

    // do any plugin-specific thread initialization
    thread_init_plugins(HAL_THREAD_ID_PERIODIC);

    // run main loop
    hal::periodic::periodic_thread_run(ctxt);

    // loop exited, do plugin-specific thread cleanup
    thread_exit_plugins(HAL_THREAD_ID_PERIODIC);

    return NULL;
}


//------------------------------------------------------------------------------
// return current thread pointer, for gRPC threads curr_thread is not set,
// however, they are considered as cfg threads
//------------------------------------------------------------------------------
thread *
hal_get_current_thread (void)
{
    return current_thread();
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
                        hal_periodic_loop_start,
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
                      "setting forwarding mode: smart-switch");
        hal_cfg->forwarding_mode = "smart-switch";
        return HAL_RET_OK;
    }

    while (std::getline(in, line)) {
        std::string key = line.substr(0, line.find("="));
        std::string val = line.substr(line.find("=")+1, line.length()-1);
        // HAL_TRACE_DEBUG("key:{}, val:{}", key, val);

        if (key == "forwarding_mode") {
            if (val != "smart-switch" && val != "smart-host-pinned" && val != "classic") {
                HAL_TRACE_ERR("Invalid forwarding mode:{}, aborting ...", val);
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

    // do platform dependent init
    HAL_ABORT(hal::pd::hal_pd_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Platform initialization done");

    // TODO_CLEANUP: this doesn't belong here, why is this outside
    // hal_state ??? how it this special compared to other global state ??
    g_lif_manager = new LIFManager();

    // Allocate LIF 0, so that we don't use it later
    int32_t hw_lif_id = g_lif_manager->LIFRangeAlloc(-1, 1);
    HAL_TRACE_DEBUG("Allocated hw_lif_id:{}", hw_lif_id);

    // do rdma init
    HAL_ABORT(rdma_hal_init() == HAL_RET_OK);

    if(!getenv("CAPRI_MOCK_MODE")) {
        // start fte threads
        for (int tid = HAL_THREAD_ID_FTE_MIN; tid <= HAL_THREAD_ID_FTE_MAX; tid++) {
            g_hal_threads[tid]->start(g_hal_threads[tid]);
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

slab*
hal_handle_slab()
{
    return g_hal_state->hal_handle_slab();
}

slab*
hal_handle_ht_entry_slab()
{
    return g_hal_state->hal_handle_ht_entry_slab();
}

ht*
hal_handle_id_ht()
{
    return g_hal_state->hal_handle_id_ht();
}

void
hal_handle_cfg_db_lock(bool readlock, bool lock)
{
    if (readlock == true) {
        if(lock == true) {
            g_hal_state->cfg_db()->rlock();
        } else {
            g_hal_state->cfg_db()->runlock();
        }
    } else {
        if(lock == true) {
            g_hal_state->cfg_db()->wlock();
        } else {
            g_hal_state->cfg_db()->wunlock();
        }
    }
}

}    // namespace hal
