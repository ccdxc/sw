//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
// This file contains the basic artemis pipeline test case
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
#include "platform/utils/lif_manager_base.hpp"
#include "platform/capri/capri_qstate.hpp"
#include "nic/sdk/platform/capri/capri_hbm_rw.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/apollo/p4/include/artemis_defines.h"
#include "nic/apollo/p4/include/artemis_table_sizes.h"
#include "nic/apollo/core/trace.hpp"
#include "gen/platform/mem_regions.hpp"
#include "gen/p4gen/artemis/include/p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "nic/apollo/test/golden/artemis_pkts.h"
#include "nic/sdk/lib/device/device.hpp"

#define EPOCH 0x55

using namespace sdk::platform::utils;
using namespace sdk::platform::capri;

#define JRXDMA_PRGM     "rxdma_program"
#define JTXDMA_PRGM     "txdma_program"
#define JLIFQSTATE      "lif2qstate_map"
#define JPKTBUFFER      "rxdma_to_txdma_buf"
#define JPKTDESC        "rxdma_to_txdma_desc"
#define JP4_PRGM        "p4_program"

#define MEM_REGION_LIF_STATS_BASE       "lif_stats_base"
#define MEM_REGION_INVALID_FLOW_BASE    "invalid_flow_base"
#define MEM_REGION_FLOW_BASE            "flow"
#define MEM_REGION_FLOW_OHASH_BASE      "flow_ohash"
#define MEM_REGION_IPV4_FLOW_BASE       "ipv4_flow"
#define MEM_REGION_IPV4_FLOW_OHASH_BASE "ipv4_flow_ohash"
#define MEM_REGION_SESSION_STATS_BASE   "session_stats"
#define MEM_REGION_METER_STATS_BASE     "meter_stats"

#define MEM_REGION_FLOW_NAME            "flow"
#define MEM_REGION_FLOW_OHASH_NAME      "flow_ohash"
#define MEM_REGION_IPV4_FLOW_NAME       "ipv4_flow"
#define MEM_REGION_IPV4_FLOW_OHASH_NAME "ipv4_flow_ohash"

#define RXDMA_SYMBOLS_MAX           1
#define TXDMA_SYMBOLS_MAX           6

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

uint64_t g_device_mac = 0x00AABBCCDDEEULL;

uint16_t g_vnic_id1 = 0x1A;;
uint16_t g_vpc_id1 = 0xC1;
uint16_t g_xlate_idx1 = 0xA1D1;
uint32_t g_session_idx1 = 0x700DBA;
uint32_t g_nexthop_idx1 = 0xE101;
uint16_t g_meter_idx1 = 0x5;

uint64_t g_dmaco1 = 0x001234567890ULL;
uint32_t g_sipo1 = 0x64656667;
uint32_t g_dipo1 = 0x0C0C0101;
uint32_t g_tunnel_vni1 = 0xABCDEF;

uint64_t g_dmac1 = 0x000102030405ULL;
uint64_t g_smac1 = 0x00C1C2C3C4C5ULL;
uint32_t g_sip1 = 0x0B0B0101;
uint32_t g_dip1 = 0x0A0A0101;
uint8_t  g_proto1 = 0x6;
uint16_t g_sport1 = 0x1234;
uint16_t g_dport1 = 0x5678;

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

static void
init_service_lif ()
{
    LIFQState qstate = {0};

    qstate.lif_id = ARTEMIS_SERVICE_LIF;
    qstate.hbm_address = asicpd_get_mem_addr(JLIFQSTATE);
    qstate.params_in.type[0].entries = 1;
    qstate.params_in.type[0].size = 1;
    push_qstate_to_capri(&qstate, 0);

    lifqstate_t lif_qstate = {0};
    lif_qstate.ring0_base = asicpd_get_mem_addr(JPKTBUFFER);
    lif_qstate.ring1_base = asicpd_get_mem_addr(JPKTDESC);
    lif_qstate.ring_size = log2(asicpd_get_mem_size_kb(JPKTBUFFER) / 10);
    lif_qstate.total_rings = 1;
    write_qstate(qstate.hbm_address, (uint8_t *)&lif_qstate,
                 sizeof(lif_qstate));

    lifqstate_t txdma_qstate = {0};
    txdma_qstate.rxdma_cindex_addr =
        qstate.hbm_address + offsetof(lifqstate_t, sw_cindex);
    txdma_qstate.ring0_base = asicpd_get_mem_addr(JPKTBUFFER);
    txdma_qstate.ring1_base = asicpd_get_mem_addr(JPKTDESC);
    txdma_qstate.ring_size = log2(asicpd_get_mem_size_kb(JPKTBUFFER) / 10);
    txdma_qstate.total_rings = 1;
    write_qstate(qstate.hbm_address + sizeof(lifqstate_t),
                 (uint8_t *)&txdma_qstate, sizeof(txdma_qstate));
}

