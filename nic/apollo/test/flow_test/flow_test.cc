// Copyright (c) 2018 Pensando Systems, Inc.

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/base/base.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/sdk/platform/capri/capri_p4.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/sdk/include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/crc_fast.hpp"
#include "nic/sdk/lib/utils/time_profile.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/apollo/test/scale/test_common.hpp"
#include "flow_test.hpp"

using std::string;
namespace pt = boost::property_tree;

char                *g_cfg_file = NULL;
bool                g_daemon_mode = false;
flow_test           *g_flow_test_obj;
test_params_t g_test_params = { 0 };

class flow_gtest : public pds_test_base {
protected:
    flow_gtest() {}
    virtual ~flow_gtest() {}
    /**< called immediately after the constructor before each test */
    virtual void SetUp() {}
    /**< called immediately after each test before the destructor */
    virtual void TearDown() {}
    /**< called at the beginning of all test cases in this class */
    static void SetUpTestCase() {
        /**< call base class function */
        test_case_params_t params;
        params.cfg_file = g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
    }
};

#define PERF_TEST_SCALE 10*1024

TEST_F(flow_gtest, flows_create) {
    parse_test_cfg(&g_test_params, std::string("apollo"));
    g_flow_test_obj = new flow_test(true);
    g_flow_test_obj->generate_dummy_epdb();
    g_flow_test_obj->set_cfg_params(&g_test_params,
                                    true, 0, PERF_TEST_SCALE, 0,
                                    100, 100, 1024, 2047);
    g_flow_test_obj->create_flows();
}

TEST_F(flow_gtest, flows_clear) {
    ASSERT_TRUE(g_flow_test_obj != NULL);
    g_flow_test_obj->clear_flows();
}

TEST_F(flow_gtest, DISABLED_p4pd_entry_read) {
    p4pd_error_t p4pdret = 0;
    uint8_t sw_key[128];
    uint8_t sw_data[128];
    for (uint32_t i = 0; i < PERF_TEST_SCALE; i++) {
        p4pdret = p4pd_entry_read(5, 0, sw_key, NULL, sw_data);
        ASSERT_TRUE(p4pdret == P4PD_SUCCESS);
    }
}

TEST_F(flow_gtest, DISABLED_p4pd_entry_install) {
    p4pd_error_t p4pdret = 0;
    flow_swkey_t sw_key;
    flow_actiondata_t sw_data;
    uint32_t index = 0;
    memset(&sw_key, 0xff, sizeof(sw_key));
    memset(&sw_data, 0xff, sizeof(sw_data));

    for (uint32_t i = 0; i < PERF_TEST_SCALE; i++) {
        index = random() % (16*1048576);
        p4pdret = p4pd_entry_install(5, index, (uint8_t *)&sw_key, NULL, (uint8_t *)&sw_data);
        ASSERT_TRUE(p4pdret == P4PD_SUCCESS);
    }
}

TEST_F(flow_gtest, DISABLED_p4pd_entry_write) {
    p4pd_error_t p4pdret = 0;
    uint8_t sw_key[128];
    uint8_t sw_data[128];
    uint32_t index = 0;
    memset(&sw_key, 0xff, sizeof(sw_key));
    memset(&sw_data, 0xff, sizeof(sw_data));

    for (uint32_t i = 0; i < PERF_TEST_SCALE; i++) {
        index = random() % (16*1048576);
        p4pdret = p4pd_entry_write(5, i, sw_key, NULL, sw_data);
        ASSERT_TRUE(p4pdret == P4PD_SUCCESS);
    }
}

TEST_F(flow_gtest, DISABLED_p4pd_hwkey_hwmask_build) {
    p4pd_error_t p4pdret = 0;
    uint8_t swkey[128];
    uint8_t hwkey[128];

    for (uint32_t i = 0; i < PERF_TEST_SCALE; i++) {
        p4pdret = p4pd_hwkey_hwmask_build(5, swkey, NULL, hwkey, NULL);
        ASSERT_TRUE(p4pdret == P4PD_SUCCESS);
    }
}

TEST_F(flow_gtest, DISABLED_crc_perf) {
    uint8_t hwkey[64];
    for (uint32_t i = 0; i < PERF_TEST_SCALE; i++) {
        sdk::utils::crc32(hwkey, 64, 0);
    }
}

TEST_F(flow_gtest, DISABLED_asicpd_hbm_table_entry_read) {
    uint8_t hwkey[128];
    uint16_t len;
    for (uint32_t i = 0; i < PERF_TEST_SCALE; i++) {
        sdk::asic::pd::asicpd_hbm_table_entry_read(5, 0, hwkey, &len);
    }
}

TEST_F(flow_gtest, DISABLED_asicpd_hbm_table_entry_write) {
    uint8_t hwkey[128];
    for (uint32_t i = 0; i < PERF_TEST_SCALE; i++) {
        sdk::asic::pd::asicpd_hbm_table_entry_write(5, 0, hwkey, 64);
    }
}

TEST_F(flow_gtest, DISABLED_p4pd_actiondata_hwfield_set) {
    uint8_t swdata[128];
    uint8_t entry_valid;
    for (uint32_t i = 0; i < 2*PERF_TEST_SCALE; i++) {
        p4pd_actiondata_hwfield_set(5, 0, 0, &entry_valid, swdata);
    }
}

TEST_F(flow_gtest, DISABLED_p4pd_actiondata_hwfield_get) {
    uint8_t swdata[128];
    uint8_t entry_valid;
    for (uint32_t i = 0; i < 2*PERF_TEST_SCALE; i++) {
        p4pd_actiondata_hwfield_get(5, 0, 0, &entry_valid, swdata);
    }
}

