//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/hal/core/core.hpp"
#include "nic/hal/core/plugins.hpp"
#include "lib/periodic/periodic.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/sdk/lib/device/device.hpp"
#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/hal/iris/include/hal_state.hpp"

uint64_t g_mgmt_if_mac;

namespace hal {

using boost::property_tree::ptree;

static sdk::lib::thread    *g_hal_threads[HAL_THREAD_ID_MAX];
bool                       gl_super_user = false;

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
    sdk::lib::periodic_thread_init(ctxt);
    // do any plugin-specific thread initialization
    thread_init_plugins(HAL_THREAD_ID_PERIODIC);
    // run main loop
    sdk::lib::periodic_thread_run(ctxt);
    // cleanup per thread state, if any
    thread_exit_plugins(HAL_THREAD_ID_PERIODIC);

    return NULL;
}

static void *
fte_pkt_loop_start (void *ctxt)
{
    SDK_THREAD_INIT(ctxt);
    sdk::lib::thread *curr_thread = (sdk::lib::thread *)ctxt;
    thread_init_plugins(curr_thread->thread_id());
    fte::fte_start(ctxt);
    thread_exit_plugins(curr_thread->thread_id());
    return NULL;
}

//------------------------------------------------------------------------------
// return current thread pointer, for gRPC threads curr_thread is not set,
// however, they are considered as cfg threads
//------------------------------------------------------------------------------
sdk::lib::thread *
hal_get_current_thread (void)
{
    return sdk::lib::thread::current_thread() ?
               sdk::lib::thread::current_thread() :
               g_hal_threads[HAL_THREAD_ID_CFG];
}

//------------------------------------------------------------------------------
// given a thread id, return the thread class corresponding to it
//------------------------------------------------------------------------------
sdk::lib::thread *
hal_get_thread (uint32_t thread_id)
{
    if (thread_id < HAL_THREAD_ID_MAX) {
        return g_hal_threads[thread_id];
    }
    return NULL;
}