#if 0
static uint8_t *
memrev (uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}
#endif

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

static void
entry_write (uint32_t tbl_id, uint32_t index, void *key, void *mask, void *data,
             bool hash_table, uint32_t table_size)
{
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
            index = generate_hash_(hwkey, hwkey_len, 0);
            index &= table_size - 1;
        }
        p4pd_entry_write(tbl_id, index, hwkey, hwmask, data);
        delete[] hwkey;
        delete[] hwmask;
    } else {
        p4pd_entry_write(tbl_id, index, NULL, NULL, data);
    }
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

    symbols[i].name = MEM_REGION_INVALID_FLOW_BASE;
    symbols[i].val = 0xc0000000;
    i++;

    symbols[i].name = MEM_REGION_FLOW_BASE;
    symbols[i].val = g_mempartition->start_addr(MEM_REGION_FLOW_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    symbols[i].name = MEM_REGION_FLOW_OHASH_BASE;
    symbols[i].val = g_mempartition->start_addr(MEM_REGION_FLOW_OHASH_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    symbols[i].name = MEM_REGION_IPV4_FLOW_BASE;
    symbols[i].val = g_mempartition->start_addr(MEM_REGION_IPV4_FLOW_BASE);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    symbols[i].name = MEM_REGION_IPV4_FLOW_OHASH_BASE;
    symbols[i].val = g_mempartition->start_addr(MEM_REGION_IPV4_FLOW_OHASH_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    SDK_ASSERT(i <= TXDMA_SYMBOLS_MAX);

    return i;
}

static void
table_constants_init (void)
{
    uint64_t stats_base_addr = 0;

    stats_base_addr = asicpd_get_mem_addr(MEM_REGION_METER_STATS_BASE);
    stats_base_addr -= ((uint64_t)1 << 31);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_METER_STATS,
                                                 stats_base_addr);

    stats_base_addr = asicpd_get_mem_addr(MEM_REGION_SESSION_STATS_BASE);
    stats_base_addr -= ((uint64_t)1 << 31);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_SESSION,
                                                 stats_base_addr);

    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_EGRESS_VNIC_INFO,
                                                 g_device_mac);
}

