//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "lib/logger/logger.hpp"
#include "lib/thread/thread.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/sdk/platform/utils/mpartition.hpp"
#include "nic/hal/core/core.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/apollo2/hal_state.hpp"
#include "nic/hal/core/plugins.hpp"
#include "nic/fte/fte_core.hpp"
#include "lib/periodic/periodic.hpp"

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

using sdk::lib::thread;
using sdk::lib::catalog;

namespace hal {

// process globals
extern bool      gl_super_user;
thread    *g_hal_threads[HAL_THREAD_ID_MAX];

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
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        break;

    case SIGHUP:
    case SIGQUIT:
    case SIGCHLD:
    case SIGURG:
    default:
        break;
    }
}

//------------------------------------------------------------------------------
// init function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_init (hal_cfg_t *hal_cfg)
{
    int          tid;
    char         *user    = NULL;
    catalog      *catalog = NULL;
    hal_ret_t    ret      = HAL_RET_OK;
    sdk::linkmgr::linkmgr_cfg_t  sdk_cfg;
    std::string mpart_json = hal_cfg->cfg_path + "/apollo/hbm_mem.json";

    // check to see if HAL is running with root permissions
    user = getenv("USER");
    if (user && !strcmp(user, "root")) {
        gl_super_user = true;
    }

    // do SDK initialization, if any
    hal_sdk_init();

    // initialize the logger
    HAL_TRACE_DEBUG("Initializing HAL ...");
    if (!getenv("DISABLE_LOGGING") && hal_logger_init(hal_cfg) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize HAL logger, ignoring ...");
    }

    // parse and initialize the catalog
    catalog = sdk::lib::catalog::factory(hal_cfg->cfg_path, hal_cfg->catalog_file);
    SDK_ASSERT(catalog != NULL);
    hal_cfg->catalog = catalog;
    hal_cfg->mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    // validate control/data cores against catalog
    HAL_ABORT(hal_cores_validate(catalog->cores_mask(),
                                 hal_cfg->control_cores_mask,
                                 hal_cfg->data_cores_mask) == HAL_RET_OK);

    // initialize random number generator
    srand(time(NULL));

    if (gl_super_user) {
        HAL_TRACE_DEBUG("Running as superuser ...");
    }

    // do HAL state initialization
    HAL_ABORT(hal_state_init(hal_cfg) == HAL_RET_OK);

    // init fte and hal plugins
    hal::init_plugins(hal_cfg);

    // spawn all necessary PI threads
    HAL_ABORT(hal_thread_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Spawned all HAL threads");

    // do platform dependent init
    //HAL_ABORT(hal::pd::hal_pd_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Platform initialization done");

    if (!getenv("DISABLE_FTE") &&
        (hal_cfg->forwarding_mode != sdk::lib::FORWARDING_MODE_CLASSIC) &&
        (hal_cfg->features != HAL_FEATURE_SET_GFT)) {
        // start fte threads
        for (uint32_t i = 0; i < hal_cfg->num_data_cores; i++) {
            tid = HAL_THREAD_ID_FTE_MIN + i;
            g_hal_threads[tid]->start(g_hal_threads[tid]);
        }
    } else {
        // FTE disabled
        fte::disable_fte();
    }

    sdk_cfg.platform_type = platform_type_t::PLATFORM_TYPE_MOCK;
    sdk_cfg.cfg_path = hal_cfg->cfg_path.c_str();
    sdk_cfg.catalog  = catalog;

    // ret = linkmgr::linkmgr_init(&sdk_cfg);
    (void)sdk_cfg;
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("linkmgr init failed");
        return HAL_RET_ERR;
    }

    // install signal handlers
    hal_sig_init(hal_sig_handler);

    HAL_TRACE_INFO("HAL initialization completed ...");
    return ret;
}

//------------------------------------------------------------------------------
// cleanup function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_destroy (void)
{
    // cancel  all necessary PI threads
    HAL_ABORT(hal_thread_destroy() == HAL_RET_OK);
    HAL_TRACE_DEBUG("Cancelled  all HAL threads");

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// top level initialization function for HAL, app is expected to only call this
// API for initialization the HAL and all of the asic
//------------------------------------------------------------------------------
int
init (char *cfg_file)
{
    hal_cfg_t    hal_cfg;
    char         *default_config_dir = NULL;

    bzero(&hal_cfg, sizeof(hal_cfg));

    // parse the HAL config file
    if (hal::hal_parse_cfg(cfg_file, &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        exit(1);
    }

    // make sure catalog file exists
    hal_cfg.catalog_file = hal_cfg.cfg_path + "/catalog.json";
    if (access(hal_cfg.catalog_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Catalog file %s has no read permissions\n",
                hal_cfg.catalog_file.c_str());
        exit(1);
    }

    // TODO: HAL_PBC_INIT_CONFIG will have to go away
    default_config_dir = std::getenv("HAL_PBC_INIT_CONFIG");
    if (default_config_dir) {
        hal_cfg.default_config_dir = std::string(default_config_dir);
    } else {
        hal_cfg.default_config_dir = std::string("8x25_hbm");
    }

    // initialize HAL
    if (hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }

    return 0;
}

}    // namespace hal

