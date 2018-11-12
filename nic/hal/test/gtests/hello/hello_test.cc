#include <stdio.h>
#include <map>
#include <iostream>
#include <gtest/gtest.h>

#include <boost/crc.hpp>
#include "boost/property_tree/json_parser.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/sdk/include/sdk/utils.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/platform/utils/lif_manager_base.hpp"
#include "nic/sdk/include/sdk/platform/capri/capri_qstate.hpp"
#include "nic/hal/pd/capri/capri_config.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/hal/pd/p4pd/p4pd_api.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "nic/hal/pd/globalpd/gpd_utils.hpp"
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "nic/p4/common/capri.h"
#include "nic/p4/common/defines.h"
#include "gen/p4gen/hello/include/p4pd.h"

hal_ret_t capri_default_config_init(capri_cfg_t *cfg);
using boost::property_tree::ptree;
using namespace sdk::platform::utils;
using namespace sdk::platform::capri;

uint8_t g_snd_pkt1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xC1,
    0xC2, 0xC3, 0xC4, 0xC5, 0x81, 0x00, 0x00, 0x64,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x85, 0x0B, 0x0B,
    0x01, 0x01, 0x0A, 0x0A, 0x01, 0x01, 0x12, 0x34,
    0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB4,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_rcv_pkt1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xC1,
    0xC2, 0xC3, 0xC4, 0xC5, 0x81, 0x00, 0x00, 0xDD,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x85, 0x0B, 0x0B,
    0x01, 0x01, 0x0A, 0x0A, 0x01, 0x01, 0x12, 0x34,
    0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB4,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_snd_pkt2[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xC1,
    0xC2, 0xC3, 0xC4, 0xC5,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x85, 0x0B, 0x0B,
    0x01, 0x01, 0x0A, 0x0A, 0x01, 0x01, 0x12, 0x34,
    0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB4,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_snd_pkt3[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xC1,
    0xC2, 0xC3, 0xC4, 0xC5, 0x81, 0x00, 0x00, 0x64,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x85, 0x0B, 0x0B,
    0x01, 0x01, 0x0A, 0x0A, 0x01, 0x01, 0x12, 0x34,
    0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB4,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_rcv_pkt3_0[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xC1,
    0xC2, 0xC3, 0xC4, 0xC5, 0x81, 0x00, 0x00, 0xDD,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x85, 0x0B, 0x0B,
    0x01, 0x01, 0x0A, 0x0A, 0x01, 0x01, 0x12, 0x34,
    0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB4,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_rcv_pkt3_1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xC1,
    0xC2, 0xC3, 0xC4, 0xC5, 0x81, 0x00, 0x00, 0xDD,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x85, 0x1B, 0x1B,
    0x11, 0x11, 0x0A, 0x0A, 0x01, 0x01, 0x43, 0x21,
    0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB4,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_rcv_pkt3_2[] = {
    0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00, 0xAA,
    0xBB, 0xCC, 0xDD, 0xEE, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0x00, 0x00, 0x64, 0x65, 0x66, 0x67, 0x0C, 0x0C,
    0x01, 0x01, 0x86, 0x2E, 0x19, 0xEB, 0x00, 0x68,
    0x00, 0x00, 0x00, 0x0C, 0x81, 0x00, 0x45, 0x00,
    0x00, 0x5C, 0x00, 0x01, 0x00, 0x00, 0x40, 0x06,
    0x63, 0x85, 0x07, 0x07, 0x07, 0x07, 0x0A, 0x0A,
    0x01, 0x01, 0x43, 0x21, 0x56, 0x78, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x02,
    0x20, 0x00, 0xF2, 0xB4, 0x00, 0x00, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A,
    0x78, 0x79,
};

static uint32_t
generate_hash_(void *key, uint32_t key_len, uint32_t crc_init_val) {
    boost::crc_basic<32> *crc_hash;
    uint32_t hash_val = 0x0;

    crc_hash = new boost::crc_basic<32>(0x04C11DB7, crc_init_val,
                                        0x00000000, false, false);
    crc_hash->process_bytes(key, key_len);
    hash_val = crc_hash->checksum();
    delete crc_hash;
    return hash_val;
}

static void
entry_write(uint32_t tbl_id, uint32_t index, void *key, void *mask,
            void *data, bool hash_table, uint32_t table_size) {
    if (key || mask) {
        // prepare entry and write hardware
        uint32_t hwkey_len = 0;
        uint32_t hwmask_len = 0;
        uint32_t hwdata_len = 0;
        uint8_t  *hwkey = NULL;
        uint8_t  *hwmask = NULL;
        p4pd_hwentry_query(tbl_id, &hwkey_len, &hwmask_len, &hwdata_len);
        hwkey_len = (hwkey_len >> 3) + ((hwkey_len & 0x7) ? 1 : 0);
        hwmask_len = (hwmask_len >> 3) + ((hwmask_len & 0x7) ? 1 : 0);
        hwdata_len = (hwdata_len >> 3) + ((hwdata_len & 0x7) ? 1 : 0);
        hwkey = new uint8_t[hwkey_len];
        hwmask = new uint8_t[hwmask_len];
        memset(hwkey, 0, hwkey_len);
        memset(hwmask, 0, hwmask_len);
        p4pd_hwkey_hwmask_build(tbl_id, key, mask, hwkey, hwmask);
        if (hash_table) {
            index = generate_hash_(hwkey, hwkey_len, 0);
            index &= table_size - 1;
        }
        p4pd_entry_write(tbl_id, index, hwkey, hwmask, data);
        delete [] hwkey;
        delete [] hwmask;
    } else {
        p4pd_entry_write(tbl_id, index, NULL, NULL, data);
    }
}

uint64_t g_layer1_smac = 0x00C1C2C3C4C5ULL;
uint64_t g_layer1_dmac = 0x000102030405ULL;
uint32_t g_layer1_sip = 0x0B0B0101;
uint32_t g_layer1_dip = 0x0A0A0101;
uint8_t  g_layer1_proto = 0x6;
uint16_t g_layer1_sport = 0x1234;
uint16_t g_layer1_dport = 0x5678;

uint16_t g_snat_index = 0xBAD;
uint16_t g_tunnel_index = 0xDAB;
uint32_t g_layer1_snat_sip = 0x07070707;
uint16_t g_layer1_snat_sport = 0x4321;

uint64_t g_layer0_smac = 0x00AABBCCDDEEULL;
uint64_t g_layer0_dmac = 0x001234567890ULL;
uint32_t g_layer0_sip = 0x64656667;
uint32_t g_layer0_dip = 0x0C0C0101;
uint32_t g_layer0_slot_id = 200;

static void
add_dst_mac_entries() {
    dst_mac_lkp_actiondata data;
    dst_mac_lkp_swkey_t key;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    memcpy(key.ethernet_1_dstAddr, &g_layer1_dmac, 6);
    data.actionid = DST_MAC_LKP_UPDATE_PORT_AND_MODIFY_VLAN_ID;
    data.dst_mac_lkp_action_u.dst_mac_lkp_update_port_and_modify_vlan.vid
        = 0xDD;
    entry_write(P4TBL_ID_DST_MAC_LKP, 0, &key, NULL, &data, true, 1024);
}

static void
add_flow_entries() {
    flow_lkp_swkey_t key;
    flow_lkp_actiondata data;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_srcAddr = g_layer1_sip;
    key.ipv4_1_dstAddr = g_layer1_dip;
    key.ipv4_1_protocol = g_layer1_proto;
    key.tcp_srcPort = g_layer1_sport;
    key.tcp_dstPort = g_layer1_dport;
    data.actionid = FLOW_LKP_FLOW_INFO_ID;
    data.flow_lkp_action_u.flow_lkp_flow_info.entry_valid = 1;
    data.flow_lkp_action_u.flow_lkp_flow_info.snat_valid = 1;
    data.flow_lkp_action_u.flow_lkp_flow_info.tunnel_valid = 1;
    data.flow_lkp_action_u.flow_lkp_flow_info.snat_index = g_snat_index;
    data.flow_lkp_action_u.flow_lkp_flow_info.tunnel_index = g_tunnel_index;
    entry_write(P4TBL_ID_FLOW_LKP, 0, &key, NULL, &data, true, 65536);
}

static void
add_snat_entries() {
    snat_actiondata data;

    memset(&data, 0, sizeof(data));
    data.actionid = SNAT_SNAT_REWRITE_ID;
    data.snat_action_u.snat_snat_rewrite.sip = g_layer1_snat_sip;
    data.snat_action_u.snat_snat_rewrite.sport = g_layer1_snat_sport;
    entry_write(P4TBL_ID_SNAT, g_snat_index, NULL, NULL, &data, false, 0);
}

static void
add_tunnel_entries() {
    tunnel_actiondata data;

    memset(&data, 0, sizeof(data));
    data.actionid = TUNNEL_TUNNEL_REWRITE_ID;
    memcpy(data.tunnel_action_u.tunnel_tunnel_rewrite.smac, &g_layer0_smac, 6);
    memcpy(data.tunnel_action_u.tunnel_tunnel_rewrite.dmac, &g_layer0_dmac, 6);
    data.tunnel_action_u.tunnel_tunnel_rewrite.sip = g_layer0_sip;
    data.tunnel_action_u.tunnel_tunnel_rewrite.dip = g_layer0_dip;
    data.tunnel_action_u.tunnel_tunnel_rewrite.slot_id = g_layer0_slot_id;
    entry_write(P4TBL_ID_TUNNEL, g_tunnel_index, NULL, NULL, &data, false, 0);
}

class hello_test : public ::testing::Test {
  protected:
    hello_test() {}
    virtual ~hello_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(hello_test, test1) {
    int ret = 0;
    uint64_t asm_base_addr;
    char *default_config_dir = NULL;
    capri_cfg_t cfg;
    sdk::lib::catalog *catalog;

    p4pd_cfg_t p4pd_cfg = {
        .table_map_cfg_file  = "hello/capri_p4_table_map.json",
        .p4pd_pgm_name       = "hello_p4",
        .p4pd_rxdma_pgm_name = "",
        .p4pd_txdma_pgm_name = "",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };

    cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    const char *hal_conf_file = "conf/hello/hal.json";
    hal::hal_platform_t platform = hal::HAL_PLATFORM_SIM;
    catalog = sdk::lib::catalog::factory(cfg.cfg_path + "/catalog.json");

    if (getenv("HAL_PLATFORM_RTL")) {
        hal_conf_file = "conf/hello/hal_rtl.json";
        platform = hal::HAL_PLATFORM_RTL;
    } else if (getenv("HAL_PLATFORM_HW")) {
        hal_conf_file = "conf/hello/hal_hw.json";
        platform= hal::HAL_PLATFORM_HW;
        catalog = sdk::lib::catalog::factory(cfg.cfg_path + "/catalog_hw.json");
    }
    ASSERT_TRUE(catalog != NULL);

    printf("Connecting to ASIC SIM\n");
    hal::hal_sdk_init();
    hal::utils::trace_init("hal", 0, true, "hal.log",
                           TRACE_FILE_SIZE_DEFAULT, TRACE_NUM_FILES_DEFAULT,
                           ::utils::trace_debug);
    ret = sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM);
    ASSERT_EQ(ret, HAL_RET_OK);
    printf("Loading Capri config\n");
    ret = capri_load_config((char *)"conf/hello/p4_bin");
    ASSERT_EQ(ret, HAL_RET_OK);

    cfg.pgm_name = "hello";
    cfg.llc_cache = true;
    cfg.p4_cache = true;
    cfg.p4plus_cache = true;
    if (getenv("LLC_CACHE_DISABLED")) {
        cfg.llc_cache = false;
    }
    if (getenv("P4_CACHE_DISABLED")) {
        cfg.p4_cache = false;
    }
    if (getenv("P4PLUS_CACHE_DISABLE")) {
        cfg.p4plus_cache = false;
    }

    printf("Parsing HBM config\n");
    ret = capri_hbm_parse(&cfg);
    ASSERT_EQ(ret, HAL_RET_OK);

    ret = p4pd_init(&p4pd_cfg);
    ASSERT_EQ(ret, HAL_RET_OK);

    printf("Loading Programs\n");
    asm_base_addr = (uint64_t)get_start_offset((char *)JP4_PRGM);
    ret = capri_load_mpu_programs("hello_p4",
                                  (char *)"conf/hello/p4_asm",
                                  asm_base_addr, NULL, 0, NULL);
    ASSERT_EQ(ret, HAL_RET_OK);

    std::ifstream json_cfg(hal_conf_file);
    ptree hal_conf;
    read_json(json_cfg, hal_conf);
    capri_list_program_addr(hal_conf.get<std::string>("asic.loader_info_file").c_str());

    hal::hal_cfg_t hal_cfg = { 0 };
    hal_cfg.platform = platform;
    ret = capri_table_rw_init(&hal_cfg);
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = capri_block_init(&cfg);
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = capri_hbm_cache_init(&cfg);
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = capri_hbm_cache_regions_init();
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = capri_tm_asic_init();
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = hal::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg);
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = hal::pd::asicpd_table_mpu_base_init(&p4pd_cfg);
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = hal::pd::asicpd_program_table_mpu_pc();
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = hal::pd::asicpd_deparser_init();
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = hal::pd::asicpd_program_hbm_table_base_addr();
    ASSERT_EQ(ret, HAL_RET_OK);

    if (!catalog->qos_sw_init_enabled()) {
        default_config_dir = std::getenv("HAL_PBC_INIT_CONFIG");
        if (default_config_dir) {
            cfg.default_config_dir = std::string(default_config_dir);
        } else {
            cfg.default_config_dir = "8x25_hbm";
        }
        ret = capri_default_config_init(&cfg);
    }
    ASSERT_EQ(ret, HAL_RET_OK);

#ifdef LATER
    ret = capri_txs_scheduler_init(1, &hal_cfg);
    ASSERT_EQ(ret, HAL_RET_OK);
#endif

    ret = capri_tm_init(catalog);
    ASSERT_EQ(ret, HAL_RET_OK);

    config_done();

    add_dst_mac_entries();
    add_flow_entries();
    add_snat_entries();
    add_tunnel_entries();

    uint32_t port = 0;
    uint32_t cos = 0;
    std::vector<uint8_t> ipkt;
    std::vector<uint8_t> opkt;
    std::vector<uint8_t> epkt;

#if 0
    testcase_begin(0, 1);
    ipkt.resize(sizeof(g_snd_pkt1));
    memcpy(ipkt.data(), g_snd_pkt1, sizeof(g_snd_pkt1));
    epkt.resize(sizeof(g_rcv_pkt1));
    memcpy(epkt.data(), g_rcv_pkt1, sizeof(g_rcv_pkt1));
    step_network_pkt(ipkt, TM_PORT_UPLINK_0);
    get_next_pkt(opkt, port, cos);
    EXPECT_TRUE(opkt == epkt);
    EXPECT_TRUE(port == TM_PORT_UPLINK_1);
    testcase_end(0, 1);

    testcase_begin(0, 2);
    ipkt.resize(sizeof(g_snd_pkt2));
    memcpy(ipkt.data(), g_snd_pkt2, sizeof(g_snd_pkt2));
    epkt.resize(sizeof(g_snd_pkt2));
    memcpy(epkt.data(), g_snd_pkt2, sizeof(g_snd_pkt2));
    step_network_pkt(ipkt, TM_PORT_UPLINK_0);
    get_next_pkt(opkt, port, cos);
    EXPECT_TRUE(opkt == epkt);
    EXPECT_TRUE(port == TM_PORT_UPLINK_1);
    testcase_end(0, 2);

    testcase_begin(0, 2);
    ipkt.resize(sizeof(g_snd_pkt2));
    memcpy(ipkt.data(), g_snd_pkt2, sizeof(g_snd_pkt2));
    epkt.resize(sizeof(g_rcv_pkt1));
    memcpy(epkt.data(), g_rcv_pkt1, sizeof(g_rcv_pkt1));
    step_network_pkt(ipkt, TM_PORT_UPLINK_0);
    get_next_pkt(opkt, port, cos);
    EXPECT_TRUE(opkt == epkt);
    EXPECT_TRUE(port == TM_PORT_UPLINK_1);
    testcase_end(0, 2);

    testcase_begin(0, 3);
    ipkt.resize(sizeof(g_snd_pkt1));
    memcpy(ipkt.data(), g_snd_pkt1, sizeof(g_snd_pkt1));
    epkt.resize(sizeof(g_rcv_pkt1));
    memcpy(epkt.data(), g_rcv_pkt1, sizeof(g_rcv_pkt1));
    for (int i = 0; i < 100; i++) {
        step_network_pkt(ipkt, TM_PORT_UPLINK_0);
        get_next_pkt(opkt, port, cos);
        EXPECT_TRUE(opkt == epkt);
        EXPECT_TRUE(port == TM_PORT_UPLINK_1);
    }
    testcase_end(0, 3);
#endif

#if 0
    testcase_begin(0, 4);
    ipkt.resize(sizeof(g_snd_pkt3));
    memcpy(ipkt.data(), g_snd_pkt3, sizeof(g_snd_pkt3));
    epkt.resize(sizeof(g_rcv_pkt3_0));
    memcpy(epkt.data(), g_rcv_pkt3_0, sizeof(g_rcv_pkt3_0));
    step_network_pkt(ipkt, TM_PORT_UPLINK_0);
    get_next_pkt(opkt, port, cos);
    EXPECT_TRUE(opkt == epkt);
    EXPECT_TRUE(port == TM_PORT_UPLINK_1);
    testcase_end(0, 4);

    testcase_begin(0, 5);
    ipkt.resize(sizeof(g_snd_pkt3));
    memcpy(ipkt.data(), g_snd_pkt3, sizeof(g_snd_pkt3));
    epkt.resize(sizeof(g_rcv_pkt3_1));
    memcpy(epkt.data(), g_rcv_pkt3_1, sizeof(g_rcv_pkt3_1));
    step_network_pkt(ipkt, TM_PORT_UPLINK_0);
    get_next_pkt(opkt, port, cos);
    EXPECT_TRUE(opkt == epkt);
    EXPECT_TRUE(port == TM_PORT_UPLINK_1);
    testcase_end(0, 5);
#endif

    testcase_begin(0, 6);
    ipkt.resize(sizeof(g_snd_pkt3));
    memcpy(ipkt.data(), g_snd_pkt3, sizeof(g_snd_pkt3));
    epkt.resize(sizeof(g_rcv_pkt3_2));
    memcpy(epkt.data(), g_rcv_pkt3_2, sizeof(g_rcv_pkt3_2));
    step_network_pkt(ipkt, TM_PORT_UPLINK_0);
    get_next_pkt(opkt, port, cos);
    EXPECT_TRUE(opkt == epkt);
    EXPECT_TRUE(port == TM_PORT_UPLINK_1);
    testcase_end(0, 6);

    exit_simulation();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
