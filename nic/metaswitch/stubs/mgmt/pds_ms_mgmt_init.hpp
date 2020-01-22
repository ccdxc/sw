// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane

#ifndef __PDS_MS_MGMT_INIT_HPP__
#define __PDS_MS_MGMT_INIT_HPP__

#include "nic/apollo/agent/svc/service.hpp"

bool pds_ms_mgmt_init();

namespace pds_ms {
void mgmt_svc_init (grpc::ServerBuilder* server_builder);
}

#endif