//------------------------------------------------------------------------------
//  uninit all  the HAL threads - both config and packet loop threads.
//------------------------------------------------------------------------------
hal_ret_t
hal_thread_destroy (void)
{
    HAL_ABORT(g_hal_threads[HAL_THREAD_ID_PERIODIC] != NULL);
    g_hal_threads[HAL_THREAD_ID_PERIODIC]->stop();

    hal::utils::trace_deinit();

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
// get thread object for given thread id
//------------------------------------------------------------------------------
sdk::lib::thread *
hal_thread_get (uint32_t thread_id)
{
    if (thread_id >= HAL_THREAD_ID_MAX) {
        return NULL;
    }
    return g_hal_threads[thread_id];
}

//------------------------------------------------------------------------------
// start execution of thread with given id
//------------------------------------------------------------------------------
void
hal_thread_start (uint32_t thread_id, void *ctxt)
{
    g_hal_threads[thread_id]->start(ctxt);
}

//------------------------------------------------------------------------------
// add given thread to thread store
//------------------------------------------------------------------------------
hal_ret_t
hal_thread_add (sdk::lib::thread *hal_thread)
{
    uint32_t    tid;

    if (hal_thread == NULL) {
        return HAL_RET_INVALID_ARG;
    }
    tid = hal_thread->thread_id();
    if ((tid >= HAL_THREAD_ID_MAX) || g_hal_threads[tid]) {
        return HAL_RET_INVALID_ARG;
    }
    g_hal_threads[tid] = hal_thread;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// wrapper API to create HAL threads
//------------------------------------------------------------------------------
sdk::lib::thread *
hal_thread_create (const char *name, uint32_t thread_id,
                   sdk::lib::thread_role_t thread_role,
                   uint64_t cores_mask,
                   sdk::lib::thread_entry_func_t entry_func,
                   uint32_t thread_prio, int sched_policy, void *data)
{
    g_hal_threads[thread_id] =
        sdk::lib::thread::factory(name, thread_id, thread_role, cores_mask,
                                  entry_func, thread_prio, sched_policy,
                                  (thread_role == sdk::lib::THREAD_ROLE_DATA) ?
                                       false : true);
    if (g_hal_threads[thread_id]) {
        g_hal_threads[thread_id]->set_data(data);
    }

    return g_hal_threads[thread_id];
}

//------------------------------------------------------------------------------
// main thread needs to be setup before other threads
//------------------------------------------------------------------------------
hal_ret_t
hal_main_thread_init (hal_cfg_t *hal_cfg)
{
    int                 rv;
    struct sched_param  sched_param = { 0 };
    pthread_attr_t      attr;
    cpu_set_t           cpu_set;
    uint64_t            cores_mask = 0x0;
    sdk::lib::thread    *hal_thread;

    // make the current thread, main hal config thread also a real-time thread
    rv = pthread_attr_init(&attr);
    if (rv != 0) {
        HAL_TRACE_ERR("pthread_attr_init failure, err : {}", rv);
        return HAL_RET_ERR;
    }

    // compute core affinity
    CPU_ZERO(&cpu_set);
    cores_mask = hal_cfg->control_cores_mask;
    while (cores_mask != 0) {
        CPU_SET(ffsl(cores_mask) - 1, &cpu_set);
        cores_mask = cores_mask & (cores_mask - 1);
    }

    HAL_TRACE_INFO("Main thread CPU mask: {:#x}",
                   sdk::lib::thread::get_cpu_mask(cpu_set));

    // and set the core affinity
    rv = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpu_set);
    if (rv != 0) {
        HAL_TRACE_ERR("pthread_attr_setaffinity_np failure, err : {}", rv);
        return HAL_RET_ERR;
    }

    // switch to real-time scheduling
    sched_param.sched_priority =
        sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL);
    rv =
        sched_setscheduler(0,
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            &sched_param);
    if (rv != 0) {
        HAL_TRACE_ERR("sched_setscheduler failure, err : {}", rv);
        return HAL_RET_ERR;
    }

    // create a placeholder thread object for this main thread
    hal_thread =
        hal_thread_create(std::string("cfg").c_str(),
                          HAL_THREAD_ID_CFG,
                          sdk::lib::THREAD_ROLE_CONTROL,
                          0x0,    // use all control cores
                          sdk::lib::thread::dummy_entry_func,
                          sched_param.sched_priority,
                          sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                          NULL);
    hal_thread->set_data(hal_thread);
    hal_thread->set_pthread_id(pthread_self());
    hal_thread->set_running(true);
    SDK_THREAD_INIT(hal_thread);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
//  spawn and setup all the HAL threads - both config and packet loop threads
//------------------------------------------------------------------------------
hal_ret_t
hal_thread_init (hal_cfg_t *hal_cfg)
{
    uint32_t            i, tid;
    char                thread_name[16];
    uint64_t            data_cores_mask = hal_cfg->data_cores_mask;
    uint64_t            cores_mask = 0x0;
    sdk::lib::thread    *hal_thread;

    if (hal_cfg->device_cfg.forwarding_mode == HAL_FORWARDING_MODE_CLASSIC) {
        // 1 FTE thread for fte-span
        tid = HAL_THREAD_ID_FTE_MIN;
        HAL_TRACE_DEBUG("Spawning FTE thread {}", tid);
        snprintf(thread_name, sizeof(thread_name), "fte-%u",
                 ffsl(data_cores_mask) - 1);
        hal_thread =
            hal_thread_create(static_cast<const char *>(thread_name),
                              tid, sdk::lib::THREAD_ROLE_CONTROL,
                              0x0, // use all control cores
                              fte_pkt_loop_start,
                              0, /* priority. used only for real time */
                              SCHED_OTHER, /* sched. policy: non-real time */
                              hal_cfg);
        SDK_ASSERT_TRACE_RETURN((hal_thread != NULL), HAL_RET_ERR,
                                "FTE thread {} creation failed", tid);

    } else if (hal_cfg->features != HAL_FEATURE_SET_GFT) {
        // spawn data core threads and pin them to their cores
        for (i = 0; i < hal_cfg->num_data_cores; i++) {
            // pin each data thread to a specific core
            cores_mask = 1 << (ffsl(data_cores_mask) - 1);
            tid = HAL_THREAD_ID_FTE_MIN + i;
            HAL_TRACE_DEBUG("Spawning FTE thread {}", tid);
            snprintf(thread_name, sizeof(thread_name), "fte-%u",
                     ffsl(data_cores_mask) - 1);
            hal_thread =
                hal_thread_create(static_cast<const char *>(thread_name),
                                  tid, sdk::lib::THREAD_ROLE_DATA,
                                  cores_mask, fte_pkt_loop_start,
                                  sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_DATA),
                                  sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_DATA),
                                  hal_cfg);
            SDK_ASSERT_TRACE_RETURN((hal_thread != NULL), HAL_RET_ERR,
                                    "FTE thread {} creation failed", tid);
            data_cores_mask = data_cores_mask & (data_cores_mask-1);
        }
    }

    // spawn periodic thread that does background tasks
    hal_thread =
        hal_thread_create(std::string("periodic").c_str(),
                          HAL_THREAD_ID_PERIODIC,
                          sdk::lib::THREAD_ROLE_CONTROL,
                          0x0,    // use all control cores
                          periodic_thread_start,
                          sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                          sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                          NULL);
    SDK_ASSERT_TRACE_RETURN((hal_thread != NULL), HAL_RET_ERR,
                            "HAL periodic thread create failure");
    hal_thread->start(hal_thread);

    return HAL_RET_OK;
}

