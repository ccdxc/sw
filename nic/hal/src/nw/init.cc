// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/gen/hal/svc/vrf_svc_gen.hpp"
#include "nic/gen/hal/svc/l2segment_svc_gen.hpp"
#include "nic/gen/hal/svc/nw_svc_gen.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "nic/gen/hal/svc/endpoint_svc_gen.hpp"
#include "nic/hal/svc/session_svc.hpp"

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

// initialization routine for network module
extern "C" hal_ret_t
init (hal_cfg_t *hal_cfg)
{
    svc_reg(hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for network module
extern "C" void
exit (void)
{
}

}    // namespace nw
}    // namespace hal
