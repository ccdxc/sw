//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
// This file contains the basic apulu pipeline test case
//----------------------------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include <map>
#include <iostream>
#include <gtest/gtest.h>
#include <arpa/inet.h>

#include <boost/crc.hpp>
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/device/device.hpp"
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "platform/utils/lif_manager_base.hpp"
#include "platform/capri/capri_qstate.hpp"
#include "nic/sdk/platform/capri/capri_hbm_rw.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "nic/apollo/p4/include/apulu_table_sizes.h"
#include "nic/apollo/core/trace.hpp"
#include "gen/platform/mem_regions.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "nic/apollo/test/golden/apulu_pkts.h"

#define LOG2_U32(x) (((x) == 0) ? 0 : (31 - __builtin_clz((x))))

#define EPOCH 0x55

using namespace sdk::platform::utils;
using namespace sdk::platform::capri;

#define JRXDMA_PRGM     "rxdma_program"
#define JTXDMA_PRGM     "txdma_program"
#define JLIFQSTATE      "lif2qstate_map"
#define JPKTBUFFER      "rxdma_to_txdma_buf"
#define JPKTDESC        "rxdma_to_txdma_desc"
#define JSTATSBASE      "session_stats"
#define JP4_PRGM        "p4_program"

#define MEM_REGION_LIF_STATS_BASE       "lif_stats_base"

#define RXDMA_SYMBOLS_MAX   1
#define TXDMA_SYMBOLS_MAX   1

#define UDP_SPORT_OFFSET    34
#define UDP21_SPORT_OFFSET  66
#define UDP22_SPORT_OFFSET  84
#define UDP_SPORT_SIZE      2

typedef struct __attribute__((__packed__)) lifqstate_ {
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

    uint16_t sw_pindex;
    uint16_t sw_cindex;
    uint64_t ring0_base : 64;
    uint64_t ring1_base : 64;
    uint64_t ring_size : 16;
    uint64_t rxdma_cindex_addr : 64;

    uint8_t pad[(512 - 336) / 8];
} lifqstate_t;

typedef struct cache_line_s {
    uint8_t action_pc;
    uint8_t packed_entry[CACHE_LINE_SIZE-sizeof(action_pc)];
} cache_line_t;

uint32_t g_lif0 = 0x1;
uint32_t g_lif1 = 0x2;
uint16_t g_vpc_id = 0x2FC;
uint16_t g_bd_id = 0x2FD;
uint16_t g_vnic_id = 0x2FE;
uint32_t g_device_ipv4_addr = 0x64656667;
uint64_t g_device_mac = 0x00AABBCCDDEEULL;
uint32_t g_nexthop_id_arm = 0x137;
uint32_t g_flow_ohash_id = 0x4A54;
uint32_t g_mapping_ohash_id = 0x77A10;
uint32_t g_local_mapping_ohash_id = 0x1177;

uint64_t g_dmac1 = 0x000102030405ULL;
uint64_t g_smac1 = 0x00C1C2C3C4C5ULL;
uint16_t g_ctag1 = 0x64;
uint32_t g_sip1 = 0x0B0B0101;
uint32_t g_dip1 = 0x0A0A0101;
uint8_t  g_proto1 = 0x6;
uint16_t g_sport1 = 0x1234;
uint16_t g_dport1 = 0x5678;

uint16_t g_vpc_id1 = 0x2EC;
uint16_t g_bd_id1 = 0x2ED;
uint16_t g_vnic_id1 = 0x2EE;
uint16_t g_egress_bd_id1 = 0x2FE;
uint32_t g_session_id1 = 0x55E51;
uint32_t g_tunnel_id1 = 0x1EF;
uint32_t g_nexthop_id1 = 0x2EF;
uint32_t g_nexthop_id2 = 0x3EF;
uint64_t g_dmaci1 = 0x001112131415ULL;
uint64_t g_vrmac1 = 0x00D1D2D3D4D5ULL;
uint64_t g_dmaco1 = 0x001234567890ULL;
uint32_t g_vni1 = 0xABCDEF;
uint32_t g_dipo1 = 0x0C0C0101;
uint16_t g_binding_id1 = 0xB1D;

uint32_t g_dip3 = 0x0A0A0A0A;

uint32_t g_dip4 = 0x0A0A0202;
uint32_t g_dipo4 = 0x0D0D0101;
uint32_t g_session_id4 = 0x55E54;
uint32_t g_tunnel_id4 = 0x1E4;
uint32_t g_nexthop_id4 = 0x2E4;
uint32_t g_tunnel2_id4 = 0x74;
uint32_t g_vni4 = 0xFEED;

uint32_t g_dip5 = 0x0A0A0203;
uint32_t g_tunnel_id5 = 0x1E5;
uint32_t g_nexthop_id5 = 0x2E5;
uint32_t g_tunnel2_id5 = 0x75;
uint32_t g_vni5 = 0xAFEED;

uint32_t g_dip6 = 0x0A0A0102;

uint32_t g_sip7 = 0x0B0B0107;
uint32_t g_dip7 = 0x0A0A0103;

uint32_t g_dip9 = 0x0A0A0109;

uint32_t g_sip11 = 0x0B0B010B;
uint16_t g_vnic_id11 = 0x20B;
uint8_t  g_mirror_id1 = 1;
uint8_t  g_mirror_id2 = 5;
uint64_t g_erspan_dmac1 = 0x000E0E0E0E0EULL;
uint64_t g_erspan_smac1 = 0x00E1E2E3E4E5ULL;
uint32_t g_erspan_dip1 = 0xC8010102;
uint32_t g_erspan_sip1 = 0xC8010101;
uint32_t g_tunnel2_id11 = 0x7B;

uint32_t g_sip12 = 0x0B0B010C;
uint16_t g_vnic_id12 = 0x20C;

uint8_t  g_proto13 = 0x11;
uint16_t g_sport13 = 0x1234;
uint16_t g_dport13 = 6081;
mpartition *g_mempartition;

class sort_mpu_programs_compare {
public:
    bool operator()(std::string p1, std::string p2) {
        std::map<std::string, p4pd_table_properties_t>::iterator it1, it2;
        it1 = tbl_map.find(p1);
        it2 = tbl_map.find(p2);
        if ((it1 == tbl_map.end()) || (it2 == tbl_map.end())) {
            return (p1 < p2);
        }
        p4pd_table_properties_t tbl_ctx1 = it1->second;
        p4pd_table_properties_t tbl_ctx2 = it2->second;
        if (tbl_ctx1.gress != tbl_ctx2.gress) {
            return (tbl_ctx1.gress < tbl_ctx2.gress);
        }
        if (tbl_ctx1.stage != tbl_ctx2.stage) {
            return (tbl_ctx1.stage < tbl_ctx2.stage);
        }
        return (tbl_ctx1.stage_tableid < tbl_ctx2.stage_tableid);
    }
    void add_table(std::string tbl_name, p4pd_table_properties_t tbl_ctx) {
        std::pair<std::string, p4pd_table_properties_t> key_value;
        key_value = std::make_pair(tbl_name.append(".bin"), tbl_ctx);
        tbl_map.insert(key_value);
    }

private:
    std::map<std::string, p4pd_table_properties_t> tbl_map;
};

