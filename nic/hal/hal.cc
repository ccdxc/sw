//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nic/hal/hal.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/sdk/include/sdk/periodic.hpp"
#include "nic/sdk/include/sdk/platform/capri/capri_lif_manager.hpp"
#include "nic/hal/src/internal/rdma.hpp"
#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/fte/fte_core.hpp"
#include "nic/hal/core/plugins.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/logger.hpp"
#include "nic/sdk/include/sdk/utils.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/hal/core/heartbeat/heartbeat.hpp"
#include "nic/hal/src/stats/stats.hpp"
#include "nic/hal/iris/delphi/delphi.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "nic/delphi/utils/log.hpp"

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

namespace hal {

// process globals
// TODO: clean this up
extern bool      gl_super_user;

//------------------------------------------------------------------------------
// handler invoked during normal termnination of HAL (e.g., gtests)
//------------------------------------------------------------------------------
static void
hal_atexit_handler (void)
{
    HAL_TRACE_DEBUG("Flushing logs before exiting ...");
    if (utils::hal_logger()) {
        utils::hal_logger()->flush();
    }
}

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
static void
hal_sig_handler (int sig, siginfo_t *info, void *ptr)
{
    HAL_TRACE_DEBUG("HAL received signal {}", sig);
    if (utils::hal_logger()) {
        utils::hal_logger()->flush();
    }

    switch (sig) {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
        if (!getenv("DISABLE_FTE") && 
            (g_hal_cfg.shm_mode == true)) {
            ipc_logger::deinit();
        }
        HAL_GCOV_FLUSH();
        raise(SIGKILL);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        break;

    case SIGHUP:
    case SIGCHLD:
    case SIGURG:
    default:
        break;
    }
}

static hal_forwarding_mode_t
hal_get_forwarding_mode (std::string mode)
{
    if (mode == "smart-switch") {
        return HAL_FORWARDING_MODE_SMART_SWITCH;
    } else if (mode == "smart-host-pinned") {
        return HAL_FORWARDING_MODE_SMART_HOST_PINNED;
    } else if (mode == "classic") {
        return HAL_FORWARDING_MODE_CLASSIC;
    }
    return HAL_FORWARDING_MODE_NONE;
}

//------------------------------------------------------------------------------
// parse HAL .ini file
//------------------------------------------------------------------------------
hal_ret_t
hal_parse_ini (const char *inifile, hal_cfg_t *hal_cfg)
{
    std::string         ini_file, line;

    // check if ini file exists
    ini_file = hal_cfg->cfg_path + "/" + std::string(inifile);
    std::ifstream in(ini_file.c_str());
    if (access(ini_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "HAL ini file %s doesn't exist or not accessible, "
                "picking smart-switch mode\n",
                ini_file.c_str());
        hal_cfg->forwarding_mode = HAL_FORWARDING_MODE_SMART_HOST_PINNED;
        goto end;
    }

    if (!in) {
        HAL_TRACE_ERR("Failed to open ini file ... "
                      "setting forwarding mode to smart-switch\n");
        hal_cfg->forwarding_mode = HAL_FORWARDING_MODE_SMART_HOST_PINNED;
        goto end;
    }

    while (std::getline(in, line)) {
        std::string key = line.substr(0, line.find("="));
        std::string val = line.substr(line.find("=")+1, line.length()-1);

        if (key == "forwarding_mode") {
            hal_cfg->forwarding_mode = hal_get_forwarding_mode(val);
            if (hal_cfg->forwarding_mode == HAL_FORWARDING_MODE_NONE) {
                HAL_ASSERT_TRACE_RETURN(FALSE, HAL_RET_INVALID_ARG,
                                        "Invalid forwarding mode {}", val);
            }
        }
    }
    in.close();

end:

    HAL_TRACE_DEBUG("NIC forwarding mode : {}\n",
                    hal_cfg->forwarding_mode);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// bring up delphi thread
//------------------------------------------------------------------------------
static hal_ret_t
hal_delphi_thread_init (hal_cfg_t *hal_cfg)
{
    int                 thread_prio, sched_policy;
    sdk::lib::thread    *hal_thread;

    delphi::SetLogger(std::shared_ptr<logger>(utils::hal_logger()));
    sched_policy = gl_super_user ? SCHED_RR : SCHED_OTHER;
    thread_prio = sched_get_priority_max(sched_policy);
    hal_thread = hal_thread_create(std::string("delphic").c_str(),
                                   HAL_THREAD_ID_DELPHI_CLIENT,
                                   sdk::lib::THREAD_ROLE_CONTROL,
                                   0x0,    // use all control cores
                                   svc::delphi_client_start,
                                   thread_prio, sched_policy,
                                   NULL);
    HAL_ASSERT_TRACE_RETURN((hal_thread != NULL), HAL_RET_ERR,
                            "Failed to spawn delphic thread");
    hal_thread->start(hal_thread);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// init function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_init (hal_cfg_t *hal_cfg)
{
    int                tid;
    char               *user    = NULL;
    sdk::lib::catalog  *catalog = NULL;
    hal_ret_t          ret      = HAL_RET_OK;

    // check to see if HAL is running with root permissions
    user = getenv("USER");
    if (user && !strcmp(user, "root")) {
        gl_super_user = true;
    }
    //gl_super_user = false;    // TODO: temporary until bringup issues are resolved

    // do SDK initialization, if any
    hal_sdk_init();

    // parse and initialize the catalog
    catalog = sdk::lib::catalog::factory(hal_cfg->catalog_file);
    HAL_ASSERT_TRACE_RETURN(catalog != NULL, HAL_RET_ERR, "Catalog file error");
    hal_cfg->catalog = catalog;

    // validate control/data cores against catalog
    ret = hal_cores_validate(catalog->cores_mask(),
                             hal_cfg->control_cores_mask,
                             hal_cfg->data_cores_mask);
    HAL_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "CPU core validation failure");

    // initialize random number generator
    srand(time(NULL));

    // initialize the logger
    HAL_TRACE_DEBUG("Initializing HAL ...");
    if (!getenv("DISABLE_LOGGING") && hal_logger_init(hal_cfg) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize HAL logger, ignoring ...");
    }
    if (gl_super_user) {
        HAL_TRACE_DEBUG("Running as superuser ...");
    }

    // instantiate delphi thread
    hal_delphi_thread_init(hal_cfg);

    // do HAL state initialization
    ret = hal_state_init(hal_cfg);
    HAL_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "HAL state init failure");
    ret = hal_main_thread_init(hal_cfg);
    HAL_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "HAL main thread initialization failure");

