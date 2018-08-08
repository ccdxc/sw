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
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
