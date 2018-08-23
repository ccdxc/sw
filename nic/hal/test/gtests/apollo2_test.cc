#include <stdio.h>
#include <iostream>
#include <gtest/gtest.h>

#include <boost/crc.hpp>
#include "boost/property_tree/json_parser.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "sdk/pal.hpp"
#include "sdk/utils.hpp"
#include "sdk/types.hpp"
#include "nic/hal/plugins/cfg/lif/lif_manager_base.hpp"
#include "nic/hal/pd/capri/capri_config.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/capri/capri_qstate.hpp"
#include "nic/hal/pd/p4pd/p4pd_api.hpp"
#include "nic/gen/apollo2/include/p4pd.h"
#include "nic/p4/apollo2/include/defines.h"
#include "nic/p4/apollo2/include/table_sizes.h"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "nic/hal/pd/globalpd/gpd_utils.hpp"

hal_ret_t capri_default_config_init(capri_cfg_t *cfg);
using boost::property_tree::ptree;

#define JRXDMA_PRGM "rxdma_program"
#define JTXDMA_PRGM "txdma_program"
#define JLIFQSTATE  "lif2qstate_map"
#define JPKTBUFFER  "rxdma_to_txdma_buf"
#define JSLACLBASE  "slacl"
#define JLPMBASE    "lpm"

typedef struct __attribute__((__packed__)) lif_qstate_  {
    uint64_t pc : 8;
    uint64_t rsvd : 8;
    uint64_t cos_a : 4;
    uint64_t coa_b : 4;
    uint64_t cos_sel : 8;
    uint64_t eval_last : 8;
    uint64_t host_rings : 4;
    uint64_t total_rings : 4;
    uint64_t pid : 16;
    uint64_t pindex : 16;
    uint64_t cindex : 16;

    uint64_t arm_pindex : 16;
    uint64_t arm_cindex : 16;
    uint64_t sw_pindex : 16;
    uint64_t sw_cindex : 16;
    uint64_t ring0_base : 64;
    uint64_t ring1_base : 64;
    uint64_t ring0_size : 16;
    uint64_t ring1_size : 16;

    uint8_t  pad[(512-320)/8];
} lif_qstate_t;

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
    0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00, 0xAA,
    0xBB, 0xCC, 0xDD, 0xEE, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x40, 0x2F,
    0x00, 0x00, 0x64, 0x65, 0x66, 0x67, 0x0C, 0x0C,
    0x01, 0x01, 0x00, 0x00, 0x88, 0x47, 0x00, 0x0C,
    0x81, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
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

uint64_t g_layer1_smac = 0x00C1C2C3C4C5ULL;
uint32_t g_layer1_sip = 0x0B0B0101;
uint32_t g_layer1_dip = 0x0A0A0101;
uint8_t  g_layer1_proto = 0x6;
uint16_t g_layer1_sport = 0x1234;
uint16_t g_layer1_dport = 0x5678;

uint16_t g_ctag1_vid = 100;
uint16_t g_local_vnic_tag = 100;
uint32_t g_flow_index = 0x31;

uint16_t g_nexthop_index = 0;
uint16_t g_tep_index = 100;
uint32_t g_gw_slot_id = 200;
uint32_t g_gw_dip = 0x0C0C0101;
uint64_t g_gw_dmac = 0x001234567890;

static void
init_service_lif() {
    hal::LIFQState qstate = { 0 };
    qstate.lif_id = APOLLO_SERVICE_LIF;
    qstate.hbm_address = get_start_offset(JLIFQSTATE);
    push_qstate_to_capri(&qstate, 0);

    lif_qstate_t lif_qstate = { 0 };
    lif_qstate.ring0_base = get_start_offset(JPKTBUFFER);
    lif_qstate.ring0_size = log2(get_size_kb(JPKTBUFFER) / 10);
    lif_qstate.total_rings = 1;
    write_qstate(qstate.hbm_address, (uint8_t *)&lif_qstate,
                 sizeof(lif_qstate));
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
        p4pd_hwentry_query(tbl_id, &hwkey_len, &hwmask_len,
                           &hwdata_len);
        if (hash_table) {
            hwkey_len = 64;
        } else {
            hwkey_len = (hwkey_len >> 3) + ((hwkey_len & 0x7) ? 1 : 0);
            hwmask_len = (hwmask_len >> 3) + ((hwmask_len & 0x7) ? 1 : 0);
        }
        hwdata_len = (hwdata_len >> 3) + ((hwdata_len & 0x7) ? 1 : 0);
        hwkey = new uint8_t[hwkey_len];
        hwmask = new uint8_t[hwmask_len];
        memset(hwkey, 0, hwkey_len);
        memset(hwmask, 0, hwmask_len);
        p4pd_hwkey_hwmask_build(tbl_id, key, mask, hwkey, hwmask);
        if (hash_table) {
            index = generate_hash_(hwkey, 64, 0);
            index &= table_size - 1;
        }
        p4pd_entry_write(tbl_id, index, hwkey, hwmask, data);
        delete [] hwkey;
        delete [] hwmask;
    } else {
        p4pd_entry_write(tbl_id, index, NULL, NULL, data);
    }
}

