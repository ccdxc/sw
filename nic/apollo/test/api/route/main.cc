//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all route test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/route.hpp"
#include "nic/apollo/test/api/utils/device.hpp"
#include "nic/apollo/test/api/utils/tep.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/subnet.hpp"
#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/nexthop_group.hpp"

namespace test {
namespace api {

// globals
static const std::string k_base_v4_pfx  = "100.100.100.1/16";

//----------------------------------------------------------------------------
// route test class
//----------------------------------------------------------------------------

class route_test : public pds_test_base {
protected:
    route_test() {}
    virtual ~route_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode()) {
            pds_test_base::SetUpTestCase(g_tc_params);
        }
        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        sample_device_setup(bctxt);
        sample_if_setup(bctxt);
        sample_nexthop_setup(bctxt);
        sample_nexthop_group_setup(bctxt);
        sample_tep_setup(bctxt);
        sample_subnet_setup(bctxt);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample_subnet_teardown(bctxt);
        sample_tep_teardown(bctxt);
        sample_nexthop_group_teardown(bctxt);
        sample_nexthop_teardown(bctxt);
        sample_if_teardown(bctxt);
        sample_device_teardown(bctxt);
        sample1_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode()) {
            pds_test_base::TearDownTestCase();
        }
    }
};

//----------------------------------------------------------------------------
// Route test cases implementation
//----------------------------------------------------------------------------

/// \defgroup ROUTE_TESTS Route tests
/// @{

TEST_F(route_test, route_add) {
    pds_batch_ctxt_t bctxt = batch_start();
    sample_route_table_setup(bctxt, k_base_v4_pfx, IP_AF_IPV4, 10, 1, 1);
    // TODO: add a route
    batch_commit(bctxt);
}

TEST_F(route_test, route_update) {
    pds_batch_ctxt_t bctxt = batch_start();
    sample_route_table_setup(bctxt, k_base_v4_pfx, IP_AF_IPV4, 10, 1, 2);
    // TODO: update a route
    batch_commit(bctxt);
}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