static void
sort_mpu_programs (std::vector<std::string> &programs)
{
    sort_mpu_programs_compare sort_compare;
    for (uint32_t tableid = p4pd_tableid_min_get();
         tableid < p4pd_tableid_max_get(); tableid++) {
        p4pd_table_properties_t tbl_ctx;
        if (p4pd_table_properties_get(tableid, &tbl_ctx) != P4PD_FAIL) {
            sort_compare.add_table(std::string(tbl_ctx.tablename), tbl_ctx);
        }
    }
    sort(programs.begin(), programs.end(), sort_compare);
}

#ifdef SIM
static bool
is_equal_encap_pkt (std::vector<uint8_t> pkt1, std::vector<uint8_t> pkt2)
{
    if (pkt1.size() != pkt2.size()) {
       return false;
    }

    return (std::equal(pkt1.begin(),
                       pkt1.begin() + UDP_SPORT_OFFSET, pkt2.begin()) &&
            std::equal(pkt1.begin() + UDP_SPORT_OFFSET + UDP_SPORT_SIZE,
                       pkt1.end(),
                       pkt2.begin() + UDP_SPORT_OFFSET + UDP_SPORT_SIZE));
}

static bool
is_equal_double_encap_pkt (std::vector<uint8_t> pkt1, std::vector<uint8_t> pkt2,
                           uint8_t encap_type)
{
    if (pkt1.size() != pkt2.size()) {
       return false;
    }

    uint8_t udp2_sport_offset = (encap_type == TX_REWRITE_ENCAP_VXLAN) ?
        UDP22_SPORT_OFFSET : UDP21_SPORT_OFFSET;
    return (std::equal(pkt1.begin(),
                       pkt1.begin() + UDP_SPORT_OFFSET, pkt2.begin()) &&
            std::equal(pkt1.begin() + UDP_SPORT_OFFSET + UDP_SPORT_SIZE,
                       pkt1.begin() + udp2_sport_offset,
                       pkt2.begin() + UDP_SPORT_OFFSET + UDP_SPORT_SIZE) &&
            std::equal(pkt1.begin() + udp2_sport_offset + UDP_SPORT_SIZE,
                       pkt1.end(),
                       pkt2.begin() + udp2_sport_offset + UDP_SPORT_SIZE));
}
#endif

static void
p4plus_table_init (platform_type_t platform_type)
{
    p4pd_table_properties_t tbl_ctx;
    p4plus_prog_t prog;

    p4pd_global_table_properties_get(P4_P4PLUS_TXDMA_TBL_ID_TX_TABLE_S0_T0,
                                     &tbl_ctx);
    memset(&prog, 0, sizeof(prog));
    prog.stageid = tbl_ctx.stage;
    prog.stage_tableid = tbl_ctx.stage_tableid;
    prog.control = "apulu_txdma";
    prog.prog_name = "txdma_stage0.bin";
    prog.pipe = P4_PIPELINE_TXDMA;
    sdk::asic::pd::asicpd_p4plus_table_init(&prog, platform_type);
}

static void
init_service_lif ()
{
    LIFQState qstate = {0};

    qstate.lif_id = APULU_SERVICE_LIF;
    qstate.hbm_address = asicpd_get_mem_addr(JLIFQSTATE);
    qstate.params_in.type[0].entries = 1;
    qstate.params_in.type[0].size = 1;
    push_qstate_to_capri(&qstate, 0);

    uint64_t pc;
    int ret;
    char progname[] = "txdma_stage0.bin";
    char labelname[] = "apollo_read_qstate";
    ret = sdk::p4::p4_program_label_to_offset("apulu_txdma", progname,
                                              labelname, &pc);
    SDK_ASSERT(ret == 0);

    lifqstate_t lif_qstate = {0};
    lif_qstate.ring0_base = asicpd_get_mem_addr(JPKTBUFFER);
    lif_qstate.ring1_base = asicpd_get_mem_addr(JPKTDESC);
    lif_qstate.ring_size = log2(asicpd_get_mem_size_kb(JPKTBUFFER) / 10);
    lif_qstate.total_rings = 1;
    write_qstate(qstate.hbm_address, (uint8_t *)&lif_qstate,
                 sizeof(lif_qstate));

    lifqstate_t txdma_qstate = {0};
    txdma_qstate.pc = pc >> 6;
    txdma_qstate.rxdma_cindex_addr =
        qstate.hbm_address + offsetof(lifqstate_t, sw_cindex);
    txdma_qstate.ring0_base = asicpd_get_mem_addr(JPKTBUFFER);
    txdma_qstate.ring1_base = asicpd_get_mem_addr(JPKTDESC);
    txdma_qstate.ring_size = log2(asicpd_get_mem_size_kb(JPKTBUFFER) / 10);
    txdma_qstate.total_rings = 1;
    write_qstate(qstate.hbm_address + sizeof(lifqstate_t),
                 (uint8_t *)&txdma_qstate, sizeof(txdma_qstate));
}

static int
sdk_trace_cb (uint32_t mod_id, sdk_trace_level_e trace_level,
              const char *format, ...)
{
    char logbuf[1024];
    va_list args;

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
generate_hash_ (void *key, uint32_t key_len, uint32_t crc_init_val)
{
    boost::crc_basic<32> *crc_hash;
    uint32_t hash_val = 0x0;

    crc_hash = new boost::crc_basic<32>(0x04C11DB7, crc_init_val, 0x00000000,
                                        false, false);
    crc_hash->process_bytes(key, key_len);
    hash_val = crc_hash->checksum();
    delete crc_hash;
    return hash_val;
}

static uint32_t
entry_write (uint32_t tbl_id, uint32_t index, void *key, void *mask, void *data,
             bool hash_table, uint32_t table_size)
{
    uint32_t hash = 0;
    if (key || mask) {
        // prepare entry and write hardware
        uint32_t hwkey_len = 0;
        uint32_t hwmask_len = 0;
        uint32_t hwdata_len = 0;
        uint8_t *hwkey = NULL;
        uint8_t *hwmask = NULL;
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
            if (index == 0) {
                hash = index = generate_hash_(hwkey, hwkey_len, 0);
            }
            index &= table_size - 1;
        }
        p4pd_entry_write(tbl_id, index, hwkey, hwmask, data);
        delete[] hwkey;
        delete[] hwmask;
    } else {
        p4pd_entry_write(tbl_id, index, NULL, NULL, data);
    }
    return hash;
}

static uint32_t
rxdma_symbols_init (void **p4plus_symbols,
                    platform_type_t platform_type)
{
    uint32_t    i = 0;

    *p4plus_symbols = (sdk::p4::p4_param_info_t *)
        SDK_CALLOC(SDK_MEM_ALLOC_PDS_RXDMA_SYMBOLS,
                   RXDMA_SYMBOLS_MAX * sizeof(sdk::p4::p4_param_info_t));
    sdk::p4::p4_param_info_t *symbols =
        (sdk::p4::p4_param_info_t *)(*p4plus_symbols);

    symbols[i].name = MEM_REGION_LIF_STATS_BASE;
    symbols[i].val = g_mempartition->
                     start_addr(MEM_REGION_LIF_STATS_NAME);
    i++;
    SDK_ASSERT(i <= RXDMA_SYMBOLS_MAX);

    return i;
}