static hal_ret_t
hal_parse_cores_cfg (ptree &pt, hal_cfg_t *hal_cfg)
{
    std::string str = "";

    str = pt.get<std::string>("sw.control_cores_mask");
    hal_cfg->control_cores_mask = std::stoul(str, nullptr, 16);
    sdk::lib::thread::control_cores_mask_set(hal_cfg->control_cores_mask);
    hal_cfg->num_control_cores =
                    sdk::lib::count_bits_set(hal_cfg->control_cores_mask);

    str = pt.get<std::string>("sw.data_cores_mask");
    hal_cfg->data_cores_mask = std::stoul(str, nullptr, 16);
    sdk::lib::thread::data_cores_mask_set(hal_cfg->data_cores_mask);
    hal_cfg->num_data_cores =
                    sdk::lib::count_bits_set(hal_cfg->data_cores_mask);

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
            hal_cfg->platform = platform_type_t::PLATFORM_TYPE_SIM;
        } else if (mode == "hw") {
            hal_cfg->platform = platform_type_t::PLATFORM_TYPE_HW;
        } else if (mode == "rtl") {
            hal_cfg->platform = platform_type_t::PLATFORM_TYPE_RTL;
        } else if (mode == "haps") {
            hal_cfg->platform = platform_type_t::PLATFORM_TYPE_HAPS;
        } else if (mode == "mock") {
            hal_cfg->platform = platform_type_t::PLATFORM_TYPE_MOCK;
        }

        hal_cfg->grpc_port = pt.get<std::string>("sw.grpc_port");
        HAL_TRACE_DEBUG("Setting GRPC port to: {}", hal_cfg->grpc_port);
        if (getenv("HAL_GRPC_PORT")) {
            hal_cfg->grpc_port = getenv("HAL_GRPC_PORT");
            HAL_TRACE_DEBUG("Overriding GRPC Port to : {}", hal_cfg->grpc_port);
        }
        hal_cfg->vmotion_port = pt.get<std::string>("sw.vmotion_port");
        if (getenv("HAL_VMOTION_PORT")) {
            hal_cfg->vmotion_port = getenv("HAL_VMOTION_PORT");
            HAL_TRACE_DEBUG("Overriding VMotion Port to : {}", hal_cfg->grpc_port);
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
        hal_cfg->shm_mode = pt.get<bool>("sw.shm", false);
        // parse (control & data) core config
        hal_parse_cores_cfg(pt, hal_cfg);
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }

    bool qemu = std::getenv("QEMU");
    if (qemu) {
        hal_cfg->qemu = true;
    } else {
        hal_cfg->qemu = false;
    }
    return HAL_RET_OK;
}

