//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all vcn test cases
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/vcn.hpp"
//#include "nic/apollo/include/api/oci_vcn.hpp"

using std::cout;
using std::cerr;
using std::endl;

namespace api_test {

// GLobals
char *g_cfg_file = NULL;

//----------------------------------------------------------------------------
// VCN test class
//----------------------------------------------------------------------------

class vcn : public ::oci_test_base {
protected:
    vcn() {}
    virtual ~vcn() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        oci_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// VCN test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VCN_TEST
/// @{

/// \brief Create a VCN
///
/// Detailed description
TEST_F(vcn, vcn_create) { vcn_util::create(1, "10/8"); }

/// \brief Delete a VCN
///
/// Detailed description
TEST_F(vcn, vcn_delete) {}

/// @}

} // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
vcn_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
}

static void
vcn_test_options_parse (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
        default: // ignore all other options
            break;
        }
    }
}

static inline sdk_ret_t
vcn_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

int
main (int argc, char **argv)
{
    vcn_test_options_parse(argc, argv);
    if (vcn_test_options_validate() != SDK_RET_OK) {
        vcn_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
