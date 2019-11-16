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
#include "nic/metaswitch/pdsa_stub_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_init.hpp"
#include "iostream"

using namespace std;

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

unsigned int g_node_a_ip;
unsigned int g_node_b_ip;
unsigned int g_node_a_ac_ip;
unsigned int g_node_b_ac_ip;
unsigned int g_evpn_if_index;

int
main (int argc, char **argv)
{
    if (argc != 6)
    {
        cout << "Error! Invalid Command Line Arguments. Usage:\n";
        cout << "pdsa_binary Local-IP Remote-IP Local-AC-IP Remote-AC-IP If-Index\n";
        cout << "IP Addresses in A.B.C.D Format. If-Index in Hex\n";
        return 0;
    }

    g_node_a_ip     = inet_network (argv[1]); // NODE A IP Address
    g_node_b_ip     = inet_network (argv[2]); // NODE B IP Address
    g_node_a_ac_ip  = inet_network (argv[3]); // NODE A AC IP
    g_node_b_ac_ip  = inet_network (argv[4]); // NODE B AC IP
    g_evpn_if_index = strtol (argv[5], NULL, 0); // If Index

    pds_init(NULL);
    if(!pdsa_stub_mgmt_init()) {
        return -1;
    }
    pdsa_stub::init();
#ifdef PDS_MOCKAPI
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    return api_test_program_run(argc, argv);
#endif
}
