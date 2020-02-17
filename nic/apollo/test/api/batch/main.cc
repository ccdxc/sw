//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all the batch test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Batch test class
//----------------------------------------------------------------------------

class batch_test : public pds_test_base {
protected:
    batch_test() {}
    virtual ~batch_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Batch test cases implementation
//----------------------------------------------------------------------------

/// \defgroup BATCH_TEST Batch Tests
/// @{

/// \brief Batch WF_B1
/// \ref WF_B1
TEST_F(batch_test, batch_workflow_b1) {
    if (!apulu()) return;

    // empty batch tests
    for (uint32_t i = 0; i < 4096; ++i) {
        pds_batch_ctxt_t bctxt = batch_start();
        batch_commit(bctxt);
    }
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
