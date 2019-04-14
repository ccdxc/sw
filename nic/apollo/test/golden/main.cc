//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the basic apollo pipeline test case
///
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
#include "nic/apollo/p4/include/defines.h"
#include "nic/apollo/p4/include/table_sizes.h"
#include "nic/apollo/p4/include/sacl_defines.h"
#include "nic/apollo/core/trace.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "gen/p4gen/apollo_txdma/include/apollo_txdma_p4pd.h"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"
#include "nic/utils/pack_bytes/pack_bytes.hpp"

#define EPOCH 0xb055
#define ROUTE_LPM_MEM_SIZE (64 + (16 * 64) + (16 * 16 * 64))
#define SACL_LPM_MEM_SIZE                                                     \
    (SACL_SPORT_TABLE_SIZE + SACL_IPV4_TABLE_SIZE +                          \
     SACL_PROTO_DPORT_TABLE_SIZE)

using namespace sdk::platform::utils;
using namespace sdk::platform::capri;

#define JRXDMA_PRGM "rxdma_program"
#define JTXDMA_PRGM "txdma_program"
#define JLIFQSTATE "lif2qstate_map"
#define JPKTBUFFER "rxdma_to_txdma_buf"
#define JSACLV4BASE "sacl_egress_v4"
#define JLPMV4BASE "lpm_v4"
#define JFLOWSTATSBASE "flow_stats"
#define JP4_PRGM        "p4_program"

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

    uint64_t arm_pindex : 16;
    uint64_t arm_cindex : 16;
    uint64_t sw_pindex : 16;
    uint16_t sw_cindex;
    uint64_t ring0_base : 64;
    uint64_t ring1_base : 64;
    uint64_t ring0_size : 16;
    uint64_t ring1_size : 16;

    uint8_t pad[(512 - 320) / 8];
} lifqstate_t;

typedef struct __attribute__((__packed__)) txdma_qstate_ {
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

    uint64_t sw_cindex : 16;
    uint64_t ring_size : 16;
    uint64_t ring_base : 64;
    uint64_t rxdma_cindex_addr : 64;

    uint8_t pad[(512 - 256) / 8];
} txdma_qstate_t;

typedef struct cache_line_s {
    uint8_t action_pc;
    uint8_t packed_entry[CACHE_LINE_SIZE-sizeof(action_pc)];
} cache_line_t;

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
    0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0xA2, 0x98, 0x64, 0x65, 0x66, 0x67, 0x0C, 0x0C,
    0x01, 0x01, 0x40, 0x8A, 0x19, 0xEB, 0x00, 0x68,
    0x00, 0x00, 0x00, 0x0C, 0x81, 0x00, 0x45, 0x00,
    0x00, 0x5C, 0x00, 0x01, 0x00, 0x00, 0x40, 0x06,
    0x63, 0x85, 0x0B, 0x0B, 0x01, 0x01, 0x0A, 0x0A,
    0x01, 0x01, 0x12, 0x34, 0x56, 0x78, 0x00, 0x00,
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