static void
key_native_init(void) {
    key_native_swkey_t key;
    key_native_swkey_mask_t mask;
    key_native_actiondata data;
    uint32_t tbl_id = P4TBL_ID_KEY_NATIVE;
    uint32_t index;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    index = 0;
    data.actionid = KEY_NATIVE_NATIVE_IPV4_PACKET_ID;
    key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_NONE;
    mask.tunnel_metadata_tunnel_type_mask = 0xff;
    key.ipv4_1_valid = 1;
    mask.ipv4_1_valid_mask = 1;

    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
key_tunneled_init(void) {
    key_tunneled_swkey_t key;
    key_tunneled_swkey_mask_t mask;
    key_tunneled_actiondata data;
    uint32_t tbl_id = P4TBL_ID_KEY_TUNNELED;
    uint32_t index;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    index = 0;
    data.actionid = KEY_TUNNELED_TUNNELED_IPV4_PACKET_ID;
    key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;
    mask.tunnel_metadata_tunnel_type_mask = 0xff;
    key.ipv4_2_valid = 1;
    mask.ipv4_2_valid_mask = 1;

    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
vnic_init(void) {
    local_vnic_by_vlan_tx_actiondata data;
    local_vnic_by_vlan_tx_local_vnic_info_tx_t *local_vnic_info =
        &data.local_vnic_by_vlan_tx_action_u.local_vnic_by_vlan_tx_local_vnic_info_tx;
    uint64_t slacl_hbm_addr;
    uint64_t lpm_hbm_addr;
    uint32_t tbl_id = P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX;
    uint32_t index;

    memset(&data, 0, sizeof(data));
    index = g_ctag1_vid;
    data.actionid = LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    local_vnic_info->local_vnic_tag = g_local_vnic_tag;
    local_vnic_info->skip_src_dst_check = true;
    memcpy(local_vnic_info->overlay_mac, &g_layer1_smac, 6);
    slacl_hbm_addr = get_start_offset(JSLACLBASE);
    memcpy(local_vnic_info->slacl_addr_1, &slacl_hbm_addr,
           sizeof(local_vnic_info->slacl_addr_1));
    memcpy(local_vnic_info->slacl_addr_2, &slacl_hbm_addr,
           sizeof(local_vnic_info->slacl_addr_2));
    lpm_hbm_addr = get_start_offset(JLPMBASE);
    memcpy(local_vnic_info->lpm_addr_1, &lpm_hbm_addr,
           sizeof(local_vnic_info->lpm_addr_1));
    memcpy(local_vnic_info->lpm_addr_2, &lpm_hbm_addr,
           sizeof(local_vnic_info->lpm_addr_2));

    entry_write(tbl_id, index, NULL, NULL, &data, false, 0);
}

static void
mappings_init(void) {
    local_ip_mapping_swkey_t key;
    local_ip_mapping_actiondata data;
    local_ip_mapping_local_ip_mapping_info_t *mapping_info =
        &data.local_ip_mapping_action_u.local_ip_mapping_local_ip_mapping_info;
    uint32_t tbl_id = P4TBL_ID_LOCAL_IP_MAPPING;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.vnic_metadata_local_vnic_tag = g_local_vnic_tag;
    memcpy(key.control_metadata_mapping_lkp_addr, &g_layer1_sip, 4);
    data.actionid = LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID;
    mapping_info->entry_valid = true;

    entry_write(tbl_id, 0, &key, NULL, &data, true, VNIC_IP_MAPPING_TABLE_SIZE);
}

static void
flow_init(void) {
    flow_swkey_t key;
    flow_actiondata data;
    flow_flow_hash_t *flow_hash_info = &data.flow_action_u.flow_flow_hash;
    uint32_t tbl_id = P4TBL_ID_FLOW;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_ktype = KEY_TYPE_IPV4;
    key.vnic_metadata_local_vnic_tag = g_local_vnic_tag;
    memcpy(key.key_metadata_src, &g_layer1_sip, 4);
    memcpy(key.key_metadata_dst, &g_layer1_dip, 4);
    key.key_metadata_proto = g_layer1_proto;
    key.key_metadata_sport = g_layer1_sport;
    key.key_metadata_dport = g_layer1_dport;
    data.actionid = FLOW_FLOW_HASH_ID;
    flow_hash_info->entry_valid = true;
    flow_hash_info->flow_index = g_flow_index;

    entry_write(tbl_id, 0, &key, NULL, &data, true, FLOW_TABLE_SIZE);
}

static void
nexthop_tx_init (void) {
    nexthop_tx_actiondata data;
    nexthop_tx_nexthop_info_t *nexthop_info =
        &data.nexthop_tx_action_u.nexthop_tx_nexthop_info;
    uint32_t tbl_id = P4TBL_ID_NEXTHOP_TX;
    uint32_t index;

    memset(&data, 0, sizeof(data));

    index = g_nexthop_index;
    data.actionid = NEXTHOP_TX_NEXTHOP_INFO_ID;
    nexthop_info->tep_index = g_tep_index;
    nexthop_info->encap_type = GW_ENCAP;
    nexthop_info->dst_slot_id = g_gw_slot_id;

    entry_write(tbl_id, index, NULL, NULL, &data, false, 0);
}

static void
tep_tx_init (void) {
    tep_tx_actiondata data;
    tep_tx_tep_tx_t *tep_info =
        &data.tep_tx_action_u.tep_tx_tep_tx;
    uint32_t tbl_id = P4TBL_ID_TEP_TX;
    uint32_t index;

    memset(&data, 0, sizeof(data));

    index = g_tep_index;
    data.actionid = TEP_TX_TEP_TX_ID;
    tep_info->dipo = g_gw_dip;
    memcpy(tep_info->dmac, &g_gw_dmac, 6);

    entry_write(tbl_id, index, NULL, NULL, &data, false, 0);
}

static void
rewrite_init (void) {

    uint64_t mytep_ip = 0x64656667; // 100.101.102.103
    uint64_t mytep_mac = 0x00AABBCCDDEE;

    // Program the table constants
    hal::pd::asicpd_program_table_constant(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX, mytep_ip);
    hal::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP_TX, mytep_ip);
    hal::pd::asicpd_program_table_constant(P4TBL_ID_TEP_TX, mytep_mac);

    nexthop_tx_init();
    tep_tx_init();
}

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
    char *default_config_dir = NULL;
    capri_cfg_t cfg;
    sdk::lib::catalog *catalog;


    p4pd_cfg_t p4pd_cfg = {
        .table_map_cfg_file  = "apollo2/capri_p4_table_map.json",
        .p4pd_pgm_name       = "apollo2_p4",
        .p4pd_rxdma_pgm_name = "apollo2_rxdma",
        .p4pd_txdma_pgm_name = "apollo2_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_rxdma_cfg = {
        .table_map_cfg_file  = "apollo2/capri_rxdma_table_map.json",
        .p4pd_pgm_name       = "apollo2_p4",
        .p4pd_rxdma_pgm_name = "apollo2_rxdma",
        .p4pd_txdma_pgm_name = "apollo2_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_txdma_cfg = {
        .table_map_cfg_file  = "apollo2/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apollo2_p4",
        .p4pd_rxdma_pgm_name = "apollo2_rxdma",
        .p4pd_txdma_pgm_name = "apollo2_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };

    const char *hal_conf_file = "conf/hal.json";
    if (getenv("HAL_PLATFORM_MODE_RTL")) {
        hal_conf_file = "conf/apollo2/hal_rtl.json";
    }

    printf("Connecting to ASIC SIM\n");
    hal::utils::trace_init("hal", 0, true, "hal.log", hal::utils::trace_debug);
    ret = sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM);
    ASSERT_NE(ret, -1);
    printf("Loading Capri config\n");
    ret = capri_load_config((char *)"obj/apollo2_p4/pgm_bin");
    ASSERT_NE(ret, -1);
    ret = capri_load_config((char *)"obj/apollo2_rxdma/pgm_bin");
    ASSERT_NE(ret, -1);
    ret = capri_load_config((char *)"obj/apollo2_txdma/pgm_bin");
    ASSERT_NE(ret, -1);
    cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    cfg.pgm_name = "apollo2";
    printf("Parsing HBM config\n");
    ret = capri_hbm_parse(&cfg);
    ASSERT_NE(ret, -1);

    printf("Loading Programs\n");
    asm_base_addr = (uint64_t)get_start_offset((char *)JP4_PRGM);
    ret = capri_load_mpu_programs("apollo2_p4",
                                  (char *)"obj/apollo2_p4/asm_bin",
                                  asm_base_addr, NULL, 0);
    ASSERT_NE(ret, -1);
    asm_base_addr = (uint64_t)get_start_offset((char *)JRXDMA_PRGM);
    ret = capri_load_mpu_programs("apollo2_rxdma",
                                  (char *)"obj/apollo2_rxdma/asm_bin",
                                  asm_base_addr, NULL, 0);
    ASSERT_NE(ret, -1);
    asm_base_addr = (uint64_t)get_start_offset((char *)JTXDMA_PRGM);
    ret = capri_load_mpu_programs("apollo2_txdma",
                                  (char *)"obj/apollo2_txdma/asm_bin",
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
    ret = capri_tm_asic_init();
    ASSERT_NE(ret, -1);
    ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    ASSERT_NE(ret, -1);
    ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    ASSERT_NE(ret, -1);
    ret = hal::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg);
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

    init_service_lif();
    key_native_init();
    key_tunneled_init();
    vnic_init();
    mappings_init();
    flow_init();
    rewrite_init();

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

    if (getenv("APOLLO2_TCID")) {
        tcid_filter = atoi(getenv("APOLLO2_TCID"));
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
            step_network_pkt(ipkt, port);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
            }
            testcase_end(i, i+1);
        }
    }

    exit_simulation();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