static void
key_native_init (void)
{
    key_native_swkey_t key;
    key_native_swkey_mask_t mask;
    key_native_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_KEY_NATIVE;
    uint32_t index = 0;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    data.action_id = KEY_NATIVE_NATIVE_IPV4_PACKET_ID;
    key.ipv4_1_valid = 1;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    mask.ipv4_1_valid_mask = 1;
    mask.ipv6_1_valid_mask = 1;
    mask.ethernet_2_valid_mask = 1;
    mask.ipv4_2_valid_mask = 1;
    mask.ipv6_2_valid_mask = 1;
    mask.ethernet_3_valid_mask = 1;
    mask.ipv4_3_valid_mask = 1;
    mask.ipv6_3_valid_mask = 1;
    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
key_tunneled_init (void)
{
    key_tunneled_swkey_t key;
    key_tunneled_swkey_mask_t mask;
    key_tunneled_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_KEY_TUNNELED;
    uint32_t index = 0;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0xFF, sizeof(mask));
    memset(&data, 0, sizeof(data));
    data.action_id = KEY_TUNNELED_TUNNELED_IPV4_PACKET_ID;
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 1;
    key.ipv4_2_valid = 1;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 1;
    mask.ipv4_2_valid_mask = 1;
    mask.ipv6_2_valid_mask = 1;
    mask.ethernet_3_valid_mask = 1;
    mask.ipv4_3_valid_mask = 1;
    mask.ipv6_3_valid_mask = 1;
    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
key_tunneled2_init (void)
{
    key_tunneled2_swkey_t key;
    key_tunneled2_swkey_mask_t mask;
    key_tunneled2_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_KEY_TUNNELED2;
    uint32_t index = 0;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0xFF, sizeof(mask));
    memset(&data, 0, sizeof(data));
    data.action_id = KEY_TUNNELED2_TUNNELED2_IPV4_PACKET_ID;
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 1;
    key.ipv4_3_valid = 1;
    key.ipv6_3_valid = 0;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0;
    mask.ipv4_2_valid_mask = 0;
    mask.ipv6_2_valid_mask = 0;
    mask.ethernet_3_valid_mask = 1;
    mask.ipv4_3_valid_mask = 1;
    mask.ipv6_3_valid_mask = 1;
    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
vnic_init (void)
{
    vnic_mapping_swkey_t key;
    vnic_mapping_swkey_mask_t mask;
    vnic_mapping_actiondata_t data;
    vnic_mapping_vnic_mapping_info_t *mapping_info =
        &data.action_u.vnic_mapping_vnic_mapping_info;
    uint32_t tbl_id = P4TBL_ID_VNIC_MAPPING;
    uint32_t index = 0;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    data.action_id = VNIC_MAPPING_VNIC_MAPPING_INFO_ID;
    key.capri_intrinsic_lif = 0;
    memcpy(key.ethernet_1_srcAddr, &g_smac1, 6);
    mask.capri_intrinsic_lif_mask = -1;
    memset(mask.ethernet_1_srcAddr_mask, 0xFF,
           sizeof(mask.ethernet_1_srcAddr_mask));
    mapping_info->epoch = EPOCH;
    mapping_info->vnic_id = g_vnic_id1;
    mapping_info->vpc_id = g_vpc_id1;
    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
egress_vnic_init (void)
{
    egress_vnic_info_actiondata_t data;
    egress_vnic_info_egress_vnic_info_t *vnic_info =
        &data.action_u.egress_vnic_info_egress_vnic_info;
    uint32_t tbl_id = P4TBL_ID_EGRESS_VNIC_INFO;

    memset(&data, 0, sizeof(data));
    vnic_info->port = TM_PORT_UPLINK_0;
    entry_write(tbl_id, g_vnic_id1, NULL, NULL, &data, false, 0);
}

static void
mapping_init (void)
{
    local_ip_mapping_swkey_t key;
    local_ip_mapping_actiondata_t data;
    local_ip_mapping_local_ip_mapping_info_t *local_info =
        &data.action_u.local_ip_mapping_local_ip_mapping_info;
    uint32_t tbl_id = P4TBL_ID_LOCAL_IP_MAPPING;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.vnic_metadata_vpc_id = g_vpc_id1;
    memcpy(key.key_metadata_mapping_ip, &g_sip1, 4);
    local_info->pa_or_ca_xlate_idx = g_xlate_idx1;
    entry_write(tbl_id, 0, &key, NULL, &data, true,
                LOCAL_IP_MAPPING_HASH_TABLE_SIZE);
}

static void
ipv4_flow_init (void)
{
    ipv4_flow_swkey_t key;
    ipv4_flow_actiondata_t data;
    ipv4_flow_ipv4_flow_hash_t *flow_hash_info =
        &data.action_u.ipv4_flow_ipv4_flow_hash;
    uint32_t tbl_id = P4TBL_ID_IPV4_FLOW;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.vnic_metadata_vpc_id = g_vpc_id1;
    key.key_metadata_ipv4_src = g_sip1;
    key.key_metadata_ipv4_dst = g_dip1;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_sport1;
    key.key_metadata_dport = g_dport1;
    flow_hash_info->entry_valid = true;
    flow_hash_info->session_index = g_session_idx1;
    flow_hash_info->flow_role = TCP_FLOW_INITIATOR;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, FLOW_TABLE_SIZE);

    key.vnic_metadata_vpc_id = g_vpc_id1;
    key.key_metadata_ipv4_src = g_dip1;
    key.key_metadata_ipv4_dst = g_sip1;
    key.key_metadata_proto = g_proto1;
    key.key_metadata_sport = g_dport1;
    key.key_metadata_dport = g_sport1;
    flow_hash_info->entry_valid = true;
    flow_hash_info->flow_role = TCP_FLOW_RESPONDER;
    flow_hash_info->session_index = g_session_idx1;
    flow_hash_info->epoch = EPOCH;
    entry_write(tbl_id, 0, &key, NULL, &data, true, FLOW_TABLE_SIZE);
}

static void
session_init (void)
{
    session_actiondata_t data;
    session_session_info_t *session_info = &data.action_u.session_session_info;
    uint32_t tbl_id = P4TBL_ID_SESSION;

    memset(&data, 0, sizeof(data));
    session_info->nexthop_idx = g_nexthop_idx1;
    session_info->tx_rewrite_flags =
        (TX_REWRITE_ENCAP_VXLAN << TX_REWRITE_ENCAP_SHIFT);
    session_info->meter_idx = g_meter_idx1;
    entry_write(tbl_id, g_session_idx1, NULL, NULL, &data, false, 0);
}

static void
nat_init (void)
{
    nat_actiondata_t data;
    nat_nat_rewrite_t *nat_info = &data.action_u.nat_nat_rewrite;
    uint32_t tbl_id = P4TBL_ID_NAT;

    memset(&data, 0, sizeof(data));
    memcpy(nat_info->nat_ip, &g_sipo1, 4);
    entry_write(tbl_id, g_xlate_idx1, NULL, NULL, &data, false, 0);
}

static void
nexthop_init (void)
{
    nexthop_actiondata_t data;
    nexthop_nexthop_info_t *nexthop_info = &data.action_u.nexthop_nexthop_info;
    uint32_t tbl_id = P4TBL_ID_NEXTHOP;

    memset(&data, 0, sizeof(data));
    nexthop_info->port = TM_PORT_UPLINK_1;
    nexthop_info->vni = g_tunnel_vni1;
    nexthop_info->ip_type = IPTYPE_IPV4;
    memcpy(nexthop_info->dipo, &g_dipo1, 4);
    memcpy(nexthop_info->dmaco, &g_dmaco1, 6);
    entry_write(tbl_id, g_nexthop_idx1, NULL, NULL, &data, false, 0);
}

static void
inter_pipe_init (void)
{
    inter_pipe_ingress_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_INTER_PIPE_INGRESS;

    memset(&data, 0, sizeof(data));
    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_EGRESS_ID;
    entry_write(tbl_id, PIPE_EGRESS, NULL, NULL, &data, false,
                INTER_PIPE_TABLE_SIZE);

    memset(&data, 0, sizeof(data));
    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_CPS_ID;
    entry_write(tbl_id, PIPE_CPS, NULL, NULL, &data, false,
                INTER_PIPE_TABLE_SIZE);

    memset(&data, 0, sizeof(data));
    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_INGRESS_ID;
    entry_write(tbl_id, PIPE_INGRESS, NULL, NULL, &data, false,
                INTER_PIPE_TABLE_SIZE);
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

class artemis_test : public ::testing::Test {
protected:
    artemis_test() {}
    virtual ~artemis_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(artemis_test, test1)
{
    int ret = 0;
    char *default_config_dir = NULL;
    asic_cfg_t cfg;
    sdk::lib::catalog *catalog;

    p4pd_cfg_t p4pd_cfg = {.table_map_cfg_file =
                               "artemis/capri_p4_table_map.json",
                           .p4pd_pgm_name = "artemis_p4",
                           .p4pd_rxdma_pgm_name = "artemis_rxdma",
                           .p4pd_txdma_pgm_name = "artemis_txdma",
                           .cfg_path = std::getenv("CONFIG_PATH")};
    p4pd_cfg_t p4pd_rxdma_cfg = {.table_map_cfg_file =
                                     "artemis/capri_rxdma_table_map.json",
                                 .p4pd_pgm_name = "artemis_p4",
                                 .p4pd_rxdma_pgm_name = "artemis_rxdma",
                                 .p4pd_txdma_pgm_name = "artemis_txdma",
                                 .cfg_path = std::getenv("CONFIG_PATH")};
    p4pd_cfg_t p4pd_txdma_cfg = {.table_map_cfg_file =
                                     "artemis/capri_txdma_table_map.json",
                                 .p4pd_pgm_name = "artemis_p4",
                                 .p4pd_rxdma_pgm_name = "artemis_rxdma",
                                 .p4pd_txdma_pgm_name = "artemis_txdma",
                                 .cfg_path = std::getenv("CONFIG_PATH")};

    cfg.cfg_path = std::string(std::getenv("CONFIG_PATH"));
    std::string mpart_json = cfg.cfg_path + "/artemis/8g/hbm_mem.json";

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
    cfg.pgm_name = std::string("artemis");

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
    cfg.asm_cfg[0].name = std::string("artemis_p4");
    cfg.asm_cfg[0].path = std::string("p4_asm");
    cfg.asm_cfg[0].base_addr = std::string(JP4_PRGM);
    cfg.asm_cfg[0].sort_func = sort_mpu_programs;
    cfg.asm_cfg[1].name = std::string("artemis_rxdma");
    cfg.asm_cfg[1].path = std::string("rxdma_asm");
    cfg.asm_cfg[1].base_addr = std::string(JRXDMA_PRGM);
    cfg.asm_cfg[1].symbols_func = rxdma_symbols_init;
    cfg.asm_cfg[2].name = std::string("artemis_txdma");
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

    init_service_lif();
    table_constants_init();
    key_native_init();
    key_tunneled_init();
    key_tunneled2_init();
    vnic_init();
    egress_vnic_init();
    mapping_init();
    ipv4_flow_init();
    session_init();
    nat_init();
    nexthop_init();
    inter_pipe_init();
    checksum_init();

#ifdef SIM
    uint32_t port = 0;
    uint32_t cos = 0;
    std::vector<uint8_t> ipkt;
    std::vector<uint8_t> opkt;
    std::vector<uint8_t> epkt;
    std::vector<uint8_t> mpkt;
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
        std::cout << "[TCID=" << tcid << "] Testing P4I-P4E" << std::endl;
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

    exit_simulation();
#endif

}

int
main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