uint8_t g_snd_pkt2[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00, 0x12,
    0x34, 0x56, 0x78, 0x90, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0xA2, 0x98, 0x0C, 0x0C, 0x01, 0x01, 0x64, 0x65,
    0x66, 0x67, 0x0C, 0x0D, 0x19, 0xEB, 0x00, 0x68,
    0xB5, 0x7D, 0x12, 0x34, 0x51, 0x00, 0x45, 0x00,
    0x00, 0x5C, 0x00, 0x01, 0x00, 0x00, 0x40, 0x06,
    0x63, 0x85, 0x0A, 0x0A, 0x01, 0x01, 0x0B, 0x0B,
    0x01, 0x01, 0x56, 0x78, 0x12, 0x34, 0x00, 0x00,
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

uint8_t g_rcv_pkt2[] = {
    0x00, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x81, 0x00, 0x00, 0x64,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x85, 0x0A, 0x0A,
    0x01, 0x01, 0x0B, 0x0B, 0x01, 0x01, 0x56, 0x78,
    0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB4,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79
};

uint8_t g_snd_pkt3[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0xC1,
    0xC2, 0xC3, 0xC4, 0xC5, 0x81, 0x00, 0x00, 0x64,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x84, 0x0B, 0x0B,
    0x01, 0x01, 0x0A, 0x0A, 0x01, 0x02, 0x12, 0x34,
    0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB3,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_rcv_pkt3[] = {
    0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00, 0xAA,
    0xBB, 0xCC, 0xDD, 0xEE, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0xA2, 0x86, 0x64, 0x65, 0x66, 0x67, 0x0C, 0x0C,
    0x01, 0x01, 0xC9, 0xA8, 0x12, 0xB5, 0x00, 0x7A,
    0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xAB, 0xCD,
    0xEF, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x00, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0x08, 0x00,
    0x45, 0x00, 0x00, 0x5C, 0x00, 0x01, 0x00, 0x00,
    0x40, 0x06, 0x63, 0x84, 0x0B, 0x0B, 0x01, 0x01,
    0x0A, 0x0A, 0x01, 0x02, 0x12, 0x34, 0x56, 0x78,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x50, 0x02, 0x20, 0x00, 0xF2, 0xB3, 0x00, 0x00,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A,
    0x78, 0x79, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_snd_pkt4[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00, 0x12,
    0x34, 0x56, 0x78, 0x90, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0xA2, 0x86, 0x0C, 0x0C, 0x01, 0x01, 0x64, 0x65,
    0x66, 0x67, 0x2B, 0xA9, 0x12, 0xB5, 0x00, 0x7A,
    0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x23,
    0x45, 0x00, 0x00, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5,
    0x00, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0x08, 0x00,
    0x45, 0x00, 0x00, 0x5C, 0x00, 0x01, 0x00, 0x00,
    0x40, 0x06, 0x63, 0x84, 0x0A, 0x0A, 0x01, 0x02,
    0x0B, 0x0B, 0x01, 0x01, 0x56, 0x78, 0x12, 0x34,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x50, 0x02, 0x20, 0x00, 0xF2, 0xB3, 0x00, 0x00,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A,
    0x78, 0x79, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79,
};

uint8_t g_rcv_pkt4[] = {
    0x00, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x81, 0x00, 0x00, 0x64,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x06, 0x63, 0x84, 0x0A, 0x0A,
    0x01, 0x02, 0x0B, 0x0B, 0x01, 0x01, 0x56, 0x78,
    0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xF2, 0xB3,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
};

uint64_t g_layer1_smac = 0x00C1C2C3C4C5ULL;
uint64_t g_layer1_dmac = 0x000102030405ULL;
uint32_t g_layer1_sip = 0x0B0B0101;
uint32_t g_layer1_dip = 0x0A0A0101;
uint32_t g_layer1_dip2 = 0x0A0A0102;
uint8_t  g_layer1_proto = 0x6;
uint16_t g_layer1_sport = 0x1234;
uint16_t g_layer1_dport = 0x5678;

uint16_t g_ctag1_vid = 100;
uint16_t g_vcn_id1 = 0x2D1;
uint16_t g_vcn_id2 = 0x2D2;
uint16_t g_local_vnic_tag = 100;
uint32_t g_local_slot_id = 0x12345;
uint32_t g_flow_index = 0x31;

uint16_t g_nexthop_index = 0x155;
uint16_t g_nexthop_index2 = 0x333;
uint16_t g_tep_index = 100;
uint16_t g_tep_index2 = 200;
uint32_t g_gw_slot_id = 200;
uint32_t g_gw_slot_id2 = 0xABCDEF;
uint32_t g_gw_dip = 0x0C0C0101;
uint64_t g_gw_dmac = 0x001234567890;

uint16_t g_sacl_ip_class_id = 0x355;
uint16_t g_sacl_sport_class_id = 0x59;
uint16_t g_sacl_proto_dport_class_id = 0xAA;
uint16_t g_sacl_p1_class_id = 0x2BB;
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

    qstate.lif_id = APOLLO_SERVICE_LIF;
    qstate.hbm_address = get_mem_addr(JLIFQSTATE);
    qstate.params_in.type[0].entries = 1;
    qstate.params_in.type[0].size = 1; // 64B
    push_qstate_to_capri(&qstate, 0);

    lifqstate_t lif_qstate = {0};
    lif_qstate.ring0_base = get_mem_addr(JPKTBUFFER);
    lif_qstate.ring0_size = log2(get_mem_size_kb(JPKTBUFFER) / 10);
    lif_qstate.total_rings = 1;
    write_qstate(qstate.hbm_address, (uint8_t *)&lif_qstate,
                 sizeof(lif_qstate));

    txdma_qstate_t txdma_qstate = {0};
    txdma_qstate.rxdma_cindex_addr =
        qstate.hbm_address + offsetof(lifqstate_t, sw_cindex);
    txdma_qstate.ring_base = get_mem_addr(JPKTBUFFER);
    txdma_qstate.ring_size = log2(get_mem_size_kb(JPKTBUFFER) / 10);
    txdma_qstate.total_rings = 1;
    write_qstate(qstate.hbm_address + sizeof(lifqstate_t),
                 (uint8_t *)&txdma_qstate, sizeof(txdma_qstate));
}

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

static int
sdk_trace_cb (sdk_trace_level_e trace_level, const char *format, ...)
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

static void
key_native_init (void)
{
    key_native_swkey_t key;
    key_native_swkey_mask_t mask;
    key_native_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_KEY_NATIVE;
    uint32_t index;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    index = 0;
    data.action_id = KEY_NATIVE_NATIVE_IPV4_PACKET_ID;
    key.ipv4_1_valid = 1;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    mask.ipv4_1_valid_mask = 1;
    mask.ipv6_1_valid_mask = 1;
    mask.ethernet_2_valid_mask = 1;
    mask.ipv4_2_valid_mask = 1;
    mask.ipv6_2_valid_mask = 1;

    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
key_tunneled_init (void)
{
    key_tunneled_swkey_t key;
    key_tunneled_swkey_mask_t mask;
    key_tunneled_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_KEY_TUNNELED;
    uint32_t index;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0xFF, sizeof(mask));
    memset(&data, 0, sizeof(data));

    index = 0;
    data.action_id = KEY_TUNNELED_TUNNELED_IPV4_PACKET_ID;
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 1;
    key.ipv6_2_valid = 0;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0;
    mask.ipv4_2_valid_mask = 1;
    mask.ipv6_2_valid_mask = 1;

    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
vnic_tx_init ()
{
    local_vnic_by_vlan_tx_actiondata_t data;
    local_vnic_by_vlan_tx_local_vnic_info_tx_t *local_vnic_info =
        &data.action_u.local_vnic_by_vlan_tx_local_vnic_info_tx;
    uint64_t sacl_hbm_addr;
    uint64_t lpm_hbm_addr;
    uint32_t tbl_id = P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX;
    uint32_t index;

    memset(&data, 0, sizeof(data));
    index = g_ctag1_vid;
    data.action_id = LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    local_vnic_info->vcn_id = g_vcn_id1;
    local_vnic_info->epoch1_valid = true;
    local_vnic_info->epoch1 = EPOCH;
    local_vnic_info->local_vnic_tag = g_local_vnic_tag;
    local_vnic_info->skip_src_dst_check1 = true;
    memcpy(local_vnic_info->overlay_mac1, &g_layer1_smac, 6);
    sacl_hbm_addr = get_mem_addr(JSACLV4BASE);
    memcpy(local_vnic_info->sacl_v4addr1, &sacl_hbm_addr,
           sizeof(local_vnic_info->sacl_v4addr1));
    memcpy(local_vnic_info->sacl_v4addr2, &sacl_hbm_addr,
           sizeof(local_vnic_info->sacl_v4addr2));
    lpm_hbm_addr = get_mem_addr(JLPMV4BASE);
    memcpy(local_vnic_info->lpm_v4addr1, &lpm_hbm_addr,
           sizeof(local_vnic_info->lpm_v4addr1));
    memcpy(local_vnic_info->lpm_v4addr2, &lpm_hbm_addr,
           sizeof(local_vnic_info->lpm_v4addr2));

    entry_write(tbl_id, index, NULL, NULL, &data, false, 0);
}

static void
vnic_rx_init ()
{
    local_vnic_by_slot_rx_swkey_t key;
    local_vnic_by_slot_rx_actiondata_t data;
    local_vnic_by_slot_rx_local_vnic_info_rx_t *local_vnic_info =
        &data.action_u.local_vnic_by_slot_rx_local_vnic_info_rx;
    uint64_t sacl_hbm_addr;
    uint32_t tbl_id = P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.mpls_dst_label = g_local_slot_id;
    data.action_id = LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID;
    local_vnic_info->vcn_id = g_vcn_id1;
    local_vnic_info->epoch1_valid = true;
    local_vnic_info->epoch1 = EPOCH;
    local_vnic_info->local_vnic_tag = g_local_vnic_tag;
    local_vnic_info->skip_src_dst_check1 = true;
    sacl_hbm_addr = get_mem_addr(JSACLV4BASE);
    memcpy(local_vnic_info->sacl_v4addr1, &sacl_hbm_addr,
           sizeof(local_vnic_info->sacl_v4addr1));
    memcpy(local_vnic_info->sacl_v4addr2, &sacl_hbm_addr,
           sizeof(local_vnic_info->sacl_v4addr2));

    entry_write(tbl_id, 0, &key, NULL, &data, true,
                LOCAL_VNIC_BY_SLOT_TABLE_SIZE);

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.vxlan_1_vni = g_local_slot_id;
    data.action_id = LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID;
    local_vnic_info->vcn_id = g_vcn_id1;
    local_vnic_info->epoch1_valid = true;
    local_vnic_info->epoch1 = EPOCH;
    local_vnic_info->local_vnic_tag = g_local_vnic_tag;
    local_vnic_info->skip_src_dst_check1 = true;
    sacl_hbm_addr = get_mem_addr(JSACLV4BASE);
    memcpy(local_vnic_info->sacl_v4addr1, &sacl_hbm_addr,
           sizeof(local_vnic_info->sacl_v4addr1));
    memcpy(local_vnic_info->sacl_v4addr2, &sacl_hbm_addr,
           sizeof(local_vnic_info->sacl_v4addr2));

    entry_write(tbl_id, 0, &key, NULL, &data, true,
                LOCAL_VNIC_BY_SLOT_TABLE_SIZE);
}

static void
egress_vnic_info_init ()
{
    egress_local_vnic_info_actiondata_t data;
    egress_local_vnic_info_egress_local_vnic_info_t *local_vnic_info =
        &data.action_u.egress_local_vnic_info_egress_local_vnic_info;
    uint32_t tbl_id = P4TBL_ID_EGRESS_LOCAL_VNIC_INFO;
    uint32_t index;

    memset(&data, 0, sizeof(data));
    index = g_local_vnic_tag;
    data.action_id = EGRESS_LOCAL_VNIC_INFO_EGRESS_LOCAL_VNIC_INFO_ID;
    local_vnic_info->overlay_vlan_id = g_ctag1_vid;
    memcpy(local_vnic_info->vr_mac, &g_layer1_dmac, 6);
    memcpy(local_vnic_info->overlay_mac, &g_layer1_smac, 6);

    entry_write(tbl_id, index, NULL, NULL, &data, false, 0);
}

static void
vnic_init (void)
{
    vnic_tx_init();
    vnic_rx_init();
    egress_vnic_info_init();
}

static void
mappings_init (void)
{
    local_ip_mapping_swkey_t key;
    local_ip_mapping_actiondata_t data;
    local_ip_mapping_local_ip_mapping_info_t *mapping_info =
        &data.action_u.local_ip_mapping_local_ip_mapping_info;
    uint32_t tbl_id = P4TBL_ID_LOCAL_IP_MAPPING;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.vnic_metadata_local_vnic_tag = g_local_vnic_tag;
    memcpy(key.control_metadata_mapping_lkp_addr, &g_layer1_sip, 4);
    data.action_id = LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID;
    mapping_info->entry_valid = true;

    entry_write(tbl_id, 0, &key, NULL, &data, true, VNIC_IP_MAPPING_TABLE_SIZE);
}

static void
flow_tx_hash_init ()
{
    flow_swkey_t key;
    flow_actiondata_t data;
    flow_flow_hash_t *flow_hash_info = &data.action_u.flow_flow_hash;
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
    data.action_id = FLOW_FLOW_HASH_ID;
    flow_hash_info->entry_valid = true;
    flow_hash_info->flow_index = g_flow_index;

    entry_write(tbl_id, 0, &key, NULL, &data, true, FLOW_TABLE_SIZE);
}

static void
flow_tx_info_init ()
{
    flow_info_actiondata_t data;
    flow_info_flow_info_t *flow_info = &data.action_u.flow_info_flow_info;
    uint32_t tbl_id = P4TBL_ID_FLOW_INFO;
    uint64_t flow_stats_addr;

    memset(&data, 0, sizeof(data));
    data.action_id = FLOW_INFO_FLOW_INFO_ID;
    flow_stats_addr = get_mem_addr(JFLOWSTATSBASE) + g_flow_index * 64;
    flow_stats_addr -= ((uint64_t)1 << 31);
    memcpy(flow_info->flow_stats_addr, &flow_stats_addr,
           sizeof(flow_info->flow_stats_addr));

    entry_write(tbl_id, g_flow_index, NULL, NULL, &data, false, 0);
}

static void
flow_rx_hash_init ()
{
#ifdef FLOW_RX_HIT
    flow_swkey_t key;
    flow_actiondata_t data;
    flow_flow_hash_t *flow_hash_info = &data.action_u.flow_flow_hash;
    uint32_t tbl_id = P4TBL_ID_FLOW;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.key_metadata_ktype = KEY_TYPE_IPV4;
    key.vnic_metadata_local_vnic_tag = g_local_vnic_tag;
    memcpy(key.key_metadata_src, &g_layer1_dip, 4);
    memcpy(key.key_metadata_dst, &g_layer1_sip, 4);
    key.key_metadata_proto = g_layer1_proto;
    key.key_metadata_sport = g_layer1_dport;
    key.key_metadata_dport = g_layer1_sport;
    data.action_id = FLOW_FLOW_HASH_ID;
    flow_hash_info->entry_valid = true;
    flow_hash_info->flow_index = g_flow_index + 1;

    entry_write(tbl_id, 0, &key, NULL, &data, true, FLOW_TABLE_SIZE);
#endif
}

static void
flow_rx_info_init ()
{
#ifdef FLOW_RX_HIT
    flow_info_actiondata_t data;
    flow_info_flow_info_t *flow_info = &data.action_u.flow_info_flow_info;
    uint32_t tbl_id = P4TBL_ID_FLOW_INFO;
    uint64_t flow_stats_addr;

    memset(&data, 0, sizeof(data));
    data.action_id = FLOW_INFO_FLOW_INFO_ID;
    flow_stats_addr = get_mem_addr(JFLOWSTATSBASE) + (g_flow_index + 1) * 64;
    flow_stats_addr -= ((uint64_t)1 << 31);
    memcpy(flow_info->flow_stats_addr, &flow_stats_addr,
           sizeof(flow_info->flow_stats_addr));

    entry_write(tbl_id, g_flow_index + 1, NULL, NULL, &data, false, 0);
#endif
}

static void
flow_init (void)
{
    flow_tx_hash_init();
    flow_tx_info_init();
    flow_rx_hash_init();
    flow_rx_info_init();
}

static void
nexthop_tx_init (uint16_t nexthop_index, uint16_t tep_index, uint32_t slot_id)
{
    nexthop_tx_actiondata_t data;
    nexthop_tx_nexthop_info_t *nexthop_info =
        &data.action_u.nexthop_tx_nexthop_info;
    uint32_t tbl_id = P4TBL_ID_NEXTHOP_TX;
    uint32_t index;

    memset(&data, 0, sizeof(data));

    index = nexthop_index;
    data.action_id = NEXTHOP_TX_NEXTHOP_INFO_ID;
    nexthop_info->tep_index = tep_index;
    nexthop_info->encap_type = GW_ENCAP;
    nexthop_info->dst_slot_id = slot_id;

    entry_write(tbl_id, index, NULL, NULL, &data, false, 0);
}

static void
tep_tx_init (uint16_t tep_index, uint8_t tunnel_type)
{
    tep_tx_actiondata_t data;
    tep_tx_gre_tep_tx_t *tep_info = &data.action_u.tep_tx_gre_tep_tx;
    uint32_t tbl_id = P4TBL_ID_TEP_TX;
    uint32_t index;

    memset(&data, 0, sizeof(data));
    index = tep_index;
    data.action_id = tunnel_type;
    tep_info->dipo = g_gw_dip;
    memcpy(tep_info->dmac, &g_gw_dmac, 6);
    entry_write(tbl_id, index, NULL, NULL, &data, false, 0);
}

static void
rewrite_init (void)
{
    uint64_t mytep_ip = 0x64656667; // 100.101.102.103
    uint64_t mytep_mac = 0x00AABBCCDDEE;

    // program the table constants
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                                                 mytep_ip);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_NEXTHOP_TX, mytep_ip);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_TEP_TX, mytep_mac);

    nexthop_tx_init(g_nexthop_index, g_tep_index, g_gw_slot_id);
    tep_tx_init(g_tep_index, TEP_TX_MPLS_UDP_TEP_TX_ID);

    nexthop_tx_init(g_nexthop_index2, g_tep_index2, g_gw_slot_id2);
    tep_tx_init(g_tep_index2, TEP_TX_VXLAN_TEP_TX_ID);
}

