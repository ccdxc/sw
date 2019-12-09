//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/hal.hpp"

#include "nic/hal/svc/debug_svc.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/hal/svc/proxy_svc.hpp"
#include "nic/hal/svc/nic_svc.hpp"
#include "nic/hal/svc/hal_ext.hpp"

#include "gen/hal/svc/vrf_svc_gen.hpp"
#include "gen/hal/svc/l2segment_svc_gen.hpp"
#include "gen/hal/svc/nw_svc_gen.hpp"
#include "gen/hal/svc/rdma_svc_gen.hpp"
#include "gen/hal/svc/nvme_svc_gen.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "gen/hal/svc/endpoint_svc_gen.hpp"
#include "gen/hal/svc/session_svc_gen.hpp"
#include "gen/hal/svc/telemetry_svc_gen.hpp"
#include "gen/hal/svc/internal_svc_gen.hpp"
#include "gen/hal/svc/nwsec_svc_gen.hpp"
#include "gen/hal/svc/qos_svc_gen.hpp"
#include "gen/hal/svc/acl_svc_gen.hpp"
#include "gen/hal/svc/ipsec_svc_gen.hpp"
#include "gen/hal/svc/cpucb_svc_gen.hpp"
#include "gen/hal/svc/tcp_proxy_svc_gen.hpp"
#include "gen/hal/svc/multicast_svc_gen.hpp"
#include "gen/hal/svc/l4lb_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

void
svc_reg (hal::hal_cfg_t *hal_cfg)
{
    ServerBuilder            *server_builder = (ServerBuilder *)hal_cfg->server_builder;
    VrfServiceImpl           vrf_svc;
    NetworkServiceImpl       nw_svc;
    InterfaceServiceImpl     if_svc;
    InternalServiceImpl      internal_svc;
    L2SegmentServiceImpl     l2seg_svc;
    DebugServiceImpl         debug_svc;
    TableServiceImpl         table_svc;
    NicServiceImpl           nic_svc;
    SessionServiceImpl       session_svc;
    EndpointServiceImpl      endpoint_svc;
    NwSecurityServiceImpl    nwsec_svc;
    QOSServiceImpl           qos_svc;
    AclServiceImpl           acl_svc;
    TelemetryServiceImpl     telemetry_svc;
    ProxyServiceImpl         proxy_svc;
    IpsecServiceImpl         ipsec_svc;
    CpuCbServiceImpl         cpucb_svc;
    TcpProxyServiceImpl      tcp_proxy_svc;
    EventServiceImpl         event_svc;
    MulticastServiceImpl     multicast_svc;
    SystemServiceImpl        system_svc;
    SoftwarePhvServiceImpl   swphv_svc;
    L4LbServiceImpl          l4lb_svc;
    //DosServiceImpl           dos_svc;

    HAL_TRACE_DEBUG("Bringing gRPC server for all API services ...");
    // register all services
    server_builder->RegisterService(&internal_svc);
    server_builder->RegisterService(&debug_svc);
    server_builder->RegisterService(&table_svc);
    server_builder->RegisterService(&nic_svc);
    server_builder->RegisterService(&proxy_svc);
    //server_builder->RegisterService(&ipsec_svc);
    server_builder->RegisterService(&cpucb_svc);
    server_builder->RegisterService(&tcp_proxy_svc);
    server_builder->RegisterService(&event_svc);
    server_builder->RegisterService(&system_svc);
    server_builder->RegisterService(&swphv_svc);

    HAL_TRACE_DEBUG("gRPC server listening on ... {}",
                    g_grpc_server_addr.c_str());
    hal::utils::hal_logger()->flush();
    HAL_SYSLOG_INFO("HAL-STATUS:UP");

    // notify sysmgr that we are up
    hal::svc::hal_init_done();
    hal::svc::set_hal_status(hal::HAL_STATUS_UP);

    // assemble the server
    std::unique_ptr<Server> server(server_builder->BuildAndStart());

    // wait for server to shutdown (some other thread must be responsible for
    // shutting down the server or else this call won't return)
    server->Wait();
}