static int
hal_sdk_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    if ((int)hal_trace_level() >= (int)tracel_level)  {
        va_start(args, format);
        vsnprintf(logbuf, sizeof(logbuf), format, args);
        switch (tracel_level) {
        case sdk::lib::SDK_TRACE_LEVEL_ERR:
            HAL_TRACE_ERR_NO_META("{}", logbuf);
            break;
        case sdk::lib::SDK_TRACE_LEVEL_WARN:
            HAL_TRACE_WARN_NO_META("{}", logbuf);
            break;
        case sdk::lib::SDK_TRACE_LEVEL_INFO:
            HAL_TRACE_INFO_NO_META("{}", logbuf);
            break;
        case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
            HAL_TRACE_DEBUG_NO_META("{}", logbuf);
            break;
        case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
            HAL_TRACE_VERBOSE_NO_META("{}", logbuf);
            break;
        default:
            break;
        }
        va_end(args);
    }

    return 0;
}

//------------------------------------------------------------------------------
// SDK initiaization
//------------------------------------------------------------------------------
hal_ret_t
hal_sdk_init (void)
{
    sdk::lib::logger::init(hal_sdk_logger);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// make sure given data and control cores are subset of total available cores
//------------------------------------------------------------------------------
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


// verify_logdir checks if the dir exists and we have write permisions
static bool
verify_logdir (const char *logdir)
{
    struct stat st = { 0 };

    // check if this log dir exists
    if (stat(logdir, &st) == -1) {
        // doesn't exist, try to create
        if (mkdir(logdir, 0755) < 0) {
            fprintf(stderr,
                "Log directory %s/ doesn't exist, failed to create one\n",
                logdir);
            return false;
        }
    } else {
        // log dir exists, check if we have write permissions
        if (access(logdir, W_OK) < 0) {
            // don't have permissions to create this directory
            fprintf(stderr,
                "No permissions to create log file in %s\n",
                logdir);
            return false;
        }
    }
    return true;
}

// get_logfile returns the full path of a logfile based on enviromental
// variables
static const std::string
get_logfile (const char *env, const char *base, const char *alt)
{
    const char *logdir = NULL;

    logdir = std::getenv(env);
    if (!logdir) {
        return std::string(alt);
    }

    if (!verify_logdir(logdir)) {
        return std::string("");
    }

    return std::string(logdir) + "/" + std::string(base);
}

//------------------------------------------------------------------------------
// initialize HAL logging
//------------------------------------------------------------------------------
hal_ret_t
hal_logger_init (hal_cfg_t *hal_cfg)
{
    std::string persistent_logfile;
    std::string non_persistent_logfile;

    persistent_logfile = get_logfile("PERSISTENT_LOGDIR", "hal.log",
        "./hal.pers.log");
    non_persistent_logfile = get_logfile("NON_PERSISTENT_LOGDIR", "hal.log",
        "./hal.nonpers.log");

    // initialize the logger
    hal_cfg->sync_mode_logging = false;
    if (hal_cfg->platform == platform_type_t::PLATFORM_TYPE_SIM) {
        // TODO: Remove this hack. alg gtests are stuck with
        //       sync disabled.
        hal_cfg->sync_mode_logging = true;
    }
    hal::utils::trace_init("hal", hal_cfg->control_cores_mask,
                           hal_cfg->sync_mode_logging,
                           persistent_logfile.c_str(),
                           non_persistent_logfile.c_str(),
                           5 << 20, // 50MB
                           10,      // 10 files
                           ::utils::trace_err,
                           getenv("DISABLE_LOGGING") ? (::utils::trace_none) :
                                                       (::utils::trace_debug));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize port control operations
//------------------------------------------------------------------------------
hal_ret_t
hal_linkmgr_init (hal_cfg_t *hal_cfg, port_event_notify_t port_event_cb)
{
    hal_ret_t ret = HAL_RET_OK;

    /* skip linkmgr init for apollo until compilation for
     * apollo-gtest is resolved
     */

#if !defined(APOLLO) && !defined(HELLO)
    // enable linkmgr only for sim/mock
    if (hal_cfg->platform == platform_type_t::PLATFORM_TYPE_SIM ||
        hal_cfg->platform == platform_type_t::PLATFORM_TYPE_MOCK ||
        hal_cfg->platform == platform_type_t::PLATFORM_TYPE_HW) {

        sdk::linkmgr::linkmgr_cfg_t sdk_cfg;
        memset(&sdk_cfg, 0, sizeof(sdk_cfg));

        sdk_cfg.platform_type  = hal_cfg->platform;
        sdk_cfg.cfg_path       = hal_cfg->cfg_path.c_str();
        sdk_cfg.catalog        = hal_cfg->catalog;
        sdk_cfg.server_builder = hal_cfg->server_builder;
        sdk_cfg.admin_state    = hal_cfg->device_cfg.admin_state;
        sdk_cfg.port_event_cb  = port_event_cb;
        sdk_cfg.mempartition   = hal_cfg->mempartition;

        ret = linkmgr::linkmgr_init(&sdk_cfg);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("linkmgr init failed");
            return ret;
        }
    }
#endif

    return ret;
}

#define HAL_VMOTION_MAX_THREADS \
    (HAL_THREAD_ID_VMOTION_THREADS_MAX - HAL_THREAD_ID_VMOTION_THREADS_MIN + 1)
#define HAL_VMOTION_PORT 50055
hal_ret_t
hal_vmotion_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t ret = HAL_RET_OK;
    vmotion   *vm;

    if (hal_cfg->device_cfg.forwarding_mode ==
        HAL_FORWARDING_MODE_SMART_HOST_PINNED) {
        HAL_TRACE_DEBUG("vmotion init");
        vm = vmotion::factory(HAL_VMOTION_MAX_THREADS,
                              stoi(hal_cfg->vmotion_port) ?
                              stoi(hal_cfg->vmotion_port) : HAL_VMOTION_PORT);
    }
    g_hal_state->set_vmotion(vm);
    return ret;
}