static void
trie_mem_init (void)
{
    uint64_t data[8];

    memset(data, 0xFF, sizeof(data));
    uint64_t lpm_hbm_addr = get_mem_addr(JLPMV4BASE);
    for (uint32_t i = 0; i < ROUTE_LPM_MEM_SIZE; i += sizeof(data)) {
        sdk::asic::asic_mem_write(lpm_hbm_addr + i, (uint8_t *)data,
                                  sizeof(data));
    }

    uint64_t sacl_hbm_addr = get_mem_addr(JSACLV4BASE);
    for (uint32_t i = 0; i < SACL_LPM_MEM_SIZE; i += sizeof(data)) {
        sdk::asic::asic_mem_write(sacl_hbm_addr + i, (uint8_t *)data,
                                  sizeof(data));
    }
}

static void
route_init (void)
{
    uint32_t len;
    uint64_t lpm_base_addr = get_mem_addr(JLPMV4BASE);
    route_actiondata_t sw_entry;
    cache_line_t cache_line;

    memset(&sw_entry, 0xFF, sizeof(sw_entry));

    /* Program stage 0*/
    sw_entry.action_id = ROUTE_SEARCH_IPV4_ID;
    p4pd_apollo_txdma_raw_table_hwentry_query(P4_APOLLO_TXDMA_TBL_ID_ROUTE,
                                              ROUTE_SEARCH_IPV4_ID,
                                              &len);
    p4pd_apollo_txdma_entry_pack(P4_APOLLO_TXDMA_TBL_ID_ROUTE,
                                 ROUTE_SEARCH_IPV4_ID,
                                 &sw_entry, cache_line.packed_entry);
    cache_line.action_pc = sdk::asic::pd::asicpd_get_action_pc(
                                P4_APOLLO_TXDMA_TBL_ID_ROUTE,
                                ROUTE_SEARCH_IPV4_ID);
    sdk::asic::asic_mem_write(lpm_base_addr + 0, (uint8_t *)&cache_line,
                              sizeof(cache_line));

    /* Program stage 1*/
    sdk::asic::asic_mem_write(lpm_base_addr + 64, (uint8_t *)&cache_line,
                              sizeof(cache_line));

    /* Program stage 2*/
    sw_entry.action_u.route_search_ipv4_retrieve.key0 =
            (g_layer1_dip2 & 0xFFFFFFFF);
    sw_entry.action_u.route_search_ipv4_retrieve.data_ = g_nexthop_index;
    sw_entry.action_u.route_search_ipv4_retrieve.data0 = (0x8000 | g_vcn_id2);

    p4pd_apollo_txdma_raw_table_hwentry_query(P4_APOLLO_TXDMA_TBL_ID_ROUTE,
                                              ROUTE_SEARCH_IPV4_RETRIEVE_ID,
                                              &len);
    p4pd_apollo_txdma_entry_pack(P4_APOLLO_TXDMA_TBL_ID_ROUTE,
                                 ROUTE_SEARCH_IPV4_RETRIEVE_ID,
                                 &sw_entry, cache_line.packed_entry);
    cache_line.action_pc = sdk::asic::pd::asicpd_get_action_pc(
                                P4_APOLLO_TXDMA_TBL_ID_ROUTE,
                                ROUTE_SEARCH_IPV4_RETRIEVE_ID);
    sdk::asic::asic_mem_write(lpm_base_addr + 64 + (16 * 64), (uint8_t *)&cache_line,
                              sizeof(cache_line));
}

