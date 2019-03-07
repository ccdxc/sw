#include <stdio.h>
#include <map>
#include <iostream>
#include <gtest/gtest.h>

#include <boost/crc.hpp>
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "nic/hal/pd/globalpd/gpd_utils.hpp"
#include "gen/p4gen/l2switch/include/p4pd.h"

#define DERIVE_LIF_TABLE_SIZE                   1024
#define L2SWITCH_TABLE_SIZE                     2048
#define XLATE_VLAN_HOST_TO_NW_TABLE_SIZE        2048
#define XLATE_VLAN_NW_TO_HOST_TABLE_SIZE        2048

using boost::property_tree::ptree;
using namespace sdk::platform::utils;
using namespace sdk::platform::capri;

uint8_t g_snd_pkt1[] = {
    0x00, 0xEE, 0xFF, 0x00, 0x00, 0x03, 0x00, 0xCC,
    0x00, 0x00, 0x00, 0x02, 0x81, 0x00, 0x00, 0x02,
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
    0x00, 0xEE, 0xFF, 0x00, 0x00, 0x03, 0x00, 0xCC,
    0x00, 0x00, 0x00, 0x02, 0x81, 0x00, 0x01, 0x01,
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

static uint8_t *
memrev(uint8_t *block, size_t elnum) {
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

static int
sdk_trace_cb (sdk_trace_level_e trace_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    switch (trace_level) {
    case sdk::lib::SDK_TRACE_LEVEL_NONE:
        return 0;
        break;
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        printf("[E] ");
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        printf("[W] ");
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        printf("[I] ");
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        printf("[D] ");
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        printf("[V] ");
        break;
    default:
        break;
    }
    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    printf(logbuf);
    printf("\n");
    va_end(args);

    return 0;
}

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

class l2switch_test : public ::testing::Test {
  protected:
    l2switch_test() {}
    virtual ~l2switch_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(l2switch_test, test1) {
    int ret = 0;
    char *default_config_dir = NULL;
    asic_cfg_t cfg;
    sdk::lib::catalog *catalog;

    p4pd_cfg_t p4pd_cfg = {
        .table_map_cfg_file  = "l2switch/capri_p4_table_map.json",
        .p4pd_pgm_name       = "l2switch_p4",
        .p4pd_rxdma_pgm_name = "",
        .p4pd_txdma_pgm_name = "",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };

    cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    const char *hal_conf_file = "l2switch/hal.json";
    std::string mpart_json = cfg.cfg_path + "/l2switch/hbm_mem.json";
    platform_type_t platform = platform_type_t::PLATFORM_TYPE_SIM;
    printf("Parsing sim catalog ...\n");
    catalog = sdk::lib::catalog::factory(cfg.cfg_path, "catalog.json");
    ASSERT_TRUE(catalog != NULL);
    cfg.catalog = catalog;
    printf("\nMPART_JSON %s\n", mpart_json.c_str());
    cfg.mempartition = sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    std::ifstream json_cfg(cfg.cfg_path + hal_conf_file);
    ptree hal_conf;
    read_json(json_cfg, hal_conf);
    cfg.loader_info_file = hal_conf.get<std::string>("asic.loader_info_file").c_str();

    default_config_dir = std::getenv("HAL_PBC_INIT_CONFIG");
    if (default_config_dir) {
        cfg.default_config_dir = std::string(default_config_dir);
    } else {
        cfg.default_config_dir = "8x25_hbm";
    }

    cfg.platform = platform;
    cfg.admin_cos = 1;
    cfg.pgm_name = std::string("l2switch");

    printf("Connecting to ASIC\n");
    sdk::lib::logger::init(sdk_trace_cb);
    ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM);

    cfg.num_pgm_cfgs = 1;
    memset(cfg.pgm_cfg, 0, sizeof(cfg.pgm_cfg));
    cfg.pgm_cfg[0].path = std::string("p4_bin");

    cfg.num_asm_cfgs = 1;
    memset(cfg.asm_cfg, 0, sizeof(cfg.asm_cfg));
    cfg.asm_cfg[0].name = std::string("l2switch_p4");
    cfg.asm_cfg[0].path = std::string("p4_asm");
    cfg.asm_cfg[0].base_addr = std::string(JP4_PRGM);
    cfg.completion_func = NULL;

    printf("Doing asic init ...\n");
    ret = sdk::asic::asic_init(&cfg);
    ASSERT_EQ(ret, SDK_RET_OK);
    printf("Doing p4pd init ...\n");
    ret = p4pd_init(&p4pd_cfg);
    ASSERT_EQ(ret, HAL_RET_OK);
    printf("Doing p4 mpu init ...\n");
    ret = sdk::asic::pd::asicpd_table_mpu_base_init(&p4pd_cfg);
    ASSERT_EQ(ret, SDK_RET_OK);
    printf("Programming mpu PC ...\n");
    ret = sdk::asic::pd::asicpd_program_table_mpu_pc();
    ASSERT_EQ(ret, SDK_RET_OK);
    printf("Doing deparser init ...\n");
    ret = sdk::asic::pd::asicpd_deparser_init();
    ASSERT_EQ(ret, SDK_RET_OK);
    printf("Programming HBM table base addresses ...\n");
    ret = sdk::asic::pd::asicpd_program_hbm_table_base_addr();
    ASSERT_EQ(ret, SDK_RET_OK);

    config_done();

    {
        derive_lif_swkey_t key;
        derive_lif_swkey_mask_t key_mask;
        derive_lif_actiondata_t data;

       memset(&key, 0, sizeof(key));
       memset(&key_mask, 0, sizeof(key_mask));
       memset(&data, 0, sizeof(data));

       unsigned char src_addr[6] = {0x02, 0x00, 0x00, 0x00, 0xCC, 0x00}; 
       memcpy(key.ethernet_srcAddr, src_addr, 6);
       key.vlan_tag_vid = 002;
       key.control_metadata_uplink = 1;
       
       unsigned char src_addr_mask[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
       memcpy(key_mask.ethernet_srcAddr_mask, src_addr_mask, 6);
       key_mask.vlan_tag_vid_mask = 0xFFFF;
       key_mask.control_metadata_uplink_mask = 0xFF;

      data.action_id = DERIVE_LIF_DERIVE_LIF_ACTION_ID;
      data.action_u.derive_lif_derive_lif_action.dir = 0;
      data.action_u.derive_lif_derive_lif_action.src_lif_check_en = 0;
      data.action_u.derive_lif_derive_lif_action.src_lif = 1;

      entry_write(P4TBL_ID_DERIVE_LIF, 0, &key, &key_mask, &data, false, 0);

    }

    {
        l2switch_table_swkey_t key;
        l2switch_table_actiondata_t data;

       memset(&key, 0, sizeof(key));
       memset(&data, 0, sizeof(data));

       unsigned char dst_addr[6] = {0x03, 0x00, 0x00, 0xFF, 0xEE, 0x00}; 
       memcpy(key.ethernet_dstAddr, dst_addr, 6);
       key.vlan_tag_vid = 002;

       data.action_id = L2SWITCH_TABLE_L2SWITCH_ID;
       data.action_u.l2switch_table_l2switch.dst_lport = 1;

       entry_write(P4TBL_ID_L2SWITCH_TABLE, 0, &key, NULL, &data, true, L2SWITCH_TABLE_SIZE);

    }

    {
       xlate_vlan_host_to_nw_swkey_t key;
       xlate_vlan_host_to_nw_actiondata_t data;

       memset(&key, 0, sizeof(key));
       memset(&data, 0, sizeof(data));

       key.control_metadata_src_lif = 1;
     
       data.action_id = XLATE_VLAN_HOST_TO_NW_XLATE_VLAN_ID;
       data.action_u.xlate_vlan_host_to_nw_xlate_vlan.vid = 0x101;

       entry_write(P4TBL_ID_XLATE_VLAN_HOST_TO_NW, 1, NULL, NULL, &data, true, XLATE_VLAN_HOST_TO_NW_TABLE_SIZE);
    }


    {
       xlate_vlan_nw_to_host_swkey_t key;
       xlate_vlan_nw_to_host_actiondata_t data;

       memset(&key, 0, sizeof(key));
       memset(&data, 0, sizeof(data));

       key.control_metadata_src_lif = 1;
     
       data.action_id = XLATE_VLAN_NW_TO_HOST_XLATE_VLAN_ID;
       data.action_u.xlate_vlan_nw_to_host_xlate_vlan.vid = 0x2;

       entry_write(P4TBL_ID_XLATE_VLAN_NW_TO_HOST, 1, NULL, NULL, &data, true, XLATE_VLAN_NW_TO_HOST_TABLE_SIZE);
    }


    {
        uint32_t port = 0;
        uint32_t cos = 0;
        std::vector<uint8_t> ipkt;
        std::vector<uint8_t> opkt;
        std::vector<uint8_t> epkt;
        uint32_t i = 0;
        uint32_t tcscale = 1;
        int tcid = 0;
        int tcid_filter = 0;

        if (getenv("TCSCALE")) {
            tcscale = atoi(getenv("TCSCALE"));
        }

        if (getenv("TCID")) {
            tcid_filter = atoi(getenv("TCID"));
        }

        tcid++;
        if (tcid_filter == 0 || tcid == tcid_filter) {
            ipkt.resize(sizeof(g_snd_pkt1));
            memcpy(ipkt.data(), g_snd_pkt1, sizeof(g_snd_pkt1));
            epkt.resize(sizeof(g_rcv_pkt1));
            memcpy(epkt.data(), g_rcv_pkt1, sizeof(g_rcv_pkt1));
            std::cout << "Testing Host to Switch" << std::endl;
            for (i = 0; i < tcscale; i++) {
                testcase_begin(tcid, i+1);
                step_network_pkt(ipkt, 0);
                if (!getenv("SKIP_VERIFY")) {
                    get_next_pkt(opkt, port, cos);

                    printf("\nIPKT PORT %d\n", 0);
                    printf("===> IPKET START\n");

                    for(int iter = 0; iter < ipkt.size(); iter++) {
                        if(iter != 0 && iter % 8 == 0) {
                           printf("\n");
                        }
                        printf(" %02x ", ipkt[iter]);
                    }
                    printf("\n");

                    printf("\nOPKT PORT %d COS %d\n", port, cos);
                    printf("===> OPKET START\n");

                    for(int iter = 0; iter < opkt.size(); iter++) {
                        if(iter != 0 && iter % 8 == 0) {
                           printf("\n");
                        }
                        printf(" %02x ", opkt[iter]);
                    }
                    printf("\n");

                    EXPECT_TRUE(opkt == epkt);
                    EXPECT_TRUE(port == 1);
                }
                testcase_end(tcid, i+1);
            }
        }
    }
    exit_simulation();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