static uint32_t
txdma_symbols_init (void **p4plus_symbols,
                    platform_type_t platform_type)
{
    uint32_t    i = 0;

    *p4plus_symbols = (sdk::p4::p4_param_info_t *)
        SDK_CALLOC(SDK_MEM_ALLOC_PDS_TXDMA_SYMBOLS,
                   TXDMA_SYMBOLS_MAX * sizeof(sdk::p4::p4_param_info_t));
    sdk::p4::p4_param_info_t *symbols =
        (sdk::p4::p4_param_info_t *)(*p4plus_symbols);

    symbols[i].name = MEM_REGION_LIF_STATS_BASE;
    symbols[i].val = g_mempartition->start_addr(MEM_REGION_LIF_STATS_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    SDK_ASSERT(i <= TXDMA_SYMBOLS_MAX);

    return i;
}

static void
device_init (void)
{
    p4i_device_info_actiondata_t p4i_data;
    p4i_device_info_p4i_device_info_t *p4i_info =
        &p4i_data.action_u.p4i_device_info_p4i_device_info;
    p4e_device_info_actiondata_t p4e_data;
    p4e_device_info_p4e_device_info_t *p4e_info =
        &p4e_data.action_u.p4e_device_info_p4e_device_info;

    memset(&p4i_data, 0, sizeof(p4i_data));
    memcpy(p4i_info->device_mac_addr1, &g_device_mac, 6);
    p4i_info->device_ipv4_addr = g_device_ipv4_addr;
    entry_write(P4TBL_ID_P4I_DEVICE_INFO, 0, NULL, NULL, &p4i_data, false, 0);

    memset(&p4e_data, 0, sizeof(p4e_data));
    p4e_info->device_ipv4_addr = g_device_ipv4_addr;
    entry_write(P4TBL_ID_P4E_DEVICE_INFO, 0, NULL, NULL, &p4e_data, false, 0);

    capri_tm_uplink_lif_set(TM_PORT_UPLINK_0, g_lif0);
    capri_tm_uplink_lif_set(TM_PORT_UPLINK_1, g_lif1);

    uint64_t session_stats_addr;
    session_stats_addr = asicpd_get_mem_addr(JSTATSBASE);
    session_stats_addr -= ((uint64_t)1 << 31);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_SESSION,
                                                 session_stats_addr);
}

