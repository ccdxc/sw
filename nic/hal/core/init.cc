//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "sdk/utils.hpp"
#include "sdk/thread.hpp"
#include "nic/hal/core/init.hpp"
#include "nic/hal/core/plugins.hpp"
#include "nic/hal/core/periodic/periodic.hpp"

namespace hal {

using boost::property_tree::ptree;

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
hal_ret_t
hal_sig_init (hal_sig_handler_t sig_handler)
{
    struct sigaction    act;

    if (sig_handler == NULL) {
        return HAL_RET_ERR;
    }

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = sig_handler;
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
// starting point for the periodic thread loop
//------------------------------------------------------------------------------
void *
periodic_thread_start (void *ctxt)
{
    // initialize timer wheel
    hal::periodic::periodic_thread_init(ctxt);
    // do any plugin-specific thread initialization
    thread_init_plugins(HAL_THREAD_ID_PERIODIC);
    // run main loop
    hal::periodic::periodic_thread_run(ctxt);
    // cleanup per thread state, if any
    thread_exit_plugins(HAL_THREAD_ID_PERIODIC);

    return NULL;
}

#if 0
static void *
fte_pkt_loop_start (void *ctxt)
{
    SDK_THREAD_INIT(ctxt);

    thread *curr_thread = hal::hal_get_current_thread();
    thread_init_plugins(curr_thread->thread_id());
    fte::fte_start(curr_thread->thread_id() - HAL_THREAD_ID_FTE_MIN);
    thread_exit_plugins(curr_thread->thread_id());
    return NULL;
}

//------------------------------------------------------------------------------
//  spawn and setup all the HAL threads - both config and packet loop threads
//------------------------------------------------------------------------------
static hal_ret_t
hal_thread_init (hal_cfg_t *hal_cfg)
{
    uint32_t            i, tid;
    int                 rv, thread_prio;
    char                thread_name[16];
    struct sched_param  sched_param = { 0 };
    pthread_attr_t      attr;
    uint64_t            data_cores_mask = hal_cfg->data_cores_mask;
    uint64_t            cores_mask = 0x0;
    cpu_set_t           cpus;

    // spawn data core threads and pin them to their cores
    thread_prio = sched_get_priority_max(SCHED_FIFO);
    assert(thread_prio >= 0);

    if (hal_cfg->features != HAL_FEATURE_SET_GFT) {
        for (i = 0; i < hal_cfg->num_data_threads; i++) {

            // pin each data thread to a specific core
            cores_mask = 1 << (ffsl(data_cores_mask) - 1);

            tid = HAL_THREAD_ID_FTE_MIN + i;

            HAL_TRACE_DEBUG("Spawning FTE thread {}", tid);

            snprintf(thread_name, sizeof(thread_name), "fte-core-%u", ffsl(data_cores_mask) - 1);

            g_hal_threads[tid] =
                thread::factory(static_cast<const char *>(thread_name),
                                tid,
                                sdk::lib::THREAD_ROLE_DATA,
                                cores_mask,
                                fte_pkt_loop_start,
                                thread_prio,
                                gl_super_user ? SCHED_FIFO : SCHED_OTHER,
                                false);

            HAL_ABORT(g_hal_threads[tid] != NULL);

            g_hal_threads[tid]->set_data(hal_cfg);

            data_cores_mask = data_cores_mask & (data_cores_mask-1);
        }
    }

    // spawn periodic thread that does background tasks
    g_hal_threads[HAL_THREAD_ID_PERIODIC] =
        thread::factory(std::string("periodic-thread").c_str(),
                        HAL_THREAD_ID_PERIODIC,
                        sdk::lib::THREAD_ROLE_CONTROL,
                        0x0 /* use all control cores */,
                        hal_periodic_loop_start,
                        /*
                         * Giving this thread highest priority for now.
                         * Periodic thread might trigger an PD update.
                         * Seems to create starvation as ASIC PD RW thread
                         * has high priority.
                         * One solution might be to increase the priority
                         * dynamically if doing a PD update.
                         */
                        sched_get_priority_max(SCHED_RR),
                        gl_super_user ? SCHED_RR : SCHED_OTHER,
                        true);
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
    cores_mask = hal_cfg->control_cores_mask;
    while (cores_mask != 0) {
        CPU_SET(ffsl(cores_mask) - 1, &cpus);
        cores_mask = cores_mask & (cores_mask - 1);
    }

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
                        sdk::lib::THREAD_ROLE_CONTROL,
                        0x0 /* use all control cores */,
                        thread::dummy_entry_func,
                        sched_param.sched_priority,
                        gl_super_user ? SCHED_RR : SCHED_OTHER,
                        true);
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
#endif

static hal_ret_t
hal_parse_thread_cfg (ptree &pt, hal_cfg_t *hal_cfg)
{
    std::string str = "";

    str = pt.get<std::string>("sw.control_cores_mask");
    hal_cfg->control_cores_mask = std::stoul(str, nullptr, 16);
    sdk::lib::thread::control_cores_mask_set(hal_cfg->control_cores_mask);
    hal_cfg->num_control_threads =
                    sdk::lib::set_bits_count(hal_cfg->control_cores_mask);

    str = pt.get<std::string>("sw.data_cores_mask");
    hal_cfg->data_cores_mask = std::stoul(str, nullptr, 16);
    sdk::lib::thread::data_cores_mask_set(hal_cfg->data_cores_mask);
    hal_cfg->num_data_threads =
                    sdk::lib::set_bits_count(hal_cfg->data_cores_mask);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// parse HAL configuration
//------------------------------------------------------------------------------
hal_ret_t
hal_parse_cfg (const char *cfgfile, hal_cfg_t *hal_cfg)
{
    ptree             pt;
    std::string       sparam;
    std::string       cfg_file;
    char              *cfg_path;

    if (!cfgfile || !hal_cfg) {
        return HAL_RET_INVALID_ARG;
    }

   // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        // stash this path so we can use it in all other modules
        hal_cfg->cfg_path = std::string(cfg_path);
        cfg_file =  hal_cfg->cfg_path + "/" + std::string(cfgfile);
        std::cout << "HAL config file " << cfg_file << std::endl;
    } else {
        hal_cfg->cfg_path = "./";
        cfg_file = hal_cfg->cfg_path + std::string(cfgfile);
    }

    // make sure cfg file exists
    if (access(cfg_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "HAL config file %s doesn't exist or not accessible\n",
                cfg_file.c_str());
        return HAL_RET_ERR;
    }

    // parse the config now
    std::ifstream json_cfg(cfg_file.c_str());
    read_json(json_cfg, pt);
    try {
    std::string mode = pt.get<std::string>("mode");
        if (mode == "sim") {
            hal_cfg->platform_mode = HAL_PLATFORM_MODE_SIM;
        } else if (mode == "hw") {
            hal_cfg->platform_mode = HAL_PLATFORM_MODE_HW;
        } else if (mode == "rtl") {
            hal_cfg->platform_mode = HAL_PLATFORM_MODE_RTL;
        } else if (mode == "haps") {
            hal_cfg->platform_mode = HAL_PLATFORM_MODE_HAPS;
        } else if (mode == "mock") {
            hal_cfg->platform_mode = HAL_PLATFORM_MODE_MOCK;
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
        if (!memcmp("iris", sparam.c_str(), 5)) {
            hal_cfg->features = HAL_FEATURE_SET_IRIS;
        } else if (!memcmp("gft", sparam.c_str(), 4)) {
            hal_cfg->features = HAL_FEATURE_SET_GFT;
        } else if (!memcmp("apollo", sparam.c_str(), 7)) {
            hal_cfg->features = HAL_FEATURE_SET_APOLLO;
        } else {
            hal_cfg->features = HAL_FEATURE_SET_NONE;
            HAL_TRACE_ERR("Unknown feature set {}", sparam.c_str());
        }
        strncpy(hal_cfg->feature_set, sparam.c_str(), HAL_MAX_NAME_STR);
        // Used to enable IPC logger
        hal_cfg->shm_mode = pt.get<bool>("sw.shm", false);

        // parse threads config
        hal_parse_thread_cfg(pt, hal_cfg);
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

static int
hal_sdk_error_logger (const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    HAL_TRACE_ERR_NO_META("{}", logbuf);
    va_end(args);

    return 0;
}

static int
hal_sdk_debug_logger (const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    HAL_TRACE_DEBUG_NO_META("{}", logbuf);
    va_end(args);

    return 0;
}

//------------------------------------------------------------------------------
// SDK initiaization
//------------------------------------------------------------------------------
hal_ret_t
hal_sdk_init (void)
{
    sdk::lib::logger::init(hal_sdk_error_logger, hal_sdk_debug_logger);
    return HAL_RET_OK;
}

hal_ret_t
hal_cores_validate (uint64_t sys_core,
                    uint64_t control_core,
                    uint64_t data_core)
{
    if ((control_core & data_core) != 0) {
        HAL_TRACE_ERR("control core mask {:#x} overlaps with"
                      " data core mask {:#x}",
                      control_core, data_core);
        return HAL_RET_ERR;
    }

    if ((sys_core & (control_core | data_core)) !=
                    (control_core | data_core)) {
        HAL_TRACE_ERR("control core mask {:#x} and data core mask {:#x}"
                      " does not match or exceeds system core mask {:#x}",
                      control_core, data_core, sys_core);
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize HAL logging
//------------------------------------------------------------------------------
hal_ret_t
hal_logger_init (hal_cfg_t *hal_cfg)
{
    std::string          logfile;
    char                 *logdir;
    struct stat          st = { 0 };

    logdir = std::getenv("HAL_LOG_DIR");
    if (!logdir) {
        // log in the current dir
        logfile = std::string("./hal.log");
    } else {
        // check if this log dir exists
        if (stat(logdir, &st) == -1) {
            // doesn't exist, try to create
            if (mkdir(logdir, 0755) < 0) {
                fprintf(stderr,
                        "Log directory %s/ doesn't exist, failed to create one\n",
                        logdir);
                return HAL_RET_ERR;
            }
        } else {
            // log dir exists, check if we have write permissions
            if (access(logdir, W_OK) < 0) {
                // don't have permissions to create this directory
                fprintf(stderr,
                        "No permissions to create log file in %s\n",
                        logdir);
                return HAL_RET_ERR;
            }
        }
        logfile = logdir + std::string("/hal.log");
    }

    // initialize the logger
    hal_cfg->sync_mode_logging = true;
    hal::utils::trace_init("hal", hal_cfg->control_cores_mask,
                           hal_cfg->sync_mode_logging, logfile.c_str(),
                           hal::utils::trace_debug);

    return HAL_RET_OK;
}

}    // namespace hal
