// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/gen/hal/svc/acl_svc_gen.hpp"
#include "nic/gen/hal/svc/qos_svc_gen.hpp"
#include "nic/hal/svc/session_svc.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace hal {
namespace aclqos {

QOSServiceImpl           g_qos_svc;
AclServiceImpl           g_acl_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "network" services
    HAL_TRACE_DEBUG("Registering gRPC network services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_acl_svc);
        server_builder->RegisterService(&g_qos_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
        // Revisit. DOL was not able to create Lif without qos class
        server_builder->RegisterService(&g_qos_svc);
    }
    HAL_TRACE_DEBUG("gRPC network services registered ...");
    return;
}

// initialization routine for network module
extern "C" hal_ret_t
aclqos_init (hal_cfg_t *hal_cfg)
{
    svc_reg(hal_cfg->server_builder, hal_cfg->features);
    return HAL_RET_OK;
}

// cleanup routine for aclqos module
extern "C" void
aclqos_exit (void)
{
}

}    // namespace aclqos
}    // namespace hal
