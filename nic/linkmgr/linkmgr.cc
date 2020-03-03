// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "grpc++/grpc++.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "linkmgr_src.hpp"
#include "linkmgr_svc.hpp"
#include "linkmgr_debug_svc.hpp"
#include "linkmgr_state.hpp"
#include "nic/linkmgr/utils.hpp"
#include "linkmgr_utils.hpp"
#include "lib/periodic/periodic.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "third-party/asic/capri/model/utils/cap_csr_py_if.h"
#include "nic/sdk/platform/csr/asicrw_if.hpp"
#include "nic/linkmgr/delphi/linkmgr_delphi.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using boost::property_tree::ptree;
using sdk::linkmgr::linkmgr_thread_id_t;
using hal::CFG_OP_WRITE;
using hal::utils::hal_logger;

namespace linkmgr {

sdk::lib::thread *g_linkmgr_threads[linkmgr_thread_id_t::LINKMGR_THREAD_ID_MAX];

// TODO required?
extern class linkmgr_state *g_linkmgr_state;

extern sdk::lib::catalog* catalog (void);

static void
svc_wait (ServerBuilder *server_builder)
{
    // assemble the server
    std::unique_ptr<Server> server(server_builder->BuildAndStart());

    hal_logger()->flush();

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

    HAL_TRACE_DEBUG("cfg file {}",  cfgfile);

    std::ifstream json_cfg(cfgfile);

    read_json(json_cfg, pt);

    try {
		std::string platform_type = pt.get<std::string>("platform_type");

        linkmgr_cfg->platform_type =
            sdk::lib::catalog::catalog_platform_type_to_platform_type(platform_type);

        linkmgr_cfg->grpc_port = pt.get<std::string>("sw.grpc_port");

        if (getenv("HAL_GRPC_PORT")) {
            linkmgr_cfg->grpc_port = getenv("HAL_GRPC_PORT");
            HAL_TRACE_DEBUG("Overriding GRPC Port to {}", linkmgr_cfg->grpc_port);
        }
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

static void*
linkmgr_periodic_start (void* ctxt)
{
    if (sdk::lib::periodic_thread_init(ctxt) == NULL) {
        SDK_TRACE_ERR("Failed to init timer");
    }

    sdk::lib::periodic_thread_run(ctxt);

    return NULL;
}

hal_ret_t
linkmgr_thread_init (void)
{
    int    thread_prio = 0, thread_id = 0;

    thread_prio = sched_get_priority_max(SCHED_OTHER);
    if (thread_prio < 0) {
        return HAL_RET_ERR;
    }

    // spawn periodic thread that does background tasks
    thread_id = linkmgr_thread_id_t::LINKMGR_THREAD_ID_PERIODIC;
    g_linkmgr_threads[thread_id] =
        sdk::lib::thread::factory(
                        std::string("linkmgr-periodic").c_str(),
                        thread_id,
                        sdk::lib::THREAD_ROLE_CONTROL,
                        0x0 /* use all control cores */,
                        linkmgr_periodic_start,
                        thread_prio - 1,
                        SCHED_OTHER,
                        true);
    if (g_linkmgr_threads[thread_id] == NULL) {
        SDK_TRACE_ERR("Failed to create linkmgr periodic thread");
        return HAL_RET_ERR;
    }

    // start the periodic thread
    g_linkmgr_threads[thread_id]->start(g_linkmgr_threads[thread_id]);

    // create a thread object for CFG thread
    thread_id = linkmgr_thread_id_t::LINKMGR_THREAD_ID_CFG;
    g_linkmgr_threads[thread_id] =
        sdk::lib::thread::factory(std::string("linkmgr-cfg").c_str(),
                                  thread_id,
                                  sdk::lib::THREAD_ROLE_CONTROL,
                                  0x0 /* use all control cores */,
                                  sdk::lib::thread::dummy_entry_func,
                                  thread_prio -1,
                                  SCHED_OTHER,
                                  true);
    g_linkmgr_threads[thread_id]->set_data(g_linkmgr_threads[thread_id]);
    g_linkmgr_threads[thread_id]->set_pthread_id(pthread_self());
    g_linkmgr_threads[thread_id]->set_running(true);

    return HAL_RET_OK;
}

hal_ret_t
linkmgr_csr_init (void)
{
    // register hal cpu interface
    auto cpu_if = new cpu_hal_if("cpu", "all");
    cpu::access()->add_if("cpu_if", cpu_if);
    cpu::access()->set_cur_if_name("cpu_if");

    // Register at top level all MRL classes.
    cap_top_csr_t *cap0_ptr = new cap_top_csr_t("cap0");

    cap0_ptr->init(0);
    CAP_BLK_REG_MODEL_REGISTER(cap_top_csr_t, 0, 0, cap0_ptr);
    register_chip_inst("cap0", 0, 0);

    return HAL_RET_OK;
}

hal_ret_t
linkmgr_global_init (linkmgr_cfg_t *linkmgr_cfg)
{
    hal_ret_t          ret_hal       = HAL_RET_OK;
    std::string        cfg_file      = linkmgr_cfg->cfg_file;
    std::string        catalog_file  = linkmgr_cfg->catalog_file;
    char               *cfg_path     = NULL;
    sdk::lib::catalog  *catalog      = NULL;
    ServerBuilder      server_builder;

    sdk::linkmgr::linkmgr_cfg_t sdk_cfg;

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");

    if (cfg_path) {
        cfg_file     = std::string(cfg_path) + "/" + cfg_file;
        catalog_file = std::string(cfg_path) + "/" + catalog_file;
    } else {
        SDK_ASSERT(FALSE);
    }

    linkmgr_parse_cfg(cfg_file.c_str(), linkmgr_cfg);

    catalog = sdk::lib::catalog::factory(cfg_path, catalog_file);

    SDK_ASSERT_RETURN((catalog != NULL), HAL_RET_ERR);

    if (sdk::lib::pal_init(linkmgr_cfg->platform_type) !=
                                            sdk::lib::PAL_RET_OK) {
        HAL_TRACE_ERR("pal init failed");
        return HAL_RET_ERR;
    }

    // listen on the given address (no authentication)
    std::string server_addr = std::string("localhost:") + linkmgr_cfg->grpc_port;

    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    linkmgr_thread_init();

    sdk_cfg.platform_type  = linkmgr_cfg->platform_type;
    sdk_cfg.cfg_path       = cfg_path;
    sdk_cfg.catalog        = catalog;
    sdk_cfg.server_builder = &server_builder;
    sdk_cfg.process_mode   = true;

    linkmgr_csr_init();

    ret_hal = linkmgr::linkmgr_init(&sdk_cfg);
    if (ret_hal != HAL_RET_OK) {
        HAL_TRACE_ERR("linkmgr init failed");
        return HAL_RET_ERR;
    }

    // register for all gRPC services
    HAL_TRACE_DEBUG("gRPC server listening on ... {}", server_addr.c_str());
    svc_wait(&server_builder);

    return ret_hal;
}

}    // namespace linkmgr

