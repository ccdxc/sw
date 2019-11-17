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
#include "boost/property_tree/ptree.hpp"

#define EVPN_CONFIG_FILE_PATH  "/sw/nic/metaswitch/config/"
using boost::property_tree::ptree;
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
    ptree       pt;
    std::string value;
    std::string cfg_file;
    std::string cfg_path = EVPN_CONFIG_FILE_PATH;

    if (argc != 2)
    {
        cout << "Error! Invalid Command Line Arguments. Usage:\n";
        cout << "pdsa-binary <json config file name>\n";
        return 0;
    }

    // Get config file
    cfg_file = cfg_path + argv[1];
    std::ifstream json_cfg (cfg_file.c_str());
    if (!json_cfg)
    {
        cout << "File Error! Cannot open configuration file " <<cfg_file<<endl;
        return -1;
    }

    //read config
    read_json (json_cfg, pt);
    value           = pt.get <std::string>("local.ip","");
    g_node_a_ip     = inet_network (value.c_str());
    value           = pt.get <std::string>("local.ac-ip","");
    g_node_a_ac_ip  = inet_network (value.c_str());
    value           = pt.get <std::string>("remote.ip","");
    g_node_b_ip     = inet_network (value.c_str());
    value           = pt.get <std::string>("remote.ac-ip","");
    g_node_b_ac_ip  = inet_network (value.c_str());
    value           = pt.get <std::string>("if-index","");
    g_evpn_if_index = strtol (value.c_str(),NULL, 0);

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
