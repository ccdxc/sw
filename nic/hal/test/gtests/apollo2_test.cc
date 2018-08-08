#include <stdio.h>
#include <iostream>
#include <gtest/gtest.h>

#include <boost/crc.hpp>
#include "boost/property_tree/json_parser.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "sdk/pal.hpp"
#include "sdk/utils.hpp"
#include "sdk/types.hpp"
#include "nic/hal/pd/capri/capri_config.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/p4pd/p4pd_api.hpp"
#include "nic/gen/apollo2/include/p4pd.h"
#include "nic/p4/apollo2/include/defines.h"
//#include "nic/p4/apollo2/include/slacl_defines.h"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "nic/hal/pd/globalpd/gpd_utils.hpp"

hal_ret_t capri_default_config_init(capri_cfg_t *cfg);
using boost::property_tree::ptree;

uint8_t g_snd_pkt1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xA1,
    0xA2, 0xA3, 0xA4, 0xA5, 0x81, 0x00, 0x00, 0x64,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x62, 0x8D, 0x0B, 0x01,
    0x02, 0x03, 0x0A, 0x0A, 0x01, 0x01, 0x03, 0xFF,
    0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0x56, 0x1A,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79};

uint8_t g_rcv_pkt1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xA1,
    0xA2, 0xA3, 0xA4, 0xA5, 0x81, 0x00, 0x00, 0x64,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x62, 0x8D, 0x0B, 0x01,
    0x02, 0x03, 0x0A, 0x0A, 0x01, 0x01, 0x03, 0xFF,
    0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0x56, 0x1A,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79};

class apollo_test : public ::testing::Test {
  protected:
    apollo_test() {}
    virtual ~apollo_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(apollo_test, test1) {
    int ret = 0;
    uint64_t asm_base_addr;
    p4pd_cfg_t    p4pd_cfg = {
        .table_map_cfg_file = "apollo2/capri_p4_table_map.json",
        .p4pd_pgm_name = "apollo2",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    const char *hal_conf_file = "conf/hal.json";
    char *default_config_dir = NULL;
    capri_cfg_t cfg;
    sdk::lib::catalog    *catalog;

    printf("Connecting to ASIC SIM\n");
    hal::utils::trace_init("hal", 0, true, "hal.log", hal::utils::trace_debug);
    ret = sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM);
    ASSERT_NE(ret, -1);
    printf("Loading CAPRI config\n");
    ret = capri_load_config((char *)"obj/apollo2/pgm_bin");
    ASSERT_NE(ret, -1);
    cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    cfg.pgm_name = "apollo2";
    printf("Parsing HBM config\n");
    ret = capri_hbm_parse(&cfg);
    ASSERT_NE(ret, -1);
    if (getenv("HAL_PLATFORM_MODE_RTL")) {
        hal_conf_file = "conf/hal_apollo2_rtl.json";
    }

    asm_base_addr = (uint64_t)get_start_offset((char *)JP4_PRGM);
    ret = capri_load_mpu_programs("apollo2", (char *)"obj/apollo2/asm_bin",
                                  asm_base_addr, NULL, 0);
    ASSERT_NE(ret, -1);
    std::ifstream json_cfg(hal_conf_file);
    ptree pt;
    read_json(json_cfg, pt);
    capri_list_program_addr(pt.get<std::string>("asic.loader_info_file").c_str());
    ret = p4pd_init(&p4pd_cfg);
    ASSERT_NE(ret, -1);
    ret = capri_table_rw_init();
    ASSERT_NE(ret, -1);
    ret = capri_hbm_cache_init(NULL);
    ASSERT_NE(ret, -1);
    ret = capri_hbm_cache_regions_init();
    ASSERT_NE(ret, -1);
    ret = hal::pd::asicpd_table_mpu_base_init(&p4pd_cfg);
    ASSERT_NE(ret, -1);
    ret = hal::pd::asicpd_program_table_mpu_pc();
    ASSERT_NE(ret, -1);
    ret = hal::pd::asicpd_deparser_init();
    ASSERT_NE(ret, -1);
    ret = hal::pd::asicpd_program_hbm_table_base_addr();
    ASSERT_NE(ret, -1);

    catalog = sdk::lib::catalog::factory(cfg.cfg_path + "/catalog.json");
    ASSERT_TRUE(catalog != NULL);

    if (!catalog->qos_sw_init_enabled()) {
        default_config_dir = std::getenv("HAL_PBC_INIT_CONFIG");
        if (default_config_dir) {
            cfg.default_config_dir = std::string(default_config_dir);
        } else {
            cfg.default_config_dir = "8x25_hbm";
        }
        ret = capri_default_config_init(&cfg);
    }
    ASSERT_NE(ret, -1);
    ret = capri_tm_init(catalog);
    ASSERT_NE(ret, -1);

    config_done();

    uint32_t port = 0;
    uint32_t cos = 0;
    std::vector<uint8_t> ipkt;
    std::vector<uint8_t> opkt;
    std::vector<uint8_t> epkt;

    ipkt.resize(sizeof(g_snd_pkt1));
    memcpy(ipkt.data(), g_snd_pkt1, sizeof(g_snd_pkt1));
    epkt.resize(sizeof(g_rcv_pkt1));
    memcpy(epkt.data(), g_rcv_pkt1, sizeof(g_rcv_pkt1));
    testcase_begin(0, 0);
    step_network_pkt(ipkt, port);
    get_next_pkt(opkt, port, cos);
    EXPECT_TRUE(opkt == epkt);
    testcase_end(0, 0);

    exit_simulation();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
