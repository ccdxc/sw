//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all vnic test cases
///
//----------------------------------------------------------------------------
#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/apollo/include/api/pds_batch.hpp"
#include "nic/apollo/include/api/pds_switchport.hpp"
#include "nic/apollo/include/api/pds_tep.hpp"
#include "nic/apollo/include/api/pds_vcn.hpp"
#include "nic/apollo/include/api/pds_subnet.hpp"
#include "nic/apollo/include/api/pds_vnic.hpp"
#include "nic/apollo/include/api/pds_mapping.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/mapping.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"

const char *g_cfg_file = "hal.json";

//----------------------------------------------------------------------------
// VNIC test class
//----------------------------------------------------------------------------

class vnic_test : public pds_test_base {
protected:
    vnic_test() {}
    virtual ~vnic_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// VNIC test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC
/// @{

/// \brief Create a VNIC
///
/// Create a VNIC and test the following
///
/// Configure both ipv4 and ipv6 addresses and run the packet test
///
/// Configure min to max of vlan ids and verify with packet
///
/// Configure varying range of mpls slots and verify with packet
///
/// Get the vnic stats and compare the valid, drops/reason matches with the
/// expectation.
TEST_F(vnic_test, vnic_create) {}

/// \brief Create many VNICs
///
/// Create max number of VNICs and test the following
///
/// Configure varying range of vlan tags & mpls slots and verify with packet
///
/// Get the vnic stats and compare the valid, drops/reason matches with the
/// expectation.
TEST_F(vnic_test, vnic_many_create) {}

/// \brief Get VNIC
///
/// Configure the vnic and make-sure the parameters are configured properly
/// by getting the configured values back
TEST_F(vnic_test, vnic_get) {}

/// \brief VNIC source and destination check
///
/// Configure skip_src_dst_check to true/false and make sure the config by
/// sending both valid and invalid IP addresses.
///
/// Get the stats and makesure the drop count/reason are as expected
TEST_F(vnic_test, vnic_src_dst_check) {}

/// @}

// print help message showing usage of HAL
static inline void
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c <hal.json> \n", argv[0]);
}

int
main (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, "hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            g_cfg_file = optarg;
            if (!g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        default:
            // ignore all other options
            break;
        }
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
