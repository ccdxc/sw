//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all policy rule test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/policy.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static constexpr uint16_t g_num_stateful_rules = 64;

//----------------------------------------------------------------------------
// Policy test class
//----------------------------------------------------------------------------

class policy_rule_test : public ::pds_test_base {
protected:
    policy_rule_test() {}
    virtual ~policy_rule_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode()) {
            pds_test_base::SetUpTestCase(g_tc_params);
        }
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_VERBOSE;
        pds_batch_ctxt_t bctxt = batch_start();
        sample_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;
        pds_batch_ctxt_t bctxt = batch_start();
        sample_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode()) {
            pds_test_base::TearDownTestCase();
        }
    }
};

//----------------------------------------------------------------------------
// Policy rule test cases implementation
//----------------------------------------------------------------------------

/// \defgroup POLICY_TEST Policy rule tests
/// @{

TEST_F(policy_rule_test, rule_add) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    sample_policy_setup(bctxt);
    batch_commit(bctxt);

    bctxt = batch_start();
    // TODO: add policy rule(s)
    batch_commit(bctxt);

    bctxt = batch_start();
    sample_policy_teardown(bctxt);
    batch_commit(bctxt);
}

TEST_F(policy_rule_test, rule_upd) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    sample_policy_setup(bctxt);
    batch_commit(bctxt);

    bctxt = batch_start();
    // TODO: update policy rule(s)
    batch_commit(bctxt);

    bctxt = batch_start();
    sample_policy_teardown(bctxt);
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