static void
sacl_init (void)
{
    uint64_t data;
    uint8_t c_data[64];
    uint16_t start_bit;
    uint64_t sacl_base_addr = get_mem_addr(JSACLV4BASE);

    uint64_t sacl_ip_addr =
        sacl_base_addr + SACL_IPV4_TABLE_OFFSET + (64 + (16 * 64));
    uint64_t sacl_sport_addr =
        sacl_base_addr + SACL_SPORT_TABLE_OFFSET + (64);
    uint64_t sacl_proto_dport_addr =
        sacl_base_addr + SACL_PROTO_DPORT_TABLE_OFFSET + (64 + (16 * 64));
    uint64_t sacl_p1_addr =
        sacl_base_addr + SACL_P1_TABLE_OFFSET +
        (((g_sacl_ip_class_id | (g_sacl_sport_class_id << 10)) / 51) * 64);
    uint64_t sacl_p2_addr =
        sacl_base_addr + SACL_P2_TABLE_OFFSET + (g_sacl_p1_class_id << 6);

    sacl_ip_data_actiondata_t sacl_ip;
    memset(&sacl_ip, 0xFF, sizeof(sacl_ip));
    sacl_ip.action_u.sacl_ip_data_match_ipv4_retrieve.key0 =
        (g_layer1_dip & 0xFFFF0000);
    sacl_ip.action_u.sacl_ip_data_match_ipv4_retrieve.data0 =
        g_sacl_ip_class_id;
    p4pd_apollo_rxdma_entry_pack(P4_APOLLO_RXDMA_TBL_ID_SACL_IP_DATA,
                                 SACL_IP_DATA_MATCH_IPV4_RETRIEVE_ID,
                                 &sacl_ip, c_data);
    sdk::asic::asic_mem_write(sacl_ip_addr, (uint8_t *)&c_data, sizeof(c_data));

    data = -1;
    data &= ~((uint64_t)0xFFFF);
    data |= ((uint64_t)htons(g_sacl_sport_class_id));
    sdk::asic::asic_mem_write(sacl_sport_addr, (uint8_t *)&data, sizeof(data));

    data = -1;
    data &= ~(((uint64_t)0xFFFFFFFFFFULL) << 16);
    data |= ((uint64_t)htons(g_sacl_proto_dport_class_id)) << 40;
    sdk::asic::asic_mem_write(sacl_proto_dport_addr, (uint8_t *)&data,
                              sizeof(data));

    data = -1;
    data &= ~(((uint64_t)0xFFFFFFFFFFULL) << 16);
    data |= ((uint64_t)htons(g_sacl_proto_dport_class_id)) << 40;
    sdk::asic::asic_mem_write(sacl_proto_dport_addr, (uint8_t *)&data,
                              sizeof(data));

    start_bit =
        (((g_sacl_ip_class_id | (g_sacl_sport_class_id << 10)) % 51) * 10);
    sdk::asic::asic_mem_read(sacl_p1_addr, c_data, 64);
    memrev(c_data, 64);
    hal::utils::pack_bytes_pack(c_data, start_bit, 10, g_sacl_p1_class_id);
    memrev(c_data, 64);
    sdk::asic::asic_mem_write(sacl_p1_addr, c_data, 64);

    start_bit = (g_sacl_proto_dport_class_id << 1);
    sdk::asic::asic_mem_read(sacl_p2_addr, c_data, 64);
    memrev(c_data, 64);
    hal::utils::pack_bytes_pack(c_data, start_bit, 2, 1);
    memrev(c_data, 64);
    sdk::asic::asic_mem_write(sacl_p2_addr, c_data, 64);
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
    key.predicate_header_redirect_to_arm = 1;
    mask.predicate_header_redirect_to_arm_mask = 1;
    data.action_id = NACL_NACL_REDIRECT_ID;
    data.action_u.nacl_nacl_redirect.app_id = P4PLUS_APPTYPE_CPU;
    data.action_u.nacl_nacl_redirect.oport = TM_PORT_DMA;
    data.action_u.nacl_nacl_redirect.lif = 0x64;
    data.action_u.nacl_nacl_redirect.qtype = 0;
    data.action_u.nacl_nacl_redirect.qid = 0;
    data.action_u.nacl_nacl_redirect.vlan_strip = 0;

    entry_write(tbl_id, index, &key, &mask, &data, false, 0);
}