TEST_F(flow_gtest, DISABLED_p4pd_actiondata_appdata_set) {
    uint8_t swdata[128];
    uint8_t appdata[32];
    for (uint32_t i = 0; i < 2*PERF_TEST_SCALE; i++) {
        p4pd_actiondata_appdata_set(5, 0, appdata, swdata);
    }
}

TEST_F(flow_gtest, DISABLED_p4pd_actiondata_appdata_get) {
    uint8_t swdata[128];
    uint8_t appdata[32];
    for (uint32_t i = 0; i < 2*PERF_TEST_SCALE; i++) {
        p4pd_actiondata_appdata_get(5, 0, appdata, swdata);
    }
}

static void
flow_test_time_profile_print() {
    printf("%-42s = %012ld\n", "TOTAL_FLOW_CREATE",
           time_profile_total(sdk::utils::time_profile::TABLE_LIB_FTL_INSERT));
    printf("%-42s = %012ld\n", "FTL_INSERT(ftl only)",
           time_profile_total(sdk::utils::time_profile::TABLE_LIB_FTL_INSERT) -
           time_profile_total(sdk::utils::time_profile::P4PD_ENTRY_READ) -
           time_profile_total(sdk::utils::time_profile::P4PD_ENTRY_INSTALL) -
           time_profile_total(sdk::utils::time_profile::COMPUTE_CRC));
    //printf("%-42s = %012ld\n", "COMPUTE_CRC",
    //       time_profile_total(sdk::utils::time_profile::COMPUTE_CRC));
    printf("%-42s = %012ld\n", "P4PD_HWKEY_HWMASK_BUILD",
           time_profile_total(sdk::utils::time_profile::P4PD_HWKEY_HWMASK_BUILD));
    printf("%-42s = %012ld\n", "P4PD_ENTRY_READ",
           time_profile_total(sdk::utils::time_profile::P4PD_ENTRY_READ) -
           time_profile_total(sdk::utils::time_profile::ASICPD_HBM_TABLE_ENTRY_READ));
    printf("%-42s = %012ld\n", "- ASICPD_HBM_TABLE_ENTRY_READ",
           time_profile_total(sdk::utils::time_profile::ASICPD_HBM_TABLE_ENTRY_READ) -
           time_profile_total(sdk::utils::time_profile::ASIC_MEM_READ));
    printf("%-42s = %012ld\n", " - ASIC_MEM_READ",
           time_profile_total(sdk::utils::time_profile::ASIC_MEM_READ) -
           time_profile_total(sdk::utils::time_profile::PAL_MEM_RD));
    printf("%-42s = %012ld\n", "   - PAL_MEM_RD",
           time_profile_total(sdk::utils::time_profile::PAL_MEM_RD));
    printf("%-42s = %012ld\n", "P4PD_ENTRY_INSTALL",
           time_profile_total(sdk::utils::time_profile::P4PD_ENTRY_INSTALL) -
           time_profile_total(sdk::utils::time_profile::ASICPD_HBM_TABLE_ENTRY_WRITE));
    printf("%-42s = %012ld\n", "- ASICPD_HBM_TABLE_ENTRY_WRITE",
           time_profile_total(sdk::utils::time_profile::ASICPD_HBM_TABLE_ENTRY_WRITE) -
           time_profile_total(sdk::utils::time_profile::CAPRI_HBM_TABLE_ENTRY_WRITE) -
           time_profile_total(sdk::utils::time_profile::CAPRI_HBM_TABLE_ENTRY_CACHE_INVALIDATE));
    printf("%-42s = %012ld\n", " - CAPRI_HBM_TABLE_ENTRY_WRITE",
           time_profile_total(sdk::utils::time_profile::CAPRI_HBM_TABLE_ENTRY_WRITE) -
           time_profile_total(sdk::utils::time_profile::ASIC_MEM_WRITE));
    printf("%-42s = %012ld\n", "   - ASIC_MEM_WRITE",
           time_profile_total(sdk::utils::time_profile::ASIC_MEM_WRITE) -
           time_profile_total(sdk::utils::time_profile::PAL_MEM_WR));
    printf("%-42s = %012ld\n", "     - PAL_MEM_WR",
           time_profile_total(sdk::utils::time_profile::PAL_MEM_WR));
    printf("%-42s = %012ld\n", " - CAPRI_HBM_TABLE_ENTRY_CACHE_INVALIDATE",
           time_profile_total(sdk::utils::time_profile::CAPRI_HBM_TABLE_ENTRY_CACHE_INVALIDATE));
    printf("%-42s = %012ld\n", "PAL_REG_WRITE",
           time_profile_total(sdk::utils::time_profile::PAL_REG_WRITE));
    //printf("%-42s = %012ld\n", "PAL_REG_READ",
    //       time_profile_total(sdk::utils::time_profile::PAL_REG_READ));
}

// print help message showing usage of HAL
static void inline
print_usage (char **argv)
{
    fprintf(stdout,
            "Usage : %s -c <hal.json> -i <object-config.json> -f <apollo|artemis>\n", argv[0]);
}

int
main (int argc, char **argv)
{
    int oc, ret;
    struct option longopts[] = {
       { "config",    required_argument, NULL, 'c' },
       { "daemon",    required_argument, NULL, 'd' },
       { "help",      no_argument,       NULL, 'h' },
       { 0,           0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hdc:f:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            g_cfg_file = optarg;
            if (!g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'd':
            g_daemon_mode = true;
            break;

        default:
            // ignore all other options
            break;
        }
    }

    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();

    flow_test_time_profile_print();
    return ret;
}
