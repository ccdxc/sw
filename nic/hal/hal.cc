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
#include "nic/hal/hal_devapi.hpp"
#include "nic/include/hal_pd.hpp"
#include "lib/periodic/periodic.hpp"
#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/core/event_ipc.hpp"
#include "nic/fte/fte_core.hpp"
#include "nic/hal/core/plugins.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/platform/marvell/marvell.hpp"
#include "lib/logger/logger.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/iris/delphi/delphi_events.hpp"
#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/hal/core/heartbeat/heartbeat.hpp"
#include "nic/hal/src/stats/stats.hpp"
#include "nic/hal/iris/delphi/delphi.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "nic/delphi/utils/log.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "platform/src/app/nicmgrd/src/nicmgr_init.hpp"
#include "platform/utils/mpartition.hpp"

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

namespace hal {

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
    nicmgr::nicmgr_exit();
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
    case SIGABRT:
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
        if (!getenv("DISABLE_FTE") &&
            (is_platform_type_hw() &&
            !getenv("DISABLE_FWLOG"))) {
            ipc_logger::deinit();
        }
        nicmgr::nicmgr_exit();
        HAL_GCOV_FLUSH();
        hal_destroy();
        raise(SIGKILL);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        break;

    case SIGHUP:
    case SIGCHLD:
    case SIGURG:
    case SIGPIPE:
    default:
        break;
    }
}

//------------------------------------------------------------------------------
// bring up delphi thread
//------------------------------------------------------------------------------
static hal_ret_t
hal_delphi_thread_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t            ret = HAL_RET_OK;
    sdk::lib::thread    *hal_thread;

    delphi::SetLogger(std::shared_ptr<logger>(utils::hal_logger()));
    hal_thread =
        hal_thread_create(std::string("delphic").c_str(),
            HAL_THREAD_ID_DELPHI_CLIENT,
            sdk::lib::THREAD_ROLE_CONTROL,
            0x0,    // use all control cores
            svc::delphi_client_start,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            NULL);
    SDK_ASSERT_TRACE_RETURN((hal_thread != NULL), HAL_RET_ERR,
                            "Failed to spawn delphic thread");
    hal_thread->start(hal_thread);

    ret = hal_events_recorder_init();
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "Failed to initialize HAL Events recorder");

    return HAL_RET_OK;
}


static void
nicmgr_thread_start (void *ctxt)
{
    sdk::platform::platform_type_t platform = g_hal_state->platform_type();
    sdk::event_thread::event_thread *curr_thread = (sdk::event_thread::event_thread *)ctxt;

    nicmgr::nicmgr_init(platform, curr_thread);
    hal_thread_add((sdk::lib::thread *)ctxt);
}

static void
nicmgr_thread_exit (void *ctxt)
{
    HAL_TRACE_DEBUG("Nicmgr exiting ..");
    nicmgr::nicmgr_exit();
}

