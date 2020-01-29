//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/test/base/base.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"

using std::string;
char *g_cfg_file = NULL;

#define MAX_TABLES_PER_PIPELINE 32
#define MAX_PIPELINES           4

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
    p4_tbl_eng_cfg_t cfg[MAX_PIPELINES][MAX_TABLES_PER_PIPELINE];
    uint32_t cfg_cnt[MAX_PIPELINES];
    sdk_ret_t ret;

    // property get and set functions
    // during upgrade gets functions are used to save the new configuration in
    // pre-upgrade stage.
    // during switch the saved values will be applied on hw in P4 quiesce state.
    // this would save the quiesce time..
    for (uint32_t i = 0; i < sizeof(pipe)/sizeof(uint32_t); i++) {
        cfg_cnt[i] = sdk::asic::pd::asicpd_tbl_eng_cfg_get(
            pipe[i], &cfg[i][0], MAX_TABLES_PER_PIPELINE);
        SDK_ASSERT(cfg_cnt[i]);
    }
    ret = sdk::asic::pd::asicpd_rss_tbl_eng_cfg_get(
        "apulu_rxdma",
        P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR,
        &cfg[P4_PIPELINE_RXDMA][P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // property set function
    for (uint32_t i = 0; i < sizeof(pipe)/sizeof(uint32_t); i++) {
        ret = sdk::asic::pd::asicpd_tbl_eng_cfg_modify(
            pipe[i], &cfg[i][0], cfg_cnt[i]);
        SDK_ASSERT(ret);
    }
    sdk::asic::pd::asicpd_rss_tbl_eng_cfg_modify(
        &cfg[P4_PIPELINE_RXDMA][P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR]);

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
