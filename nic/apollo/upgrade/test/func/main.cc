//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/platform/capri/capri_quiesce.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/test/base/base.hpp"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"

using std::string;
char *g_cfg_file = NULL;

class upg_func_gtest : public pds_test_base {
protected:
    upg_func_gtest() {}
    virtual ~upg_func_gtest() {}
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

// This test validates the table property get and set functions
TEST_F(upg_func_gtest, table_property_get_set) {
    p4pd_pipeline_t pipe[] = { P4_PIPELINE_INGRESS, P4_PIPELINE_EGRESS,
        P4_PIPELINE_RXDMA, P4_PIPELINE_TXDMA };
    p4_tbl_eng_cfg_t cfg[P4_PIPELINE_MAX][P4TBL_ID_MAX];
    uint32_t cfg_cnt[P4_PIPELINE_MAX];
    sdk_ret_t ret;

    // property get and set functions
    // during upgrade gets functions are used to save the new configuration in
    // pre-upgrade stage.
    // during switch the saved values will be applied on hw in P4 quiesce state.
    // this would save the quiesce time..
    for (uint32_t i = 0; i < sizeof(pipe)/sizeof(uint32_t); i++) {
        cfg_cnt[i] = sdk::asic::pd::asicpd_tbl_eng_cfg_get(
            pipe[i], &cfg[i][0], P4TBL_ID_MAX);
        SDK_ASSERT(cfg_cnt[i]);
    }
    ret = sdk::asic::pd::asicpd_rss_tbl_eng_cfg_get(
        "apulu_rxdma",
        P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR,
        &cfg[P4_PIPELINE_RXDMA][P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // property set function
    sdk::platform::capri::capri_quiesce_start();
    for (uint32_t i = 0; i < sizeof(pipe)/sizeof(uint32_t); i++) {
        ret = sdk::asic::pd::asicpd_tbl_eng_cfg_modify(
            pipe[i], &cfg[i][0], cfg_cnt[i]);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    sdk::asic::pd::asicpd_rss_tbl_eng_cfg_modify(
        &cfg[P4_PIPELINE_RXDMA][P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR]);
    sdk::platform::capri::capri_quiesce_stop();

}

// This test validates the pds upgrade fucntions
TEST_F(upg_func_gtest, start_switch) {
    pds_upg_spec_t spec;
    sdk_ret_t ret;

    spec.stage = UPG_STAGE_START;
    ret = pds_upgrade(&spec);
    SDK_ASSERT(ret == SDK_RET_OK);

    spec.stage = UPG_STAGE_SWITCHOVER;
    sdk::platform::capri::capri_quiesce_start();
    ret = pds_upgrade(&spec);
    sdk::platform::capri::capri_quiesce_stop();
    SDK_ASSERT(ret == SDK_RET_OK);
}

// print help message showing usage of HAL
static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c <hal.json>\n", argv[0]);
}

int
main (int argc, char **argv)
{
    int oc, ret;
    struct option longopts[] = {
       { "config",    required_argument, NULL, 'c' },
       { "help",      no_argument,       NULL, 'h' },
       { 0,           0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1) {
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
    ret = RUN_ALL_TESTS();

    return ret;
}
