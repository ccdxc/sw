// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "gen/hal/svc/vrf_svc_gen.hpp"
#include "gen/hal/svc/l2segment_svc_gen.hpp"
#include "gen/hal/svc/nw_svc_gen.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "gen/hal/svc/endpoint_svc_gen.hpp"
#include "gen/hal/svc/session_svc_gen.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/sdk/include/sdk/if.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace nw {

VrfServiceImpl           g_vrf_svc;
L2SegmentServiceImpl     g_l2seg_svc;
NetworkServiceImpl       g_nw_svc;
InterfaceServiceImpl     g_if_svc;
EndpointServiceImpl      g_endpoint_svc;
SessionServiceImpl       g_session_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "network" services
    HAL_TRACE_DEBUG("Registering gRPC network services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_vrf_svc);
        server_builder->RegisterService(&g_nw_svc);
        server_builder->RegisterService(&g_if_svc);
        server_builder->RegisterService(&g_l2seg_svc);
        server_builder->RegisterService(&g_session_svc);
        server_builder->RegisterService(&g_endpoint_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
        server_builder->RegisterService(&g_vrf_svc);
        server_builder->RegisterService(&g_l2seg_svc);
        server_builder->RegisterService(&g_if_svc);
        server_builder->RegisterService(&g_endpoint_svc);
    }
    HAL_TRACE_DEBUG("gRPC network services registered ...");
    return;
}

//------------------------------------------------------------------------------
// create CPU interface, this will be used by FTEs to receive packets from
// dataplane and to inject packets into the dataplane
//------------------------------------------------------------------------------
static inline hal_ret_t
hal_cpu_if_create (uint32_t lif_id)
{
    InterfaceSpec      spec;
    InterfaceResponse  response;
    hal_ret_t          ret;

    spec.mutable_key_or_handle()->set_interface_id(IF_ID_CPU);
    spec.set_type(::intf::IfType::IF_TYPE_CPU);
    spec.set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec.mutable_if_cpu_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = interface_create(spec, &response);
    if ((ret == HAL_RET_OK) || (ret == HAL_RET_ENTRY_EXISTS)) {
        HAL_TRACE_DEBUG("CPU interface {} create success, handle {}",
                        IF_ID_CPU, response.status().if_handle());
    } else {
        HAL_TRACE_ERR("CPU interface {} create failed, err : {}",
                      IF_ID_CPU, ret);
    }
    hal::hal_cfg_db_close();

    return HAL_RET_OK;
}

static hal_ret_t inline
hal_uplink_if_create (uint64_t if_id, 
                      uint32_t port_num,
                      bool is_oob)
{
    InterfaceSpec        spec;
    InterfaceResponse    response;
    hal_ret_t            ret;

    spec.mutable_key_or_handle()->set_interface_id(if_id);
    spec.set_type(::intf::IfType::IF_TYPE_UPLINK);
    spec.set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec.mutable_if_uplink_info()->set_port_num(port_num);
    spec.mutable_if_uplink_info()->set_is_oob_management(is_oob);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = interface_create(spec, &response);
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("Uplink interface {}/port {} create success, handle {}",
                        if_id, port_num, response.status().if_handle());
    } else {
        HAL_TRACE_ERR("Uplink interface {}/port {} create failed, err : {}",
                      if_id, port_num, ret);
    }
    hal::hal_cfg_db_close();

    return HAL_RET_OK;
}

static hal_ret_t hal_uplink_ifs_create (hal_cfg_t *hal_cfg)
{
    uint32_t ifindex;
    uint32_t fp_port;
    uint32_t if_id = NETAGENT_IF_ID_UPLINK_MIN;
    uint32_t logical_port;
    sdk::lib::catalog *catalog = hal_cfg->catalog;
    uint32_t num_fp_ports = catalog->num_fp_ports();

    for (fp_port = 1; fp_port <= num_fp_ports; ++fp_port) {
        ifindex = ETH_IFINDEX(catalog->slot(), fp_port, ETH_IF_DEFAULT_CHILD_PORT);
        logical_port = sdk::lib::catalog::ifindex_to_logical_port(ifindex);
        hal_uplink_if_create(if_id, logical_port,
                             (catalog->port_type_fp(fp_port) == port_type_t::PORT_TYPE_MGMT) ? true : false);
        if_id++;
    }

    return HAL_RET_OK;
}

static hal_ret_t
hal_fte_span_create()
{
    FteSpanRequest      req;
    FteSpanResponse     rsp;
    hal_ret_t           ret;

    req.set_selector(0);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte_span_create(req, &rsp);
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("FTE span create done.");
    } else {
        HAL_TRACE_DEBUG("FTE span create fail. err: {}", ret);
    }
    hal::hal_cfg_db_close();

    return HAL_RET_OK;
}

// initialization routine for network module
extern "C" hal_ret_t
init (hal_cfg_t *hal_cfg)
{
    hal_ret_t ret = HAL_RET_OK;

    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);

    // set the forwarding mode
    g_hal_state->set_forwarding_mode(hal_cfg->device_cfg.forwarding_mode);

    // set the mgmt vlan
    g_hal_state->set_mgmt_vlan(hal_cfg->device_cfg.mgmt_vlan);

    // default set to local switch prom. for DOLs to pass
    g_hal_state->set_allow_local_switch_for_promiscuous(true);

    // create cpu interface
    ret = hal_cpu_if_create(HAL_LIF_CPU);
    HAL_ABORT(ret == HAL_RET_OK);

    if (hal_cfg->platform == platform_type_t::PLATFORM_TYPE_HW) {
        // create uplink interfaces
        ret = hal_uplink_ifs_create(hal_cfg);
        HAL_ABORT(ret == HAL_RET_OK);
    }

    // Create FTE span. Only for iris pipeline
    if (hal_cfg->features == hal::HAL_FEATURE_SET_IRIS) {
        ret = hal_fte_span_create();
        HAL_ABORT(ret == HAL_RET_OK);
    }

    return HAL_RET_OK;
}

extern "C" void
nw_thread_init (int tid)
{
    // Periodic cleanup needs to be inited only
    // for config thread
    if (tid != HAL_THREAD_ID_PERIODIC)
        return;

    // Init periodic timer for session garbage collection
    if (hal::g_hal_cfg.features != hal::HAL_FEATURE_SET_GFT)
        HAL_ABORT(hal::session_init(&hal::g_hal_cfg) == HAL_RET_OK);

    return;
}

// cleanup routine for network module
extern "C" void
exit (void)
{
}

}    // namespace nw
}    // namespace hal
