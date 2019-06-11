//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all device test cases
///
//----------------------------------------------------------------------------

#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"
#include "nic/apollo/test/utils/workflow.hpp"
#include "nic/apollo/test/utils/utils.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static const char *g_cfg_file = NULL;
static std::string g_pipeline("");

//----------------------------------------------------------------------------
// Device test class
//----------------------------------------------------------------------------

class device_test : public pds_test_base {
protected:
    device_test() {}
    virtual ~device_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;

        params.cfg_file = api_test::g_cfg_file;
        params.pipeline = api_test::g_pipeline;
        params.enable_fte = FALSE;
        pds_test_base::SetUpTestCase(params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Device test cases implementation
//----------------------------------------------------------------------------

/// \defgroup DEVICE_TEST
/// @{

/// \brief Device WF_1
TEST_F(device_test, device_workflow_1) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_1<device_feeder>(feeder);
}

/// \brief Device WF_2
TEST_F(device_test, device_workflow_2) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_2<device_feeder>(feeder);
}

/// \brief Device WF_4
TEST_F(device_test, device_workflow_4) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_4<device_feeder>(feeder);
}

/// \brief Device WF_6
TEST_F(device_test, device_workflow_6) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_6<device_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_7
TEST_F(device_test, device_workflow_7) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_7<device_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_8
TEST_F(device_test, DISABLED_device_workflow_8) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_8<device_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_9
TEST_F(device_test, DISABLED_device_workflow_9) {
    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    workflow_9<device_feeder>(feeder1, feeder1A);
}

/// \brief Device WF_N_1
TEST_F(device_test, device_workflow_neg_1) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_neg_1<device_feeder>(feeder);
}

/// \brief Device WF_N_3
TEST_F(device_test, device_workflow_neg_3) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_neg_3<device_feeder>(feeder);
}

/// \brief Device WF_N_5
TEST_F(device_test, DISABLED_device_workflow_neg_5) {
    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    workflow_neg_5<device_feeder>(feeder1, feeder1A);
}

/// @}

}    // namespace api_test

static inline void
device_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json> -f <apollo|artemis>" << endl;
    return;
}

static void
device_test_options_parse (int argc, char **argv)
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

static inline sdk_ret_t
device_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return SDK_RET_ERR;
    }
    api_test::g_pipeline = api_test::pipeline_get();
    if (!IS_APOLLO() && !IS_ARTEMIS()) {
        cerr << "Pipeline specified is invalid" << endl;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------
int
main (int argc, char **argv)
{
    device_test_options_parse(argc, argv);
    if (device_test_options_validate() != SDK_RET_OK) {
        device_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