static void
vxlan_mappings_init (void)
{
    remote_vnic_mapping_tx_swkey_t key;
    remote_vnic_mapping_tx_actiondata_t data;
    remote_vnic_mapping_tx_remote_vnic_mapping_tx_info_t *mapping_info =
        &data.action_u.remote_vnic_mapping_tx_remote_vnic_mapping_tx_info;
    uint32_t tbl_id = P4TBL_ID_REMOTE_VNIC_MAPPING_TX;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    memcpy(key.p4e_apollo_i2e_dst, &g_layer1_dip2, 4);
    key.txdma_to_p4e_header_vcn_id = g_vcn_id2;
    data.action_id = REMOTE_VNIC_MAPPING_TX_REMOTE_VNIC_MAPPING_TX_INFO_ID;
    mapping_info->entry_valid = true;
    mapping_info->nexthop_index = g_nexthop_index2;
    mapping_info->dst_slot_id_valid = true;
    mapping_info->dst_slot_id = g_gw_slot_id2;

    entry_write(tbl_id, 0, &key, NULL, &data, true,
                REMOTE_VNIC_MAPPING_TX_TABLE_SIZE);
}

static void
vxlan_init (void)
{
    vxlan_mappings_init();
}

static void
inter_pipe_init (void)
{
    ingress_to_rxdma_actiondata_t data;
    uint32_t tbl_id = P4TBL_ID_INGRESS_TO_RXDMA;

    memset(&data, 0, sizeof(data));
    data.action_id = INGRESS_TO_RXDMA_INGRESS_TO_RXDMA_ID;
    entry_write(tbl_id, 0, NULL, NULL, &data, false, APP_TABLE_SIZE);

    memset(&data, 0, sizeof(data));
    data.action_id = INGRESS_TO_RXDMA_CLASSIC_NIC_APP_ID;
    entry_write(tbl_id, P4PLUS_APPTYPE_CLASSIC_NIC, NULL, NULL, &data,
                false, APP_TABLE_SIZE);

    memset(&data, 0, sizeof(data));
    data.action_id = INGRESS_TO_RXDMA_REDIRECT_TO_ARM_ID;
    entry_write(tbl_id, P4PLUS_APPTYPE_CPU, NULL, NULL, &data,
                false, APP_TABLE_SIZE);
}