hal_ret_t
hal_nicmgr_init (hal_cfg_t *hal_cfg)
{
    sdk::event_thread::event_thread *nicmgr_thread;

    nicmgr_thread =
        sdk::event_thread::event_thread::factory(
            "nicmgr", HAL_THREAD_ID_NICMGR,
            sdk::lib::THREAD_ROLE_CONTROL,
            0x0,    // use all control cores
            nicmgr_thread_start,  // entry function
            nicmgr_thread_exit,  // exit function
            NULL,  // thread event callback
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            NULL);
    SDK_ASSERT_TRACE_RETURN((nicmgr_thread != NULL), HAL_RET_ERR,
                            "Failed to spawn nicmgr thread");
    nicmgr_thread->start(nicmgr_thread);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// init function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_init (hal_cfg_t *hal_cfg)
{
    int                tid;
    sdk::lib::catalog  *catalog = NULL;
    hal_ret_t          ret      = HAL_RET_OK;
    std::string        mpart_json;
    marvell_cfg_t      marvell_cfg;

    // read the startup device config
    hal_device_cfg_init(hal_cfg);

    mpart_json = sdk::platform::utils::mpartition::get_mpart_file_path(hal_cfg->cfg_path, hal_cfg->feature_set, (sdk::lib::dev_feature_profile_t)hal_cfg->device_cfg.feature_profile);

    // do SDK initialization, if any
    hal_sdk_init();

    // initialize the logger
    if (hal_logger_init(hal_cfg) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize HAL logger, ignoring ...");
    }
    HAL_TRACE_ERR("----------------------------- HAL Start -----------------------------");
    HAL_TRACE_INFO("Initializing HAL with : {}", mpart_json);

    // parse and initialize the catalog
    catalog = sdk::lib::catalog::factory(hal_cfg->cfg_path, hal_cfg->catalog_file, hal_cfg->platform);
    SDK_ASSERT_TRACE_RETURN(catalog != NULL, HAL_RET_ERR, "Catalog file error");
    hal_cfg->catalog = catalog;
    hal_cfg->mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    // This should come from hal-config file or policy
    hal_cfg->bypass_fte = false;
    hal_cfg->max_sessions = HAL_DSC_MAX_SESSIONS; // This is total sessions supported per DSC
                                                 // should come from config-file, should be modifiable in future

    // validate control/data cores against catalog
    ret = hal_cores_validate(catalog->cores_mask(),
                             hal_cfg->control_cores_mask,
                             hal_cfg->data_cores_mask);
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "CPU core validation failure");

    // initialize random number generator
    srand(time(NULL));

    // instantiate delphi thread
    hal_delphi_thread_init(hal_cfg);

    // do HAL state initialization
    ret = hal_state_init(hal_cfg);
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "HAL state init failure");
    ret = hal_main_thread_init(hal_cfg);
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "HAL main thread initialization failure");

    // do platform dependent init
    ret = pd::hal_pd_init(hal_cfg);
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "HAL PD layer initialization failure");
    HAL_TRACE_DEBUG("Platform initialization done");

    // initialize the marvell switch
    memset(&marvell_cfg, 0, sizeof(marvell_cfg_t));
    marvell_cfg.catalog = catalog;
    sdk::marvell::marvell_switch_init(&marvell_cfg);

    // nicmgr init
    if (!getenv("DISABLE_NICMGR_HAL_THREAD")) {
        hal_nicmgr_init(hal_cfg);
    }

    // linkmgr init
    hal_linkmgr_init(hal_cfg, hal::port_event_cb);

    // init fte and hal plugins
    init_plugins(hal_cfg);

    // spawn all necessary PI threads
    ret = hal_thread_init(hal_cfg);
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "HAL thread initialization failure");
    HAL_TRACE_DEBUG("Spawned all HAL threads");

    // unless periodic thread is fully initialized and
    // done calling all plugins' thread init callbacks
    // we shouldn't spawn FTE threads
    while (!sdk::lib::periodic_thread_is_ready()) {
        pthread_yield();
    }

    // notify sysmgr that we are up
    //svc::hal_init_done();
    if (!getenv("DISABLE_FTE")) {
        if (hal_cfg->features != HAL_FEATURE_SET_GFT) {
            HAL_TRACE_DEBUG("Number of IPC instances: {}", hal_cfg->num_data_cores);
            // set the number of instances as read from config
            // Number of instances is equal to number of data cores == 1.
            ipc_logger::set_ipc_instances(hal_cfg->num_data_cores);
            // start fte threads
            for (uint32_t i = 0; i < hal_cfg->num_data_cores; i++) {
                // init IPC logger infra for FTE
                if (!i && (hal_cfg->platform == platform_type_t::PLATFORM_TYPE_HW) &&
                    !getenv("DISABLE_FWLOG") &&
                    ipc_logger::init(std::shared_ptr<logger>(utils::hal_logger())) != HAL_RET_OK) {
                    HAL_TRACE_ERR("IPC logger init failed");
                }
                tid = HAL_THREAD_ID_FTE_MIN + i;
                hal_thread_start(tid, hal_thread_get(tid));
            }
        }
    } else {
        // FTE disabled
        fte::disable_fte();
    }

    if (getenv("MICROSEG_ENABLE")) {
        g_hal_state->set_fwd_mode(sys::FWD_MODE_MICROSEG);
        g_hal_state->set_policy_mode(sys::POLICY_MODE_ENFORCE);
    }

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
    HAL_TRACE_DEBUG("Cancelling  all HAL threads");
    HAL_ABORT(hal_thread_destroy() == HAL_RET_OK);
    HAL_TRACE_DEBUG("Waiting for  all HAL threads to stop");
    HAL_ABORT(hal_wait() == HAL_RET_OK);
    HAL_TRACE_DEBUG("HAL exiting ...");
    hal::utils::hal_logger()->flush();
    hal::utils::trace_deinit();
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
    HAL_TRACE_VERBOSE("Opening cfg db with mode {}", op);
    return g_hal_state->cfg_db()->db_open(op);
}

//------------------------------------------------------------------------------
// API to close the HAL cfg db after performing commit/abort operation
//------------------------------------------------------------------------------
hal_ret_t
hal_cfg_db_close (void)
{
    HAL_TRACE_VERBOSE("Closing cfg db, current mode {}", t_cfg_db_ctxt.cfg_op_);
    return g_hal_state->cfg_db()->db_close();
}

//------------------------------------------------------------------------------
// API to close the HAL cfg db after performing commit/abort operation
//------------------------------------------------------------------------------
hal::cfg_op_t
hal_cfg_db_get_mode (void)
{
    return (t_cfg_db_ctxt.cfg_op_);
}

//------------------------------------------------------------------------------
// API Callback api from SDK library
//------------------------------------------------------------------------------
void
asiccfg_init_completion_event (sdk_status_t status)
{

}

void
svc_init_done (void)
{
    // notify sysmgr that we are up
    hal::svc::hal_init_done();
    g_devapi_hal.set_hal_up(true);
    // raise HAL_UP event
    sdk::ipc::broadcast(event_id_t::EVENT_ID_HAL_UP, NULL, 0);
}

}    // namespace hal
