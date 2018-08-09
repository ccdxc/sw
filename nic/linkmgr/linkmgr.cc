// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "grpc++/grpc++.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "sdk/pal.hpp"
#include "linkmgr_src.hpp"
#include "linkmgr_svc.hpp"
#include "linkmgr_state.hpp"
#include "nic/linkmgr/utils.hpp"
#include "linkmgr_utils.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using boost::property_tree::ptree;
using sdk::types::platform_type_t;

namespace linkmgr {

linkmgr_cfg_t linkmgr_cfg;

// TODO required?
extern class linkmgr_state *g_linkmgr_state;

extern sdk::lib::catalog* catalog (void);

static uint32_t
num_uplink_ports (void)
{
    return catalog()->num_uplink_ports();
}

static port_speed_t
port_speed (uint32_t port)
{
    return catalog()->port_speed(port);
}

static port_type_t
port_type (uint32_t port)
{
    return catalog()->port_type(port);
}

static uint32_t
num_lanes (uint32_t port)
{
    return catalog()->num_lanes(port);
}

static bool
enabled (uint32_t port)
{
    return catalog()->enabled(port);
}

static uint32_t
mac_id (uint32_t port, uint32_t lane)
{
    return catalog()->mac_id(port, lane);
}

static uint32_t
mac_ch (uint32_t port, uint32_t lane)
{
    return catalog()->mac_ch(port, lane);
}

static platform_type_t
platform_type (void)
{
    return linkmgr_cfg.platform_type;
}

static hal_ret_t
linkmgr_uplink_create(uint32_t uplink_port)
{
    hal_ret_t     ret  = HAL_RET_OK;
    PortSpec      spec;
    PortResponse  response;

    spec.mutable_key_or_handle()->set_port_id(uplink_port);
    spec.set_port_speed(linkmgr::sdk_port_speed_to_port_speed_spec(port_speed(uplink_port)));
    spec.set_num_lanes(num_lanes(uplink_port));
    spec.set_port_type(linkmgr::sdk_port_type_to_port_type_spec(port_type(uplink_port)));
    spec.set_mac_id(mac_id(uplink_port, 0));
    spec.set_mac_ch(mac_ch(uplink_port, 0));

    if (enabled(uplink_port) == true) {
        spec.set_admin_state(::port::PORT_ADMIN_STATE_UP);
    }

    HAL_TRACE_DEBUG("creating uplink port {}",  uplink_port);

    linkmgr::g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);

    ret = linkmgr::port_create(spec, &response);

    linkmgr::g_linkmgr_state->cfg_db_close();

    return ret;
}

//------------------------------------------------------------------------------
// create uplink ports in the catalog file
//------------------------------------------------------------------------------
static hal_ret_t
linkmgr_uplinks_create()
{
    uint32_t  uplink_port = 0;
    hal_ret_t ret = HAL_RET_OK;

    for (uplink_port = 1; uplink_port <= num_uplink_ports();
                                             ++uplink_port) {
        ret = linkmgr_uplink_create(uplink_port);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error creating uplink port {}",
                            uplink_port);
        }
    }

    return ret;
}

static void
svc_reg (const std::string& server_addr)
{
    PortServiceImpl   port_svc;
    ServerBuilder     server_builder;

    // listen on the given address (no authentication)
    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    // register all services
    server_builder.RegisterService(&port_svc);

    HAL_TRACE_DEBUG("gRPC server listening on ... {}", server_addr.c_str());

    // assemble the server
    std::unique_ptr<Server> server(server_builder.BuildAndStart());

    hal::utils::hal_logger()->flush();

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

hal_ret_t
linkmgr_global_init (void)
{
    hal_ret_t                    ret_hal   = HAL_RET_OK;
    std::string                  cfg_file  = "linkmgr.json";
    char                         *cfg_path = NULL;
    sdk::lib::catalog            *catalog;
    sdk::linkmgr::linkmgr_cfg_t  sdk_cfg;

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        cfg_file = std::string(cfg_path) + "/" + cfg_file;
    } else {
        HAL_ASSERT(FALSE);
    }

    linkmgr_parse_cfg(cfg_file.c_str(), &linkmgr_cfg);

    catalog =
        sdk::lib::catalog::factory(std::string(cfg_path) + "/catalog.json");
    HAL_ASSERT_RETURN((catalog != NULL), HAL_RET_ERR);

    if (sdk::lib::pal_init(platform_type()) != sdk::lib::PAL_RET_OK) {
        HAL_TRACE_ERR("pal init failed");
        return HAL_RET_ERR;
    }

    sdk_cfg.platform_type = platform_type();
    sdk_cfg.cfg_path = cfg_path;
    sdk_cfg.catalog  = catalog;

    ret_hal = linkmgr::linkmgr_init(&sdk_cfg);
    if (ret_hal != HAL_RET_OK) {
        HAL_TRACE_ERR("linkmgr init failed");
        return HAL_RET_ERR;
    }

    // must be done after sdk linkmgr init
    ret_hal = linkmgr_uplinks_create();
    if (ret_hal != HAL_RET_OK) {
        HAL_TRACE_ERR("uplinks create failed");
    }

    // start the linkmgr control thread
    // TODO move to linkmgr_src.cc
    sdk::linkmgr::linkmgr_event_wait();

    // register for all gRPC services
    svc_reg(std::string("localhost:") + linkmgr_cfg.grpc_port);

    return ret_hal;
}

}    // namespace linkmgr

