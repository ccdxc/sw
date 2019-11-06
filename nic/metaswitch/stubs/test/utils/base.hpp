//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __BASE_HPP__
#define __BASE_HPP__

#include <iostream>
#include <getopt.h>
#include <stdarg.h>
#include <gtest/gtest.h>
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/policy.hpp"
#include "nic/apollo/test/utils/route.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/svc_mapping.hpp"
#include "nic/apollo/test/utils/tag.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/vpc_peer.hpp"

namespace api_test {

// Global Test params
typedef struct pdsa_test_params_s {
    device_feeder       device;
    nexthop_feeder      nh;
    policy_feeder       policy;
    route_table_feeder  route_table;
    subnet_feeder       subnet;
    svc_mapping_feeder  svc_mapping;
    tag_feeder          tag;
    tep_feeder          tep;
    vnic_feeder         vnic;
    vpc_feeder          vpc;
    vpc_peer_feeder     vpc_peer;
} pdsa_test_params_t;

} // namespace api_test

#endif