static void
nacl_init ()
{
    nacl_swkey_t key;
    nacl_swkey_mask_t mask;
    nacl_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_NACL;
    uint32_t index;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    index = 0;
    key.control_metadata_flow_miss = 1;
    mask.control_metadata_flow_miss_mask = 1;
    data.action_id = NACL_NACL_REDIRECT_TO_ARM_ID;
    data.action_u.nacl_nacl_redirect_to_arm.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    data.action_u.nacl_nacl_redirect_to_arm.nexthop_id = g_nexthop_id_arm;
    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
checksum_init (void)
{
    checksum_swkey_t key;
    checksum_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_CHECKSUM;
    uint64_t idx;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV4_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(tbl_id, &key);
    entry_write(tbl_id, idx, NULL, NULL, &data, false, CHECKSUM_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 1;
    key.udp_1_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV4_UDP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(tbl_id, &key);
    entry_write(tbl_id, idx, NULL, NULL, &data, false, CHECKSUM_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 1;
    key.tcp_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV4_TCP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(tbl_id, &key);
    entry_write(tbl_id, idx, NULL, NULL, &data, false, CHECKSUM_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 1;
    key.icmp_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV4_ICMP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(tbl_id, &key);
    entry_write(tbl_id, idx, NULL, NULL, &data, false, CHECKSUM_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv6_1_valid = 1;
    key.udp_1_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV6_UDP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(tbl_id, &key);
    entry_write(tbl_id, idx, NULL, NULL, &data, false, CHECKSUM_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv6_1_valid = 1;
    key.tcp_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV6_TCP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(tbl_id, &key);
    entry_write(tbl_id, idx, NULL, NULL, &data, false, CHECKSUM_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv6_1_valid = 1;
    key.icmp_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV6_ICMP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(tbl_id, &key);
    entry_write(tbl_id, idx, NULL, NULL, &data, false, CHECKSUM_TABLE_SIZE);
}

static void
lif_table_init (void)
{
    lif_actiondata_t data;
    lif_lif_info_t *lif_info = &data.action_u.lif_lif_info;
    uint16_t tbl_id = P4TBL_ID_LIF;

    memset(&data, 0, sizeof(data));
    data.action_id = LIF_LIF_INFO_ID;
    lif_info->vpc_id = g_vpc_id;
    lif_info->bd_id = g_bd_id;
    lif_info->vnic_id = g_vnic_id;
    lif_info->direction = P4_LIF_DIR_HOST;
    entry_write(tbl_id, g_lif0, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    data.action_id = LIF_LIF_INFO_ID;
    lif_info->vpc_id = g_vpc_id;
    lif_info->bd_id = 0;
    lif_info->vnic_id = 0;
    lif_info->direction = P4_LIF_DIR_UPLINK;
    entry_write(tbl_id, g_lif1, 0, 0, &data, false, 0);
}

static void
vlan_table_init (void)
{
    vlan_actiondata_t data;
    vlan_vlan_info_t *vlan_info = &data.action_u.vlan_vlan_info;
    uint16_t tbl_id = P4TBL_ID_VLAN;
    uint16_t index = g_ctag1;

    memset(&data, 0, sizeof(data));
    data.action_id = VLAN_VLAN_INFO_ID;
    vlan_info->vpc_id = g_vpc_id1;
    vlan_info->bd_id = g_bd_id1;
    vlan_info->vnic_id = 0;
    entry_write(tbl_id, index, 0, 0, &data, false, 0);
}

static void
vni_table_init (void)
{
    vni_swkey_t key;
    vni_actiondata_t data;
    vni_vni_info_t *vni_info = &data.action_u.vni_vni_info;
    uint16_t tbl_id = P4TBL_ID_VNI;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.vxlan_1_vni = g_vni1;
    data.action_id = VNI_VNI_INFO_ID;
    vni_info->vpc_id = g_vpc_id1;
    vni_info->bd_id = g_bd_id1;
    vni_info->vnic_id = 0;
    entry_write(tbl_id, 0, &key, 0, &data, true, VNI_HASH_TABLE_SIZE);
}

static void
input_properties_init (void)
{
    lif_table_init();
    vlan_table_init();
    vni_table_init();
}

static void
local_mappings_init (void)
{
    local_mapping_swkey_t key;
    local_mapping_actiondata_t data;
    local_mapping_local_mapping_info_t *local_info =
        &data.action_u.local_mapping_local_mapping_info;
    uint32_t tbl_id = P4TBL_ID_LOCAL_MAPPING;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV4;
    key.key_metadata_local_mapping_lkp_id = g_vpc_id1;
    memcpy(key.key_metadata_local_mapping_lkp_addr, &g_sip1, 4);
    local_info->entry_valid = 1;
    local_info->vnic_id = g_vnic_id1;
    local_info->binding_check_enabled = 1;
    local_info->binding_id1 = g_binding_id1;
    local_info->allow_tagged_pkts = 1;
    entry_write(tbl_id, 0, &key, NULL, &data, true, LOCAL_MAPPING_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV4;
    key.key_metadata_local_mapping_lkp_id = g_vpc_id1;
    memcpy(key.key_metadata_local_mapping_lkp_addr, &g_sip11, 4);
    local_info->entry_valid = 1;
    local_info->vnic_id = g_vnic_id11;
    local_info->binding_check_enabled = 1;
    local_info->binding_id1 = g_binding_id1;
    local_info->allow_tagged_pkts = 1;
    entry_write(tbl_id, 0, &key, NULL, &data, true, LOCAL_MAPPING_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV4;
    key.key_metadata_local_mapping_lkp_id = g_vpc_id1;
    memcpy(key.key_metadata_local_mapping_lkp_addr, &g_sip12, 4);
    local_info->entry_valid = 1;
    local_info->vnic_id = g_vnic_id12;
    local_info->binding_check_enabled = 0;
    local_info->allow_tagged_pkts = 1;
    entry_write(tbl_id, 0, &key, NULL, &data, true, LOCAL_MAPPING_TABLE_SIZE);
}

static void
local_mappings_with_ohash_init (void)
{
    local_mapping_swkey_t key;
    local_mapping_actiondata_t data;
    local_mapping_local_mapping_info_t *local_info =
        &data.action_u.local_mapping_local_mapping_info;
    uint32_t hash = 0;
    uint32_t hint = 0;
    uint32_t hint_nxt = 0;

    // main table
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV4;
    key.key_metadata_local_mapping_lkp_id = g_vpc_id1;
    memcpy(key.key_metadata_local_mapping_lkp_addr, &g_sip7, 4);
    hash = entry_write(P4TBL_ID_LOCAL_MAPPING, 0, &key, NULL, &data, true,
                       LOCAL_MAPPING_TABLE_SIZE);
    hint = g_local_mapping_ohash_id++;
    memset(&key, 0, sizeof(key));
    local_info->entry_valid = 1;
    local_info->hash1 = hash >> LOG2_U32(LOCAL_MAPPING_TABLE_SIZE);
    local_info->hint1 = hint;
    entry_write(P4TBL_ID_LOCAL_MAPPING, hash, &key, NULL, &data, true,
                LOCAL_MAPPING_TABLE_SIZE);
    // ohash table, force recirc
    hint_nxt = g_local_mapping_ohash_id++;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    local_info->entry_valid = 1;
    local_info->hash1 = hash >> LOG2_U32(LOCAL_MAPPING_TABLE_SIZE);
    local_info->hint1 = hint_nxt;
    entry_write(P4TBL_ID_LOCAL_MAPPING_OHASH, hint, &key, NULL, &data, true,
                LOCAL_MAPPING_OHASH_TABLE_SIZE);
    // ohash table, force recirc
    hint = hint_nxt;
    hint_nxt = g_local_mapping_ohash_id++;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    local_info->entry_valid = 1;
    local_info->hash1 = hash >> LOG2_U32(LOCAL_MAPPING_TABLE_SIZE);
    local_info->hint1 = hint_nxt;
    entry_write(P4TBL_ID_LOCAL_MAPPING_OHASH, hint, &key, NULL, &data, true,
                LOCAL_MAPPING_OHASH_TABLE_SIZE);
    // ohash table, force recirc
    hint = hint_nxt;
    hint_nxt = g_local_mapping_ohash_id++;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    local_info->entry_valid = 1;
    local_info->hash1 = hash >> LOG2_U32(LOCAL_MAPPING_TABLE_SIZE);
    local_info->hint1 = hint_nxt;
    entry_write(P4TBL_ID_LOCAL_MAPPING_OHASH, hint, &key, NULL, &data, true,
                LOCAL_MAPPING_OHASH_TABLE_SIZE);
    // ohash table with result
    hint = hint_nxt;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV4;
    key.key_metadata_local_mapping_lkp_id = g_vpc_id1;
    memcpy(key.key_metadata_local_mapping_lkp_addr, &g_sip7, 4);
    local_info->entry_valid = 1;
    local_info->vnic_id = g_vnic_id1;
    local_info->binding_check_enabled = 1;
    local_info->binding_id1 = g_binding_id1;
    local_info->allow_tagged_pkts = 1;
    entry_write(P4TBL_ID_LOCAL_MAPPING_OHASH, hint, &key, NULL, &data, true,
                LOCAL_MAPPING_OHASH_TABLE_SIZE);
}

static void
bindings_init (void)
{
    ip_mac_binding_actiondata_t data;
    ip_mac_binding_binding_info_t *binding_info =
        &data.action_u.ip_mac_binding_binding_info;
    uint32_t tbl_id = P4TBL_ID_IP_MAC_BINDING;

    memset(&data, 0, sizeof(data));
    memcpy(binding_info->addr, &g_smac1, 6);
    entry_write(tbl_id, g_binding_id1, NULL, NULL, &data, false, 0);
}

static void
mappings_init (void)
{
    mapping_swkey_t key;
    mapping_actiondata_t data;
    mapping_mapping_info_t *mapping_info = &data.action_u.mapping_mapping_info;
    uint32_t tbl_id = P4TBL_ID_MAPPING;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip1, 4);
    mapping_info->entry_valid = 1;
    mapping_info->nexthop_valid = 1;
    mapping_info->nexthop_type = NEXTHOP_TYPE_TUNNEL;
    mapping_info->nexthop_id = g_tunnel_id1;
    mapping_info->egress_bd_id = g_egress_bd_id1;
    memcpy(mapping_info->dmaci, &g_dmaci1, 6);
    entry_write(tbl_id, 0, &key, NULL, &data, true, MAPPING_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_sip1, 4);
    mapping_info->entry_valid = 1;
    mapping_info->nexthop_valid = 1;
    mapping_info->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    mapping_info->nexthop_id = g_nexthop_id2;
    mapping_info->egress_bd_id = g_bd_id1;
    memcpy(mapping_info->dmaci, &g_smac1, 6);
    entry_write(tbl_id, 0, &key, NULL, &data, true, MAPPING_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip3, 4);
    mapping_info->entry_valid = 1;
    mapping_info->nexthop_valid = 1;
    mapping_info->nexthop_type = NEXTHOP_TYPE_TUNNEL;
    mapping_info->nexthop_id = g_tunnel_id1;
    mapping_info->egress_bd_id = g_egress_bd_id1;
    memcpy(mapping_info->dmaci, &g_dmaci1, 6);
    entry_write(tbl_id, 0, &key, NULL, &data, true, MAPPING_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip4, 4);
    mapping_info->entry_valid = 1;
    mapping_info->nexthop_valid = 1;
    mapping_info->nexthop_type = NEXTHOP_TYPE_TUNNEL;
    mapping_info->nexthop_id = g_tunnel_id4;
    mapping_info->egress_bd_id = g_egress_bd_id1;
    memcpy(mapping_info->dmaci, &g_dmaci1, 6);
    entry_write(tbl_id, 0, &key, NULL, &data, true, MAPPING_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip5, 4);
    mapping_info->entry_valid = 1;
    mapping_info->nexthop_valid = 1;
    mapping_info->nexthop_type = NEXTHOP_TYPE_TUNNEL;
    mapping_info->nexthop_id = g_tunnel_id5;
    mapping_info->egress_bd_id = g_egress_bd_id1;
    memcpy(mapping_info->dmaci, &g_dmaci1, 6);
    entry_write(tbl_id, 0, &key, NULL, &data, true, MAPPING_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip9, 4);
    mapping_info->entry_valid = 1;
    mapping_info->nexthop_valid = 1;
    mapping_info->nexthop_type = NEXTHOP_TYPE_TUNNEL;
    mapping_info->nexthop_id = g_tunnel_id1;
    mapping_info->egress_bd_id = g_egress_bd_id1;
    memcpy(mapping_info->dmaci, &g_dmaci1, 6);
    entry_write(tbl_id, 0, &key, NULL, &data, true, MAPPING_TABLE_SIZE);
}

static void
mappings_with_ohash_init (void)
{
    mapping_swkey_t key;
    mapping_actiondata_t data;
    mapping_mapping_info_t *mapping_info = &data.action_u.mapping_mapping_info;
    uint32_t hash = 0;
    uint32_t hint = 0;
    uint32_t hint_nxt = 0;

    // no recirc
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip6, 4);
    hash = entry_write(P4TBL_ID_MAPPING, 0, &key, NULL, &data, true,
                       MAPPING_TABLE_SIZE);
    hint = g_mapping_ohash_id++;
    memset(&key, 0, sizeof(key));
    mapping_info->entry_valid = 1;
    mapping_info->hash1 = hash >> LOG2_U32(MAPPING_TABLE_SIZE);
    mapping_info->hint1 = hint;
    entry_write(P4TBL_ID_MAPPING, hash, &key, NULL, &data, true,
                MAPPING_TABLE_SIZE);
    // ohash table with result
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip6, 4);
    mapping_info->entry_valid = 1;
    mapping_info->nexthop_valid = 1;
    mapping_info->nexthop_type = NEXTHOP_TYPE_TUNNEL;
    mapping_info->nexthop_id = g_tunnel_id1;
    mapping_info->egress_bd_id = g_egress_bd_id1;
    memcpy(mapping_info->dmaci, &g_dmaci1, 6);
    entry_write(P4TBL_ID_MAPPING_OHASH, hint, &key, NULL, &data, true,
                MAPPING_OHASH_TABLE_SIZE);

    // 2 recircs
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip7, 4);
    hash = entry_write(P4TBL_ID_MAPPING, 0, &key, NULL, &data, true,
                       MAPPING_TABLE_SIZE);
    hint = g_mapping_ohash_id++;
    memset(&key, 0, sizeof(key));
    mapping_info->entry_valid = 1;
    mapping_info->hash1 = hash >> LOG2_U32(MAPPING_TABLE_SIZE);
    mapping_info->hint1 = hint;
    entry_write(P4TBL_ID_MAPPING, hash, &key, NULL, &data, true,
                MAPPING_TABLE_SIZE);
    // ohash table, force 1st recirc
    hint_nxt = g_mapping_ohash_id++;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    mapping_info->entry_valid = 1;
    mapping_info->hash1 = hash >> LOG2_U32(MAPPING_TABLE_SIZE);
    mapping_info->hint1 = hint_nxt;
    entry_write(P4TBL_ID_MAPPING_OHASH, hint, &key, NULL, &data, true,
                MAPPING_OHASH_TABLE_SIZE);
    // ohash table, force 2nd recirc
    hint = hint_nxt;
    hint_nxt = g_mapping_ohash_id++;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    mapping_info->entry_valid = 1;
    mapping_info->hash1 = hash >> LOG2_U32(MAPPING_TABLE_SIZE);
    mapping_info->hint1 = hint_nxt;
    entry_write(P4TBL_ID_MAPPING_OHASH, hint, &key, NULL, &data, true,
                MAPPING_OHASH_TABLE_SIZE);
    // ohash table with result
    hint = hint_nxt;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;
    key.p4e_i2e_mapping_lkp_id = g_vpc_id1;
    memcpy(key.p4e_i2e_mapping_lkp_addr, &g_dip7, 4);
    mapping_info->entry_valid = 1;
    mapping_info->nexthop_valid = 1;
    mapping_info->nexthop_type = NEXTHOP_TYPE_TUNNEL;
    mapping_info->nexthop_id = g_tunnel_id1;
    mapping_info->egress_bd_id = g_egress_bd_id1;
    memcpy(mapping_info->dmaci, &g_dmaci1, 6);
    entry_write(P4TBL_ID_MAPPING_OHASH, hint, &key, NULL, &data, true,
                MAPPING_OHASH_TABLE_SIZE);
}

static void
flows_init (void)
{
    ipv4_flow_swkey_t key;
    ipv4_flow_actiondata_t data;
    ipv4_flow_ipv4_flow_hash_t *flow_hash_info =
        &data.action_u.ipv4_flow_ipv4_flow_hash;
    uint32_t tbl_id = P4TBL_ID_IPV4_FLOW;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip1;
    key.key_metadata_ipv4_dst = g_dip1;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id1;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_dip1;
    key.key_metadata_ipv4_dst = g_sip1;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_dport1;
    key.key_metadata_dport = g_sport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id1;
    flow_hash_info->flow_role = TCP_FLOW_RESPONDER;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip1;
    key.key_metadata_ipv4_dst = g_dip4;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id4;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip1;
    key.key_metadata_ipv4_dst = g_dip5;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id4;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip7;
    key.key_metadata_ipv4_dst = g_dip9;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id4;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip11;
    key.key_metadata_ipv4_dst = g_dip1;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id1;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip12;
    key.key_metadata_ipv4_dst = g_dip1;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id1;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip1;
    key.key_metadata_ipv4_dst = g_dip1;
    key.key_metadata_proto = g_proto13;
    key.key_metadata_sport = g_sport13;
    key.key_metadata_dport = g_dport13;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id1;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_dip1;
    key.key_metadata_ipv4_dst = g_sip1;
    key.key_metadata_proto = g_proto13;
    key.key_metadata_sport = g_sport13;
    key.key_metadata_dport = g_dport13;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id1;
    flow_hash_info->flow_role = TCP_FLOW_RESPONDER;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, IPV4_FLOW_TABLE_SIZE);
}

static void
flows_with_ohash_init (void)
{
    ipv4_flow_swkey_t key;
    ipv4_flow_actiondata_t data;
    ipv4_flow_ipv4_flow_hash_t *flow_hash_info =
        &data.action_u.ipv4_flow_ipv4_flow_hash;
    uint32_t hash = 0;
    uint32_t hint = 0;
    uint32_t hint_nxt = 0;

    // no recirc
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip1;
    key.key_metadata_ipv4_dst = g_dip6;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    hash = entry_write(P4TBL_ID_IPV4_FLOW, 0, &key, NULL, &data, true,
                       IPV4_FLOW_TABLE_SIZE);
    hint = g_flow_ohash_id++;
    memset(&key, 0, sizeof(key));
    flow_hash_info->entry_valid = 1;
    flow_hash_info->hash1 = hash >> LOG2_U32(IPV4_FLOW_TABLE_SIZE);
    flow_hash_info->hint1 = hint;
    entry_write(P4TBL_ID_IPV4_FLOW, hash, &key, NULL, &data, true,
                IPV4_FLOW_TABLE_SIZE);
    // ohash table with result
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip1;
    key.key_metadata_ipv4_dst = g_dip6;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id1;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(P4TBL_ID_IPV4_FLOW_OHASH, hint, &key, NULL, &data, true,
                IPV4_FLOW_OHASH_TABLE_SIZE);

    // 2 recircs
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip7;
    key.key_metadata_ipv4_dst = g_dip7;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    hash = entry_write(P4TBL_ID_IPV4_FLOW, 0, &key, NULL, &data, true,
                       IPV4_FLOW_TABLE_SIZE);
    hint = g_flow_ohash_id++;
    memset(&key, 0, sizeof(key));
    flow_hash_info->entry_valid = 1;
    flow_hash_info->hash1 = hash >> LOG2_U32(IPV4_FLOW_TABLE_SIZE);
    flow_hash_info->hint1 = hint;
    entry_write(P4TBL_ID_IPV4_FLOW, hash, &key, NULL, &data, true,
                IPV4_FLOW_TABLE_SIZE);
    // ohash table, force 1st recirc
    hint_nxt = g_flow_ohash_id++;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    flow_hash_info->entry_valid = 1;
    flow_hash_info->hash1 = hash >> LOG2_U32(IPV4_FLOW_TABLE_SIZE);
    flow_hash_info->hint1 = hint_nxt;
    entry_write(P4TBL_ID_IPV4_FLOW_OHASH, hint, &key, NULL, &data, true,
                IPV4_FLOW_OHASH_TABLE_SIZE);
    // ohash table, force 2nd recirc
    hint = hint_nxt;
    hint_nxt = g_flow_ohash_id++;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    flow_hash_info->entry_valid = 1;
    flow_hash_info->hash1 = hash >> LOG2_U32(IPV4_FLOW_TABLE_SIZE);
    flow_hash_info->hint1 = hint_nxt;
    entry_write(P4TBL_ID_IPV4_FLOW_OHASH, hint, &key, NULL, &data, true,
                IPV4_FLOW_OHASH_TABLE_SIZE);
    // ohash table with result
    hint = hint_nxt;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_flow_lkp_id = g_bd_id1;
    key.key_metadata_ipv4_src = g_sip7;
    key.key_metadata_ipv4_dst = g_dip7;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = 1;
    flow_hash_info->session_index = g_session_id1;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(P4TBL_ID_IPV4_FLOW_OHASH, hint, &key, NULL, &data, true,
                IPV4_FLOW_OHASH_TABLE_SIZE);
}

static void
sessions_init (void)
{
    session_actiondata_t data;
    session_session_info_t *session_info = &data.action_u.session_session_info;
    uint16_t tbl_id = P4TBL_ID_SESSION;

    memset(&data, 0, sizeof(data));
    data.action_id = SESSION_SESSION_INFO_ID;
    session_info->tx_rewrite_flags =
        ((TX_REWRITE_SMAC_FROM_VRMAC << TX_REWRITE_SMAC_START) |
         (TX_REWRITE_DMAC_FROM_MAPPING << TX_REWRITE_DMAC_START) |
         (TX_REWRITE_ENCAP_VXLAN << TX_REWRITE_ENCAP_START));
    session_info->rx_rewrite_flags =
        ((RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START) |
         (RX_REWRITE_ENCAP_VLAN << RX_REWRITE_ENCAP_START));
    entry_write(tbl_id, g_session_id1, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    data.action_id = SESSION_SESSION_INFO_ID;
    session_info->tx_rewrite_flags =
        ((TX_REWRITE_SMAC_FROM_VRMAC << TX_REWRITE_SMAC_START) |
         (TX_REWRITE_DMAC_FROM_MAPPING << TX_REWRITE_DMAC_START) |
         (TX_REWRITE_ENCAP_VXLAN << TX_REWRITE_ENCAP_START));
    entry_write(tbl_id, g_session_id4, 0, 0, &data, false, 0);
}

static void
egress_bd_init (void)
{
    bd_actiondata_t data;
    bd_bd_info_t *bd_info = &data.action_u.bd_bd_info;
    uint16_t tbl_id = P4TBL_ID_BD;

    memset(&data, 0, sizeof(data));
    data.action_id = BD_BD_INFO_ID;
    bd_info->vni = g_vni1;
    memcpy(bd_info->vrmac, &g_vrmac1, 6);
    entry_write(tbl_id, g_egress_bd_id1, 0, 0, &data, false, 0);
}

static void
egress_properties_init (void)
{
    egress_bd_init();
}

static void
tunnels_init (void)
{
    tunnel_actiondata_t data;
    tunnel_tunnel_info_t *tunnel_info = &data.action_u.tunnel_tunnel_info;
    uint16_t tbl_id = P4TBL_ID_TUNNEL;

    memset(&data, 0, sizeof(data));
    memcpy(tunnel_info->dipo, &g_dipo1, 4);
    tunnel_info->ip_type = IPTYPE_IPV4;
    tunnel_info->nexthop_base = g_nexthop_id1;
    tunnel_info->num_nexthops = 1;
    entry_write(tbl_id, g_tunnel_id1, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    memcpy(tunnel_info->dipo, &g_dipo1, 4);
    tunnel_info->ip_type = IPTYPE_IPV4;
    tunnel_info->nexthop_base = g_nexthop_id4;
    tunnel_info->num_nexthops = 1;
    entry_write(tbl_id, g_tunnel_id4, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    memcpy(tunnel_info->dipo, &g_dipo1, 4);
    tunnel_info->ip_type = IPTYPE_IPV4;
    tunnel_info->nexthop_base = g_nexthop_id5;
    tunnel_info->num_nexthops = 1;
    entry_write(tbl_id, g_tunnel_id5, 0, 0, &data, false, 0);
}

static void
nexthops_init (void)
{
    nexthop_actiondata_t data;
    nexthop_nexthop_info_t *nexthop_info = &data.action_u.nexthop_nexthop_info;
    uint16_t tbl_id = P4TBL_ID_NEXTHOP;

    memset(&data, 0, sizeof(data));
    data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nexthop_info->port = TM_PORT_UPLINK_1;
    entry_write(tbl_id, g_nexthop_id_arm, 0, 0, &data, false, 0);
    memset(&data, 0, sizeof(data));

    memset(&data, 0, sizeof(data));
    data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nexthop_info->port = TM_PORT_UPLINK_1;
    memcpy(nexthop_info->dmaco, &g_dmaco1, 6);
    memcpy(nexthop_info->smaco, &g_device_mac, 6);
    entry_write(tbl_id, g_nexthop_id1, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nexthop_info->port = TM_PORT_UPLINK_0;
    nexthop_info->vlan = g_ctag1;
    entry_write(tbl_id, g_nexthop_id2, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nexthop_info->port = TM_PORT_UPLINK_1;
    memcpy(nexthop_info->dmaco, &g_dmaco1, 6);
    memcpy(nexthop_info->smaco, &g_device_mac, 6);
    nexthop_info->tunnel2_id = g_tunnel2_id4;
    nexthop_info->vlan = g_vni4;
    entry_write(tbl_id, g_nexthop_id4, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    nexthop_info->port = TM_PORT_UPLINK_1;
    memcpy(nexthop_info->dmaco, &g_dmaco1, 6);
    memcpy(nexthop_info->smaco, &g_device_mac, 6);
    nexthop_info->tunnel2_id = g_tunnel2_id5;
    nexthop_info->vlan = g_vni5;
    entry_write(tbl_id, g_nexthop_id5, 0, 0, &data, false, 0);
}

static void
tunnel2_init (void)
{
    tunnel2_actiondata_t data;
    tunnel2_tunnel2_info_t *tunnel2_info = &data.action_u.tunnel2_tunnel2_info;
    uint16_t tbl_id = P4TBL_ID_TUNNEL2;

    memset(&data, 0, sizeof(data));
    memcpy(tunnel2_info->dipo, &g_dipo4, 4);
    tunnel2_info->ip_type = IPTYPE_IPV4;
    tunnel2_info->encap_type = TX_REWRITE_ENCAP_MPLSoUDP;
    entry_write(tbl_id, g_tunnel2_id4, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    memcpy(tunnel2_info->dipo, &g_dipo4, 4);
    tunnel2_info->ip_type = IPTYPE_IPV4;
    tunnel2_info->encap_type = TX_REWRITE_ENCAP_VXLAN;
    entry_write(tbl_id, g_tunnel2_id5, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    memcpy(tunnel2_info->dipo, &g_dipo1, 4);
    tunnel2_info->ip_type = IPTYPE_IPV4;
    tunnel2_info->encap_type = TX_REWRITE_ENCAP_VXLAN;
    entry_write(tbl_id, g_tunnel2_id11, 0, 0, &data, false, 0);
}

static void
vnic_init (void)
{
    vnic_actiondata_t data;
    vnic_vnic_info_t *vnic_info = &data.action_u.vnic_vnic_info;
    uint16_t tbl_id = P4TBL_ID_VNIC;

    memset(&data, 0, sizeof(data));
    vnic_info->tx_mirror_session = (1 << g_mirror_id1) | (1 << g_mirror_id2);
    entry_write(tbl_id, g_vnic_id11, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    vnic_info->epoch = EPOCH + 1;
    entry_write(tbl_id, g_vnic_id12, 0, 0, &data, false, 0);
}

static void
mirror_init (void)
{
    mirror_actiondata_t data;
    mirror_erspan_t *erspan_info = &data.action_u.mirror_erspan;
    uint16_t tbl_id = P4TBL_ID_MIRROR;

    memset(&data, 0, sizeof(data));
    data.action_id = MIRROR_ERSPAN_ID;
    memcpy(erspan_info->dmac, &g_erspan_dmac1, 6);
    memcpy(erspan_info->smac, &g_erspan_smac1, 6);
    erspan_info->dip = g_erspan_dip1;
    erspan_info->sip = g_erspan_sip1;
    erspan_info->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    erspan_info->nexthop_id = g_nexthop_id1;
    erspan_info->span_id = g_mirror_id1;
    entry_write(tbl_id, g_mirror_id1, 0, 0, &data, false, 0);

    memset(&data, 0, sizeof(data));
    data.action_id = MIRROR_ERSPAN_ID;
    memcpy(erspan_info->dmac, &g_erspan_dmac1, 6);
    memcpy(erspan_info->smac, &g_erspan_smac1, 6);
    erspan_info->dip = g_erspan_dip1;
    erspan_info->sip = g_erspan_sip1;
    erspan_info->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    erspan_info->nexthop_id = g_nexthop_id1;
    erspan_info->span_id = g_mirror_id2;
    erspan_info->apply_tunnel2 = 1;
    erspan_info->tunnel2_id = g_tunnel2_id11;
    erspan_info->tunnel2_vni = g_vni1;
    entry_write(tbl_id, g_mirror_id2, 0, 0, &data, false, 0);
}

class apulu_test : public ::testing::Test {
protected:
    apulu_test() {}
    virtual ~apulu_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(apulu_test, test1)
{
    int ret = 0;
    char *default_config_dir = NULL;
    asic_cfg_t cfg;
    sdk::lib::catalog *catalog;

    p4pd_cfg_t p4pd_cfg = {.table_map_cfg_file =
                               "apulu/capri_p4_table_map.json",
                           .p4pd_pgm_name = "apulu_p4",
                           .p4pd_rxdma_pgm_name = "apulu_rxdma",
                           .p4pd_txdma_pgm_name = "apulu_txdma",
                           .cfg_path = std::getenv("CONFIG_PATH")};
    p4pd_cfg_t p4pd_rxdma_cfg = {.table_map_cfg_file =
                                     "apulu/capri_rxdma_table_map.json",
                                 .p4pd_pgm_name = "apulu_p4",
                                 .p4pd_rxdma_pgm_name = "apulu_rxdma",
                                 .p4pd_txdma_pgm_name = "apulu_txdma",
                                 .cfg_path = std::getenv("CONFIG_PATH")};
    p4pd_cfg_t p4pd_txdma_cfg = {.table_map_cfg_file =
                                     "apulu/capri_txdma_table_map.json",
                                 .p4pd_pgm_name = "apulu_p4",
                                 .p4pd_rxdma_pgm_name = "apulu_rxdma",
                                 .p4pd_txdma_pgm_name = "apulu_txdma",
                                 .cfg_path = std::getenv("CONFIG_PATH")};

    cfg.cfg_path = std::string(std::getenv("CONFIG_PATH"));
    std::string mpart_json = cfg.cfg_path + "/apulu/8g/hbm_mem.json";

    platform_type_t platform = platform_type_t::PLATFORM_TYPE_SIM;
    printf("Parsing sim catalog ...\n");
    catalog = sdk::lib::catalog::factory(cfg.cfg_path, "catalog.json");

    if (getenv("HAL_PLATFORM_RTL")) {
        platform = platform_type_t::PLATFORM_TYPE_RTL;
    } else if (getenv("HAL_PLATFORM_HW")) {
        platform = platform_type_t::PLATFORM_TYPE_HW;
        printf("Parsing hw catalog ...\n");
        catalog = sdk::lib::catalog::factory(cfg.cfg_path, "");
    }
    ASSERT_TRUE(catalog != NULL);
    cfg.catalog = catalog;
    cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());
    g_mempartition = cfg.mempartition;

    default_config_dir = std::getenv("HAL_PBC_INIT_CONFIG");
    if (default_config_dir) {
        cfg.default_config_dir = std::string(default_config_dir);
    } else {
        cfg.default_config_dir = "8x25_hbm";
    }

    cfg.platform = platform;
    cfg.admin_cos = 1;
    cfg.pgm_name = std::string("apulu");

    printf("Connecting to ASIC\n");
    sdk::lib::logger::init(sdk_trace_cb);
#ifdef HW
    ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW);
#else
    ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM);
#endif

    cfg.num_pgm_cfgs = 3;
    memset(cfg.pgm_cfg, 0, sizeof(cfg.pgm_cfg));
    cfg.pgm_cfg[0].path = std::string("p4_bin");
    cfg.pgm_cfg[1].path = std::string("rxdma_bin");
    cfg.pgm_cfg[2].path = std::string("txdma_bin");

    cfg.num_asm_cfgs = 3;
    memset(cfg.asm_cfg, 0, sizeof(cfg.asm_cfg));
    cfg.asm_cfg[0].name = std::string("apulu_p4");
    cfg.asm_cfg[0].path = std::string("p4_asm");
    cfg.asm_cfg[0].base_addr = std::string(JP4_PRGM);
    cfg.asm_cfg[0].sort_func = sort_mpu_programs;
    cfg.asm_cfg[1].name = std::string("apulu_rxdma");
    cfg.asm_cfg[1].path = std::string("rxdma_asm");
    cfg.asm_cfg[1].base_addr = std::string(JRXDMA_PRGM);
    cfg.asm_cfg[1].symbols_func = rxdma_symbols_init;
    cfg.asm_cfg[2].name = std::string("apulu_txdma");
    cfg.asm_cfg[2].path = std::string("txdma_asm");
    cfg.asm_cfg[2].base_addr = std::string(JTXDMA_PRGM);
    cfg.asm_cfg[2].symbols_func = txdma_symbols_init;

    cfg.completion_func = NULL;
    sdk::lib::device_profile_t device_profile = {0};
    device_profile.qos_profile = {9216, 8, 25, 27, 16, 2, {0, 24}};
    cfg.device_profile = &device_profile;

    printf("Doing asic init ...\n");
    ret = sdk::asic::pd::asicpd_init(&cfg);
    ASSERT_EQ(ret, SDK_RET_OK);
    printf("Doing p4pd init ...\n");
    ret = p4pd_init(&p4pd_cfg);
    ASSERT_EQ(ret, P4PD_SUCCESS);
    printf("Doing p4+ rxdma init ...\n");
    ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    ASSERT_EQ(ret, P4PD_SUCCESS);
    printf("Doing p4+ txdma init ...\n");
    ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    ASSERT_EQ(ret, P4PD_SUCCESS);
    printf("Doing p4+ mpu init ...\n");
    ret = sdk::asic::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg);
    ASSERT_EQ(ret, SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_program_p4plus_table_mpu_base_pc();
    SDK_ASSERT(ret == SDK_RET_OK);
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

#ifdef SIM
    config_done();
#endif

    p4plus_table_init(platform);
    init_service_lif();
    device_init();
    nacl_init();
    input_properties_init();
    local_mappings_init();
    local_mappings_with_ohash_init();
    vnic_init();
    bindings_init();
    mappings_init();
    mappings_with_ohash_init();
    flows_init();
    flows_with_ohash_init();
    mirror_init();
    sessions_init();
    egress_properties_init();
    tunnels_init();
    nexthops_init();
    tunnel2_init();
    checksum_init();

#ifdef SIM
    uint32_t port = 0;
    uint32_t cos = 0;
    std::vector<uint8_t> ipkt;
    std::vector<uint8_t> opkt;
    std::vector<uint8_t> epkt;
    std::vector<uint8_t> mpkt1;
    std::vector<uint8_t> mpkt2;
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
        std::cout << "[TCID=" << tcid << "] Tx:P4I-P4E" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_encap_pkt(opkt, epkt));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt2));
        memcpy(ipkt.data(), g_snd_pkt2, sizeof(g_snd_pkt2));
        epkt.resize(sizeof(g_rcv_pkt2));
        memcpy(epkt.data(), g_rcv_pkt2, sizeof(g_rcv_pkt2));
        std::cout << "[TCID=" << tcid << "] Rx:P4I-P4E" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_1);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
                EXPECT_TRUE(port == TM_PORT_UPLINK_0);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt3));
        memcpy(ipkt.data(), g_snd_pkt3, sizeof(g_snd_pkt3));
        epkt.resize(sizeof(g_rcv_pkt3));
        memcpy(epkt.data(), g_rcv_pkt3, sizeof(g_rcv_pkt3));
        std::cout << "[TCID=" << tcid << "] ARM:P4I-RxDMA-TxDMA-P4E" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt4));
        memcpy(ipkt.data(), g_snd_pkt4, sizeof(g_snd_pkt4));
        epkt.resize(sizeof(g_rcv_pkt4));
        memcpy(epkt.data(), g_rcv_pkt4, sizeof(g_rcv_pkt4));
        std::cout << "[TCID=" << tcid << "] Tx:P4I-P4E:DoubleEncap:MPLS"
            << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_double_encap_pkt(opkt, epkt,
                            TX_REWRITE_ENCAP_MPLSoUDP));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt5));
        memcpy(ipkt.data(), g_snd_pkt5, sizeof(g_snd_pkt5));
        epkt.resize(sizeof(g_rcv_pkt5));
        memcpy(epkt.data(), g_rcv_pkt5, sizeof(g_rcv_pkt5));
        std::cout << "[TCID=" << tcid << "] Tx:P4I-P4E:DoubleEncap:VxLAN"
            << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_double_encap_pkt(opkt, epkt,
                            TX_REWRITE_ENCAP_VXLAN));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt6));
        memcpy(ipkt.data(), g_snd_pkt6, sizeof(g_snd_pkt6));
        epkt.resize(sizeof(g_rcv_pkt6));
        memcpy(epkt.data(), g_rcv_pkt6, sizeof(g_rcv_pkt6));
        std::cout << "[TCID=" << tcid << "] ARM:TCP-FIN:P4I-RxDMA-TxDMA-P4E" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt7));
        memcpy(ipkt.data(), g_snd_pkt7, sizeof(g_snd_pkt7));
        epkt.resize(sizeof(g_rcv_pkt7));
        memcpy(epkt.data(), g_rcv_pkt7, sizeof(g_rcv_pkt7));
        std::cout << "[TCID=" << tcid << "] OHASH:No recirc" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_encap_pkt(opkt, epkt));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt8));
        memcpy(ipkt.data(), g_snd_pkt8, sizeof(g_snd_pkt8));
        epkt.resize(sizeof(g_rcv_pkt8));
        memcpy(epkt.data(), g_rcv_pkt8, sizeof(g_rcv_pkt8));
        std::cout << "[TCID=" << tcid << "] OHASH: recircs" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_encap_pkt(opkt, epkt));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt9));
        memcpy(ipkt.data(), g_snd_pkt9, sizeof(g_snd_pkt9));
        epkt.resize(sizeof(g_rcv_pkt9));
        memcpy(epkt.data(), g_rcv_pkt9, sizeof(g_rcv_pkt9));
        std::cout << "[TCID=" << tcid << "] OHASH: recircs too" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_encap_pkt(opkt, epkt));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt10));
        memcpy(ipkt.data(), g_snd_pkt10, sizeof(g_snd_pkt10));
        epkt.resize(sizeof(g_rcv_pkt10));
        memcpy(epkt.data(), g_rcv_pkt10, sizeof(g_rcv_pkt10));
        std::cout << "[TCID=" << tcid << "] TCP OPTS PARSING" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_encap_pkt(opkt, epkt));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt11));
        memcpy(ipkt.data(), g_snd_pkt11, sizeof(g_snd_pkt11));
        epkt.resize(sizeof(g_rcv_pkt11));
        memcpy(epkt.data(), g_rcv_pkt11, sizeof(g_rcv_pkt11));
        mpkt1.resize(sizeof(g_rcv_mpkt11_1));
        memcpy(mpkt1.data(), g_rcv_mpkt11_1, sizeof(g_rcv_mpkt11_1));
        std::cout << "[TCID=" << tcid << "] Tx SPAN" << std::endl;
        mpkt2.resize(sizeof(g_rcv_mpkt11_2));
        memcpy(mpkt2.data(), g_rcv_mpkt11_2, sizeof(g_rcv_mpkt11_2));
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == mpkt1);
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_encap_pkt(opkt, mpkt2));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_encap_pkt(opkt, epkt));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt12));
        memcpy(ipkt.data(), g_snd_pkt12, sizeof(g_snd_pkt12));
        epkt.resize(sizeof(g_rcv_pkt12));
        memcpy(epkt.data(), g_rcv_pkt12, sizeof(g_rcv_pkt12));
        std::cout << "[TCID=" << tcid << "] EPOCH:P4I-P4I-RxDMA-TxDMA-P4E" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt13));
        memcpy(ipkt.data(), g_snd_pkt13, sizeof(g_snd_pkt13));
        epkt.resize(sizeof(g_rcv_pkt13));
        memcpy(epkt.data(), g_rcv_pkt13, sizeof(g_rcv_pkt13));
        std::cout << "[TCID=" << tcid << "] Tx:GENEVE:P4I-P4E" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_DMA);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(is_equal_encap_pkt(opkt, epkt));
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt14));
        memcpy(ipkt.data(), g_snd_pkt14, sizeof(g_snd_pkt14));
        epkt.resize(sizeof(g_rcv_pkt14));
        memcpy(epkt.data(), g_rcv_pkt14, sizeof(g_rcv_pkt14));
        std::cout << "[TCID=" << tcid << "] Rx:GENEVE:P4I-P4E" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_1);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
                EXPECT_TRUE(port == TM_PORT_UPLINK_0);
            }
            testcase_end(tcid, i + 1);
        }
    }

    exit_simulation();
#endif

}

int
main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
