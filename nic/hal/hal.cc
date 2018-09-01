//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nic/hal/hal.hpp"
#include "nic/include/hal_pd.hpp"
#include "sdk/periodic.hpp"
#include "nic/hal/plugins/cfg/lif/lif_manager.hpp"
#include "nic/hal/src/internal/rdma.hpp"
#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/fte/fte_core.hpp"
#include "nic/hal/core/plugins.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "sdk/types.hpp"
#include "sdk/logger.hpp"
#include "sdk/utils.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/hal/core/heartbeat/heartbeat.hpp"

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

namespace hal {

// process globals
// TODO: clean this up and make thread store static to core
extern bool      gl_super_user;

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

    if (!getenv("DISABLE_FTE")) {
        ipc_logger::deinit();
    }

    switch (sig) {
    case SIGINT:
    case SIGTERM:
        HAL_GCOV_FLUSH();
        raise(SIGKILL);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        break;

    case SIGHUP:
    case SIGQUIT:
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
        fprintf(stderr, "HAL ini file %s doesn't exist or not accessible,"
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
                HAL_TRACE_ERR("Invalid forwarding mode : {}, aborting ...\n",
                              val);
                HAL_ABORT(0);
            }
        }
    }
    in.close();

end:
    HAL_TRACE_DEBUG("NIC forwarding mode : {}\n",
                    hal_cfg->forwarding_mode);

    return HAL_RET_OK;
}

// TODO remove duplicate in asic_pd
static platform_type_t
hal_platform_mode_to_sdk_platform_type (hal_platform_mode_t platform_mode)
{
    switch(platform_mode) {
    case HAL_PLATFORM_MODE_SIM:
    case HAL_PLATFORM_MODE_RTL:
        return sdk::types::platform_type_t::PLATFORM_TYPE_SIM;

    case HAL_PLATFORM_MODE_HW:
    case HAL_PLATFORM_MODE_HAPS:
        return sdk::types::platform_type_t::PLATFORM_TYPE_HW;

    case HAL_PLATFORM_MODE_MOCK:
        return sdk::types::platform_type_t::PLATFORM_TYPE_MOCK;

    default:
        break;
    }

    return sdk::types::platform_type_t::PLATFORM_TYPE_HW;
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

    sdk::linkmgr::linkmgr_cfg_t  sdk_cfg;

    // check to see if HAL is running with root permissions
    user = getenv("USER");
    if (user && !strcmp(user, "root")) {
        gl_super_user = true;
    }

    // do SDK initialization, if any
    hal_sdk_init();

    // parse and initialize the catalog
    catalog = sdk::lib::catalog::factory(hal_cfg->catalog_file);
    HAL_ASSERT(catalog != NULL);
    hal_cfg->catalog = catalog;

    // validate control/data cores against catalog
    HAL_ABORT(hal_cores_validate(catalog->cores_mask(),
                                 hal_cfg->control_cores_mask,
                                 hal_cfg->data_cores_mask) == HAL_RET_OK);

    // initialize random number generator
    srand(time(NULL));

    // initialize the logger
    HAL_TRACE_DEBUG("Initializing HAL ...");
    if (!getenv("DISABLE_LOGGING") && hal_logger_init(hal_cfg) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize HAL logger, ignoring ...");
    }

    // do HAL state initialization
    HAL_ABORT(hal_state_init(hal_cfg) == HAL_RET_OK);

    // do platform dependent init
    HAL_ABORT(hal::pd::hal_pd_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Platform initialization done");

    // init fte and hal plugins
    hal::init_plugins(hal_cfg);

    // spawn all necessary PI threads
    HAL_ABORT(hal_thread_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Spawned all HAL thread");

    // do platform dependent clock delta comp init
    HAL_ABORT(hal::pd::hal_pd_clock_delta_comp_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Platform Clock Delta Comp initialization done");

    // do rdma init
    HAL_ABORT(rdma_hal_init() == HAL_RET_OK);

    // unless periodic thread is fully initialized and
    // done calling all plugins' thread init callbacks
    // we shouldn't spawn FTE threads
    while (!sdk::lib::periodic_thread_is_ready()) {
        pthread_yield();
    }

    if (!getenv("DISABLE_FTE") &&
        (hal_cfg->forwarding_mode != HAL_FORWARDING_MODE_CLASSIC) &&
        (hal_cfg->features != HAL_FEATURE_SET_GFT)) {
        // set the number of instances as read from config
        ipc_logger::set_ipc_instances(hal_cfg->num_data_threads);
        // start fte threads
        for (uint32_t i = 0; i < hal_cfg->num_data_threads; i++) {
            // init IPC logger infra for FTE
            if (!i && hal_cfg->shm_mode && ipc_logger::init() != HAL_RET_OK) {
                HAL_TRACE_ERR("IPC logger init failed");
            }
            tid = HAL_THREAD_ID_FTE_MIN + i;
            hal_thread_start(tid, hal_thread_get(tid));
        }
    } else {
        // FTE disabled
        fte::disable_fte();
    }

    // do proxy init
    if (hal_cfg->features == HAL_FEATURE_SET_IRIS) {
        hal_proxy_svc_init();
    }

    // install default HAL configuration
    HAL_ABORT(hal_default_cfg_init(hal_cfg) == HAL_RET_OK);

    sdk_cfg.platform_type  =
            hal_platform_mode_to_sdk_platform_type(hal_cfg->platform_mode);
    sdk_cfg.cfg_path       = hal_cfg->cfg_path.c_str();
    sdk_cfg.catalog        = catalog;
    sdk_cfg.server_builder = hal_cfg->server_builder;

    // ret = linkmgr::linkmgr_init(&sdk_cfg);
    (void)sdk_cfg;
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("linkmgr init failed");
        return HAL_RET_ERR;
    }
    // sdk::linkmgr::linkmgr_event_wait();

    // start monitoring HAL heartbeat
    hal::hb::heartbeat_init();

    // install signal handlers
    hal_sig_init(hal_sig_handler);

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

void
cfg_db_open(cfg_op_t cfg_op)
{
    g_hal_state->cfg_db()->db_open(cfg_op);
}

void
cfg_db_close(void)
{
    g_hal_state->cfg_db()->db_close();
}

}    // namespace hal