#define MEM_REGION_LIF_STATS_BASE   "lif_stats_base"
#define RXDMA_SYMBOLS_MAX           1
#define TXDMA_SYMBOLS_MAX           1
uint32_t
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

uint32_t
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
    symbols[i].val = g_mempartition->
                     start_addr(MEM_REGION_LIF_STATS_NAME);
    i++;
    SDK_ASSERT(i <= TXDMA_SYMBOLS_MAX);

    return i;
}


class apollo_test : public ::testing::Test {
protected:
    apollo_test() {}
    virtual ~apollo_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(apollo_test, test1)
{
    int ret = 0;
    char *default_config_dir = NULL;
    asic_cfg_t cfg;
    sdk::lib::catalog *catalog;

    p4pd_cfg_t p4pd_cfg = {.table_map_cfg_file =
                               "apollo/capri_p4_table_map.json",
                           .p4pd_pgm_name = "apollo_p4",
                           .p4pd_rxdma_pgm_name = "apollo_rxdma",
                           .p4pd_txdma_pgm_name = "apollo_txdma",
                           .cfg_path = std::getenv("HAL_CONFIG_PATH")};
    p4pd_cfg_t p4pd_rxdma_cfg = {.table_map_cfg_file =
                                     "apollo/capri_rxdma_table_map.json",
                                 .p4pd_pgm_name = "apollo_p4",
                                 .p4pd_rxdma_pgm_name = "apollo_rxdma",
                                 .p4pd_txdma_pgm_name = "apollo_txdma",
                                 .cfg_path = std::getenv("HAL_CONFIG_PATH")};
    p4pd_cfg_t p4pd_txdma_cfg = {.table_map_cfg_file =
                                     "apollo/capri_txdma_table_map.json",
                                 .p4pd_pgm_name = "apollo_p4",
                                 .p4pd_rxdma_pgm_name = "apollo_rxdma",
                                 .p4pd_txdma_pgm_name = "apollo_txdma",
                                 .cfg_path = std::getenv("HAL_CONFIG_PATH")};

    cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    std::string mpart_json = cfg.cfg_path + "/apollo/hbm_mem.json";

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
    cfg.pgm_name = std::string("apollo");

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
    cfg.asm_cfg[0].name = std::string("apollo_p4");
    cfg.asm_cfg[0].path = std::string("p4_asm");
    cfg.asm_cfg[0].base_addr = std::string(JP4_PRGM);
    cfg.asm_cfg[0].sort_func = sort_mpu_programs;
    cfg.asm_cfg[1].name = std::string("apollo_rxdma");
    cfg.asm_cfg[1].path = std::string("rxdma_asm");
    cfg.asm_cfg[1].base_addr = std::string(JRXDMA_PRGM);
    cfg.asm_cfg[1].symbols_func = rxdma_symbols_init;
    cfg.asm_cfg[2].name = std::string("apollo_txdma");
    cfg.asm_cfg[2].path = std::string("txdma_asm");
    cfg.asm_cfg[2].base_addr = std::string(JTXDMA_PRGM);
    cfg.asm_cfg[2].symbols_func = txdma_symbols_init;

    cfg.completion_func = NULL;

    printf("Doing asic init ...\n");
    ret = sdk::asic::asic_init(&cfg);
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

    trie_mem_init();

#ifdef SIM
    config_done();
#endif

    init_service_lif();
    key_native_init();
    key_tunneled_init();
    vnic_init();
    mappings_init();
    flow_init();
    rewrite_init();
    route_init();
    sacl_init();
    nacl_init();

    vxlan_init();
    inter_pipe_init();

#ifdef SIM
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
        std::cout << "Testing Host to Switch (MPLSoUDP)" << std::endl;
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
        ipkt.resize(sizeof(g_snd_pkt2));
        memcpy(ipkt.data(), g_snd_pkt2, sizeof(g_snd_pkt2));
        epkt.resize(sizeof(g_rcv_pkt2));
        memcpy(epkt.data(), g_rcv_pkt2, sizeof(g_rcv_pkt2));
        std::cout << "Testing Switch to Host (MPLSoUDP)" << std::endl;
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
        std::cout << "Testing Host to Switch (VxLAN)" << std::endl;
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
        std::cout << "Testing Switch to Host (VxLAN)" << std::endl;
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
