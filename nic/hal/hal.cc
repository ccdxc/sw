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
#include "nic/hal/src/tcp_proxy_cb.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/include/fte.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/hal/plugins/plugins.hpp"
#include "nic/hal/src/utils.hpp"
#include "sdk/logger.hpp"
#include "sdk/utils.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/qos.hpp"
#include "nic/hal/src/acl.hpp"
#include <google/protobuf/util/json_util.h>
#include "nic/hal/src/nwsec_group.hpp"

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

namespace hal {

// process globals
thread   *g_hal_threads[HAL_THREAD_ID_MAX];
bool     gl_super_user = false;

// TODO_CLEANUP: THIS DOESN'T BELONG HERE !!
LIFManager *g_lif_manager = nullptr;

// thread local variables
thread_local cfg_db_ctxt_t t_cfg_db_ctxt;

thread_local void *g_clock_delta_timer;

using boost::property_tree::ptree;

static thread *
current_thread (void)
{
    return sdk::lib::thread::current_thread() ?
               sdk::lib::thread::current_thread() :
               g_hal_threads[HAL_THREAD_ID_CFG];
}

static void *
fte_pkt_loop_start (void *ctxt)
{
    SDK_THREAD_INIT(ctxt);

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
    if (utils::hal_logger()) {
        utils::hal_logger()->flush();
    }

    switch (sig) {
    case SIGINT:
    case SIGTERM:
        HAL_GCOV_FLUSH();
        if (utils::hal_logger()) {
            utils::hal_logger()->flush();
        }
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        if (utils::hal_logger()) {
            utils::hal_logger()->flush();
        }
        break;

    case SIGHUP:
    case SIGQUIT:
    case SIGCHLD:
    case SIGURG:
    default:
        if (utils::hal_logger()) {
            utils::hal_logger()->flush();
        }
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
hal_thread_init (hal_cfg_t *hal_cfg)
{
    uint32_t            tid;
    int                 rv, thread_prio;
    char                thread_name[16];
    struct sched_param  sched_param = { 0 };
    pthread_attr_t      attr;
    uint64_t            data_cores_mask = hal_cfg->data_cores_mask;
    uint64_t            cores_mask = 0x0;

    // spawn data core threads and pin them to their cores
    thread_prio = sched_get_priority_max(SCHED_FIFO);
    assert(thread_prio >= 0);

    for (uint32_t i = 0; i < hal_cfg->num_data_threads; i++) {

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

    // spawn periodic thread that does background tasks
    g_hal_threads[HAL_THREAD_ID_PERIODIC] =
        thread::factory(std::string("periodic-thread").c_str(),
                        HAL_THREAD_ID_PERIODIC,
                        sdk::lib::THREAD_ROLE_CONTROL,
                        0x0 /* use all control cores */,
                        hal_periodic_loop_start,
                        thread_prio - 1,
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
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    uint64_t mask = sdk::lib::thread::control_cores_mask();
    while (mask != 0) {
        CPU_SET(ffsl(mask) - 1, &cpus);
        mask = mask & (mask - 1);
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

//------------------------------------------------------------------------------
// parse HAL .ini file
//------------------------------------------------------------------------------
hal_ret_t
hal_parse_ini (const char *inifile, hal_cfg_t *hal_cfg) 
{
    std::string         ini_file, line;

    // check if ini file exists
    ini_file = hal_cfg->cfg_path + "/" + std::string(inifile);
    if (access(ini_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "HAL ini file %s doesn't exist or not accessible,"
                "picking defaults. mode: smart-switch",
                ini_file.c_str());
        hal_cfg->forwarding_mode = "smart-switch";
        return HAL_RET_OK;
    }

    std::ifstream in(ini_file.c_str());
    if (!in) {
        HAL_TRACE_ERR("Failed to open ini file ... "
                      "setting forwarding mode to smart-switch");
        hal_cfg->forwarding_mode = "smart-switch";
        return HAL_RET_OK;
    }

    while (std::getline(in, line)) {
        std::string key = line.substr(0, line.find("="));
        std::string val = line.substr(line.find("=")+1, line.length()-1);

        if (key == "forwarding_mode") {
            if ((val != "smart-switch") && (val != "smart-host-pinned") &&
                (val != "classic")) {
                HAL_TRACE_ERR("Invalid forwarding mode : {}, aborting ...",
                              val);
                HAL_ABORT(0);
            }
            hal_cfg->forwarding_mode = val;
            HAL_TRACE_DEBUG("NIC forwarding mode : {}", val);
        }
    }
    in.close();

    return HAL_RET_OK;
}

static hal_ret_t
hal_parse_thread_cfg (ptree &pt, hal_cfg_t *hal_cfg)
{
    std::string str = "";

    str = pt.get<std::string>("sw.control_cores_mask");
    hal_cfg->control_cores_mask = std::stoul (str, nullptr, 16);
    sdk::lib::thread::control_cores_mask_set(hal_cfg->control_cores_mask);
    hal_cfg->num_control_threads =
                    sdk::lib::set_bits_count(hal_cfg->control_cores_mask);

    str = pt.get<std::string>("sw.data_cores_mask");
    hal_cfg->data_cores_mask = std::stoul (str, nullptr, 16);
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
        hal_cfg->hbm_cache =
                pt.get<std::string>("asic.hbm_cache", "true");

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
        } else {
            hal_cfg->features = HAL_FEATURE_SET_NONE;
            HAL_TRACE_ERR("Uknown feature set {}", sparam.c_str());
        }
        strncpy(hal_cfg->feature_set, sparam.c_str(), HAL_MAX_NAME_STR);

        // parse threads config
        hal_parse_thread_cfg(pt, hal_cfg);

    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

int
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

int
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
static inline hal_ret_t
hal_sdk_init (void)
{
    sdk::lib::logger::init(hal_sdk_error_logger, hal_sdk_debug_logger);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// create CPU interface, this will be used by FTEs to receive packets from
// dataplane and to inject packets into the dataplane
//------------------------------------------------------------------------------
hal_ret_t
hal_cpu_if_create (uint32_t lif_id)
{
    InterfaceSpec      spec;
    InterfaceResponse  response;
    hal_ret_t          ret = HAL_RET_OK;

    spec.mutable_key_or_handle()->set_interface_id(IF_ID_CPU);
    spec.set_type(::intf::IfType::IF_TYPE_CPU);
    spec.set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec.mutable_if_cpu_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    ret = interface_create(spec, &response);
    if (ret == HAL_RET_OK) {
        HAL_TRACE_ERR("{}: CPU if create success, handle = {}",
                      __FUNCTION__, response.status().if_handle());
    } else {
        HAL_TRACE_DEBUG("{}: CPU if create failed", __FUNCTION__);
    }

    hal::hal_cfg_db_close();

    return HAL_RET_OK;
}

static hal_ret_t
hal_cores_validate (uint64_t sys_core,
                    uint64_t control_core,
                    uint64_t data_core)
{
    if ((control_core & data_core) != 0) {
        HAL_TRACE_ERR("control core mask 0x{0:x} overlaps with"
                      " data core mask 0x{1:x}",
                      control_core, data_core);
        return HAL_RET_ERR;
    }

    if ((sys_core & (control_core | data_core)) !=
                    (control_core | data_core)) {
        HAL_TRACE_ERR("control core mask 0x{0:x} and data core mask 0x{1:x}"
                      " does not match or exceeds system core mask 0x{2:x}",
                      control_core, data_core, sys_core);
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

static hal_ret_t
hal_qos_config_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t          ret = HAL_RET_OK;
    sdk_ret_t          sdk_ret;
    QosClassRequestMsg qos_class_request;
    QosClassResponse   qos_class_rsp;
    CoppRequestMsg     copp_request;
    CoppResponse       copp_rsp;

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    // Qos class
    std::string qos_class_configs;
    std::string copp_configs;

    sdk_ret = sdk::lib::catalog::get_child_str(hal_cfg->catalog_file, 
                                               "qos.configs.qos_class", 
                                               qos_class_configs);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting qos_class configs from catalog: ret {}", ret);
        return ret;
    }

    sdk_ret = sdk::lib::catalog::get_child_str(hal_cfg->catalog_file, 
                                               "qos.configs.copp", 
                                               copp_configs);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting copp configs from catalog: ret {}", ret);
        return ret;
    }

    google::protobuf::util::JsonStringToMessage(qos_class_configs, &qos_class_request);
    for (int i = 0; i < qos_class_request.request_size(); i++) {
        auto spec = qos_class_request.request(i);
        ret = qosclass_create(spec, &qos_class_rsp);
        if (qos_class_rsp.api_status() != types::API_STATUS_OK) {
            HAL_TRACE_ERR("Error  creating qos class ret: {}",
                          ret);
            goto end;
        }
    }

    google::protobuf::util::JsonStringToMessage(copp_configs, &copp_request);
    for (int i = 0; i < copp_request.request_size(); i++) {
        auto spec = copp_request.request(i);
        ret = copp_create(spec, &copp_rsp);
        if (copp_rsp.api_status() != types::API_STATUS_OK) {
            HAL_TRACE_ERR("Error  creating copp ret: {}",
                          ret);
            goto end;
        }
    }

end:
    hal::hal_cfg_db_close();
    return ret;
}

static hal_ret_t
hal_smart_nic_acl_config_init (void)
{
    hal_ret_t     ret;
    AclSpec       spec;
    AclResponse   rsp;
    AclSelector   *match; 
    AclActionInfo *action;

    if (g_hal_state->forwarding_mode() != HAL_FORWARDING_MODE_SMART_SWITCH) {
        HAL_TRACE_DEBUG("Skipping smart nic acls");
        return HAL_RET_OK;
    } 

    // Drop IP fragmented packets
    spec.Clear();
    match = spec.mutable_match();
    action = spec.mutable_action();

    spec.mutable_key_or_handle()->set_acl_id(ACL_IP_FRAGMENT_DROP_ENTRY_ID);
    spec.set_priority(ACL_IP_FRAGMENT_DROP_ENTRY_PRIORITY);

    match->mutable_internal_key()->set_ip_frag(true);
    match->mutable_internal_mask()->set_ip_frag(true);

    action->set_action(acl::AclAction::ACL_ACTION_DENY);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("IP Fragment drop acl entry create failed ret {}", ret);
        goto end;
    }

    HAL_TRACE_DEBUG("IP fragment drop acl entry created");

end:
    return ret;
}

static hal_ret_t
hal_eplearn_acl_config_init (void)
{
    hal_ret_t     ret;
    AclSpec       spec;
    AclResponse   rsp;
    AclSelector   *match;
    AclActionInfo *action;
    AclActionInfo arp_action;
    AclActionInfo dhcp_action;
    uint32_t      acl_id;
    uint32_t      priority;

    arp_action.Clear();
    arp_action.set_action(acl::AclAction::ACL_ACTION_REDIRECT);
    arp_action.mutable_redirect_if_key_handle()->set_interface_id(IF_ID_CPU);
    arp_action.mutable_internal_actions()->set_qid(types::CPUCB_ID_FLOWMISS);
    arp_action.mutable_internal_actions()->set_qid_valid(true);
    arp_action.mutable_copp_key_handle()->set_copp_type(kh::COPP_TYPE_ARP);

    dhcp_action.Clear();
    dhcp_action.set_action(acl::AclAction::ACL_ACTION_REDIRECT);
    dhcp_action.mutable_redirect_if_key_handle()->set_interface_id(IF_ID_CPU);
    dhcp_action.mutable_internal_actions()->set_qid(types::CPUCB_ID_FLOWMISS);
    dhcp_action.mutable_internal_actions()->set_qid_valid(true);
    dhcp_action.mutable_copp_key_handle()->set_copp_type(kh::COPP_TYPE_DHCP);

    acl_id = ACL_EPLEARN_ENTRY_ID_BEGIN;
    priority = ACL_EPLEARN_ENTRY_PRIORITY_BEGIN;
    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = arp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        match->mutable_eth_selector()->set_eth_type(ETHERTYPE_ARP);
        match->mutable_eth_selector()->set_eth_type_mask(0xffff);
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);

        ret = hal::acl_create(spec, &rsp);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = dhcp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        acl::IPSelector *ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        ip_selector->mutable_icmp_selector()->set_icmp_code(0);
        ip_selector->mutable_icmp_selector()->set_icmp_code_mask(0xff);
        ip_selector->mutable_icmp_selector()->set_icmp_type(ICMP_NEIGHBOR_SOLICITATION);
        ip_selector->mutable_icmp_selector()->set_icmp_type_mask(0xff);
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);

        ret = hal::acl_create(spec, &rsp);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = dhcp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        acl::IPSelector *ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        ip_selector->mutable_icmp_selector()->set_icmp_code(0);
        ip_selector->mutable_icmp_selector()->set_icmp_code_mask(0xff);
        ip_selector->mutable_icmp_selector()->set_icmp_type(ICMP_NEIGHBOR_ADVERTISEMENT);
        ip_selector->mutable_icmp_selector()->set_icmp_type_mask(0xff);
        match->mutable_internal_key()->set_flow_miss(true);
        match->mutable_internal_mask()->set_flow_miss(true);

        ret = hal::acl_create(spec, &rsp);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = dhcp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        acl::IPSelector *ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        ip_selector->mutable_udp_selector()->mutable_dst_port_range()->set_port_low(DHCP_CLIENT_PORT);
        ip_selector->mutable_udp_selector()->mutable_dst_port_range()->set_port_high(DHCP_CLIENT_PORT);

        ret = hal::acl_create(spec, &rsp);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();
        *action = dhcp_action;
        spec.mutable_key_or_handle()->set_acl_id(acl_id++);
        spec.set_priority(priority++);

        acl::IPSelector *ip_selector = match->mutable_ip_selector();
        ip_selector->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        ip_selector->mutable_udp_selector()->mutable_dst_port_range()->set_port_low(DHCP_SERVER_PORT);
        ip_selector->mutable_udp_selector()->mutable_dst_port_range()->set_port_high(DHCP_SERVER_PORT);

        ret = hal::acl_create(spec, &rsp);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    return ret;
}

static hal_ret_t
hal_acl_config_init (void)
{
    hal_ret_t ret;
    AclSpec       spec;
    AclResponse   rsp;
    AclSelector   *match; 
    AclActionInfo *action;

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_SWITCH) {
        ret = hal_smart_nic_acl_config_init ();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error creating smart nic acl entries ret {}", ret);
            goto end;
        }
    }

    {
        spec.Clear();
        match = spec.mutable_match();
        action = spec.mutable_action();

        spec.mutable_key_or_handle()->set_acl_id(ACL_QUIESCE_ENTRY_ID);
        spec.set_priority(ACL_QUIESCE_ENTRY_PRIORITY);

        match->mutable_eth_selector()->set_src_mac(0x00eeff000004);
        match->mutable_eth_selector()->set_src_mac_mask(0xffffffffffff);
        match->mutable_eth_selector()->set_dst_mac(0x00eeff000005);
        match->mutable_eth_selector()->set_dst_mac_mask(0xffffffffffff);
        match->mutable_eth_selector()->set_eth_type(0xaaaa);
        match->mutable_eth_selector()->set_eth_type_mask(0xffff);
        match->mutable_internal_key()->set_from_cpu(true);
        match->mutable_internal_mask()->set_from_cpu(true);

        action->set_action(acl::AclAction::ACL_ACTION_REDIRECT);
        action->mutable_redirect_if_key_handle()->set_interface_id(IF_ID_CPU);
        action->mutable_internal_actions()->set_qid(types::CPUCB_ID_QUIESCE);
        action->mutable_internal_actions()->set_qid_valid(true);
        action->mutable_copp_key_handle()->set_copp_type(kh::COPP_TYPE_FLOW_MISS);

        ret = hal::acl_create(spec, &rsp);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Quiesce acl entry create failed ret {}", ret);
            goto end;
        }

        HAL_TRACE_DEBUG("Quiesce acl entry created");
    }

    ret = hal_eplearn_acl_config_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Eplearn acl entry create failed ret {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Eplearn acl entry created");

end:
    hal::hal_cfg_db_close();
    return ret;
}

//------------------------------------------------------------------------------
// initialize HAL logging
//------------------------------------------------------------------------------
static hal_ret_t
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
    hal::utils::logger_init(ffsl(sdk::lib::thread::control_cores_mask()) - 1,
                            hal_cfg->sync_mode_logging, logfile);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// init function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_init (hal_cfg_t *hal_cfg)
{
    int                  tid;
    char                 *user = NULL;
    hal_ret_t            ret;
    sdk::lib::catalog    *catalog;

    // initialize the logger
    if (hal_logger_init(hal_cfg) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize HAL logger, ignoring ...");
    }
    HAL_TRACE_DEBUG("Initializing HAL ...");
    srand(time(NULL));

    // do SDK initialization, if any
    hal_sdk_init();

    // do memory related initialization
    HAL_ABORT(hal_mem_init() == HAL_RET_OK);

    catalog = sdk::lib::catalog::factory(hal_cfg->catalog_file);
    HAL_ASSERT(catalog != NULL);
    g_hal_state->set_catalog(catalog);

    // validate control/data cores against catalog
    HAL_ABORT(hal_cores_validate(catalog->cores_mask(),
                                 hal_cfg->control_cores_mask,
                                 hal_cfg->data_cores_mask) == HAL_RET_OK);

    // initialize config parameters from the JSON file
    HAL_ABORT(hal_cfg_init(hal_cfg) == HAL_RET_OK);

    // init fte and hal plugins
    hal::init_plugins(hal_cfg);

    // check to see if HAL is running with root permissions
    user = getenv("USER");
    if (user && !strcmp(user, "root")) {
        gl_super_user = true;
    }

    // spawn all necessary PI threads
    HAL_ABORT(hal_thread_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Spawned all HAL threads");

    // do platform dependent init
    HAL_ABORT(hal::pd::hal_pd_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Platform initialization done");

    // do per module initialization
    // TODO: needed only in smart nic mode
    HAL_ABORT(hal::session_init() == HAL_RET_OK);


    // TODO_CLEANUP: this doesn't belong here, why is this outside
    // hal_state ??? how it this special compared to other global state ??
    g_lif_manager = new LIFManager();

    // Allocate LIF 0, so that we don't use it later
    int32_t hw_lif_id = g_lif_manager->LIFRangeAlloc(-1, 1);
    HAL_TRACE_DEBUG("Allocated hw_lif_id:{}", hw_lif_id);

    // do rdma init
    HAL_ABORT(rdma_hal_init() == HAL_RET_OK);

    if (!getenv("CAPRI_MOCK_MODE") && 
        !getenv("DISABLE_FTE") &&
        !(hal_cfg->forwarding_mode == "classic")) {
        // start fte threads
        for (uint32_t i = 0; i < hal_cfg->num_data_threads; i++) {
            tid = HAL_THREAD_ID_FTE_MIN + i;
            g_hal_threads[tid]->start(g_hal_threads[tid]);
        }
    }

    // do proxy init
    if (hal_cfg->features == HAL_FEATURE_SET_IRIS) {
        hal_proxy_svc_init();
    }

    // create cpu interface
    hal_cpu_if_create(SERVICE_LIF_CPU);

    // do qos creates 
    ret = hal_qos_config_init(hal_cfg);
    HAL_ABORT(ret == HAL_RET_OK);

    // TODO acls need cpu interface. Right now, in GFT mode, the
    // hal_cpu_if_create() fails, so cpu interface is not created
    // So skipping installation of acls
    if (hal_cfg->features != HAL_FEATURE_SET_GFT) {
        // do acl creates after qos creates. acls depend on qos config
        ret = hal_acl_config_init();
        HAL_ABORT(ret == HAL_RET_OK);
    }

    // install signal handlers
    hal_sig_init();

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

}    // namespace hal