    // do platform dependent init
    ret = pd::hal_pd_init(hal_cfg);
    HAL_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "HAL PD layer initialization failure");
    HAL_TRACE_DEBUG("Platform initialization done");

    // init fte and hal plugins
    init_plugins(hal_cfg);

    // spawn all necessary PI threads
    ret = hal_thread_init(hal_cfg);
    HAL_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "HAL thread initialization failure");
    HAL_TRACE_DEBUG("Spawned all HAL threads");

    // do platform dependent clock delta computation initialization
    ret = pd::hal_pd_clock_delta_comp_init(hal_cfg);
    HAL_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "Clock delta computation initialization failure");
    HAL_TRACE_DEBUG("Platform clock delta computation init done");

    // do rdma init
    ret = rdma_hal_init();
    HAL_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "RDMA intialization failure");

    // unless periodic thread is fully initialized and
    // done calling all plugins' thread init callbacks
    // we shouldn't spawn FTE threads
    while (!sdk::lib::periodic_thread_is_ready()) {
        pthread_yield();
    }

    // notify sysmgr that we are up
    svc::hal_init_done();

    if (!getenv("DISABLE_FTE") &&
        (hal_cfg->forwarding_mode != HAL_FORWARDING_MODE_CLASSIC) &&
        (hal_cfg->features != HAL_FEATURE_SET_GFT)) {
        // set the number of instances as read from config
        ipc_logger::set_ipc_instances(hal_cfg->num_data_threads);
        // start fte threads
        for (uint32_t i = 0; i < hal_cfg->num_data_threads; i++) {
            // init IPC logger infra for FTE
            if (!i && hal_cfg->shm_mode &&
                ipc_logger::init(std::shared_ptr<logger>(utils::hal_logger())) != HAL_RET_OK) {
                HAL_TRACE_ERR("IPC logger init failed");
            }
            tid = HAL_THREAD_ID_FTE_MIN + i;
            hal_thread_start(tid, hal_thread_get(tid));
        }
    } else {
        // FTE disabled
        fte::disable_fte();
    }

    // linkmgr init
    hal_linkmgr_init(hal_cfg);

    // start monitoring HAL heartbeat
    hb::heartbeat_init();

    // initialize stats module
    hal_stats_init(hal_cfg);

    // install signal handlers
    hal_sig_init(hal_sig_handler);

    // install atexit() handler
    atexit(hal_atexit_handler);

    return ret;
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

slab *
hal_handle_slab (void)
{
    return g_hal_state->hal_handle_slab();
}

slab *
hal_handle_ht_entry_slab (void)
{
    return g_hal_state->hal_handle_ht_entry_slab();
}

ht *
hal_handle_id_ht (void)
{
    return g_hal_state->hal_handle_id_ht();
}

void
hal_handle_cfg_db_lock (bool readlock, bool lock)
{
    if (readlock == true) {
        if (lock == true) {
            g_hal_state->cfg_db()->rlock();
        } else {
            g_hal_state->cfg_db()->runlock();
        }
    } else {
        if (lock == true) {
            g_hal_state->cfg_db()->wlock();
        } else {
            g_hal_state->cfg_db()->wunlock();
        }
    }
}

//------------------------------------------------------------------------------
// API open HAL cfg db in read/write mode
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db_open (cfg_op_t op)
{
    HAL_TRACE_DEBUG("Opening cfg db with mode {}", op);
    return g_hal_state->cfg_db()->db_open(op);
}

//------------------------------------------------------------------------------
// API to close the HAL cfg db after performing commit/abort operation
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db_close (void)
{
    HAL_TRACE_DEBUG("Closing cfg db, current mode {}", t_cfg_db_ctxt.cfg_op_);
    return g_hal_state->cfg_db()->db_close();
}

}    // namespace hal
