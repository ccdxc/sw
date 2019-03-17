//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all route table test cases
///
//----------------------------------------------------------------------------
#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/route.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;

//----------------------------------------------------------------------------
// Route table test class
//----------------------------------------------------------------------------

class route_test : public pds_test_base {
protected:
    route_test() {}
    virtual ~route_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(api_test::g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// Route table test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Route table
/// @{

/// \brief Create a V4 route table
/// Create a v4 route table with ten routes
TEST_F(route_test, v4_route_table_create) {
    pds_batch_params_t batch_params = {0};
    pds_route_table_info_t info;
    pds_route_table_id_t rt_table_id = 1;
    uint32_t num_routes = 10;
    const char *first_nh_ip_str = "30.30.30.1";
    const char *first_route_pfx_str = "100.100.0.0/16";
    route_table_util rt_obj(rt_table_id, first_route_pfx_str,
                            first_nh_ip_str, IP_AF_IPV4, num_routes);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create max teps
    ASSERT_TRUE(tep_util::many_create(PDS_MAX_TEP-1, first_nh_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rt_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

#if 0
    // read API is NOT available yet
    ASSERT_TRUE(rt_obj->read(&info) == sdk::SDK_RET_OK);
#endif

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

#if 0
    ASSERT_TRUE(rt_obj->read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
#endif
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
route_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
route_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
route_test_options_parse (int argc, char **argv)
{
    int oc = -1;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            break;
        default:    // ignore all other options
            break;
        }
    }
    return;
}

int
main (int argc, char **argv)
{
    route_test_options_parse(argc, argv);
    if (route_test_options_validate() != sdk::SDK_RET_OK) {
        route_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