static inline hal_forwarding_mode_t
parse_forwarding_mode (std::string forwarding_mode)
{
    if (forwarding_mode == "FORWARDING_MODE_CLASSIC") {
        return HAL_FORWARDING_MODE_CLASSIC;
    } else if (forwarding_mode == "FORWARDING_MODE_SWITCH") {
        return HAL_FORWARDING_MODE_SMART_SWITCH;
    } else if (forwarding_mode == "FORWARDING_MODE_HOSTPIN") {
        return HAL_FORWARDING_MODE_SMART_HOST_PINNED;
    }
    return HAL_FORWARDING_MODE_CLASSIC;
}

static inline hal_feature_profile_t
parse_feature_profile (std::string feature_profile)
{
    if (feature_profile == "classic-default") {
        return HAL_FEATURE_PROFILE_CLASSIC_DEFAULT;
    } else if (feature_profile == "classic-eth-dev-scale") {
        return HAL_FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE;
    }
    return HAL_FEATURE_PROFILE_CLASSIC_DEFAULT;
}

//------------------------------------------------------------------------------
// initialize port control operations
//------------------------------------------------------------------------------
hal_ret_t
hal_device_cfg_init (hal_cfg_t *hal_cfg)
{
    device_cfg_t *device_cfg = &hal_cfg->device_cfg;
    sdk::lib::device *device = NULL;
    std::string device_cfg_path;

    if (hal_cfg->platform == platform_type_t::PLATFORM_TYPE_HW) {
        device_cfg_path = std::string(SYSCONFIG_PATH);
    } else {
        device_cfg_path = hal_cfg->cfg_path;
    }

    device = sdk::lib::device::factory(device_cfg_path + "/" + DEVICE_CFG_FNAME);
    SDK_ASSERT_TRACE_RETURN(device != NULL, HAL_RET_ERR, "Device conf file error");

    device_cfg->forwarding_mode = (hal::hal_forwarding_mode_t)device->get_forwarding_mode();
    device_cfg->feature_profile = (hal::hal_feature_profile_t)device->get_feature_profile();
    device_cfg->admin_state = device->get_port_admin_state() ?
        port_admin_state_t::PORT_ADMIN_STATE_DOWN : port_admin_state_t::PORT_ADMIN_STATE_UP;
    g_mgmt_if_mac = device->get_mgmt_if_mac();
    device_cfg->device_profile = device->device_profile();
    device_cfg->mgmt_vlan = device->get_mgmt_vlan() ? device->get_mgmt_vlan() : NATIVE_VLAN_ID;

    printf("Hal forwarding mode: %s, feature_profile: %d, port_admin_state: %d, mgmt_vlan: %d\n",
           FORWARDING_MODES_str(device_cfg->forwarding_mode),
           device_cfg->feature_profile,
           (int)device_cfg->admin_state,
           device_cfg->mgmt_vlan);
    return HAL_RET_OK;
}

}    // namespace hal
