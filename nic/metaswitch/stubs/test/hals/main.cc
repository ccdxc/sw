//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all nexthop test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/metaswitch/stubs/test/utils/base.hpp"

api_test::pdsa_test_params_t g_test_params;
namespace api_test {

extern pds_state g_pds_state;

class hals_test : public ::pds_test_base {
protected:
    hals_test() {}
    virtual ~hals_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

TEST_F(hals_test, vxlan_create_update) {
    //TODO: Vxlan create/update cases

}


} // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
#ifdef PDS_MOCKAPI
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    return api_test_program_run(argc, argv);
#endif
}
