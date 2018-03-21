//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// sw PHV injection
//-----------------------------------------------------------------------------

#ifndef __HAL_PD_SWPHV_HPP__
#define __HAL_PD_SWPHV_HPP__

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/pd_api.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#ifdef SWPHV_BIGENDIAN
struct phv_intr_global_t {
    uint64_t    tm_iport : 4;
    uint64_t    tm_oport : 4;
    uint64_t    tm_iq : 5;
    uint64_t    lif : 11;
    uint64_t    timestamp : 48;
    uint64_t    tm_span_session : 8;
    uint64_t    tm_replicate_ptr : 16;
    uint64_t    tm_replicate_en : 1;
    uint64_t    tm_cpu : 1;
    uint64_t    tm_q_depth : 14;
    uint64_t    drop : 1;
    uint64_t    bypass : 1;
    uint64_t    hw_error : 1;
    uint64_t    tm_oq : 5;
    uint64_t    debug_trace : 1;
    uint64_t    csum_err : 5;
    uint64_t    error_bits : 6;
    uint64_t    tm_instance_type : 4;
} __PACK__;

struct phv_intr_p4_t {
    uint64_t    crc_err : 1;
    uint64_t    len_err : 4;
    uint64_t    recirc_count : 3;
    uint64_t    parser_err : 1;
    uint64_t    p4_pad : 1;
    uint64_t    frame_size : 14;
    uint64_t    no_data : 1;
    uint64_t    recirc : 1;
    uint64_t    packet_len : 14;
} __PACK__;

struct phv_intr_rxdma_t {
    uint64_t    intr_qid : 24;
    uint64_t    intr_dma_cmd_ptr : 6;
    uint64_t    intr_qstate_addr : 34;
    uint64_t    intr_qtype : 3;
    uint64_t    intr_rx_splitter_offset : 10;
    uint64_t    intr_rxdma_rsv : 3;
} __PACK__;

struct phv_intr_txdma_t {
    uint64_t    intr_qid : 24;
    uint64_t    intr_dma_cmd_ptr : 6;
    uint64_t    intr_qstate_addr : 34;
    uint64_t    intr_qtype : 3;
    uint64_t    intr_txdma_rsv : 5;
    uint64_t    padding : 8;
} __PACK__;

typedef struct sw_phv_intr_ {
    struct phv_intr_global_t p4_intr_global;
    struct phv_intr_p4_t     p4_intr;
    union {
        struct phv_intr_rxdma_t  p4_intr_rxdma;
        struct phv_intr_txdma_t  p4_intr_txdma;
    } __PACK__ dma;
    uint8_t    padding[32];
} __PACK__ sw_phv_intr_t;

#else
struct phv_intr_global_t {
    uint64_t    tm_instance_type : 4;
    uint64_t    error_bits : 6;
    uint64_t    csum_err : 5;
    uint64_t    debug_trace : 1;
    uint64_t    tm_oq : 5;
    uint64_t    hw_error : 1;
    uint64_t    bypass : 1;
    uint64_t    drop : 1;
    uint64_t    tm_q_depth : 14;
    uint64_t    tm_cpu : 1;
    uint64_t    tm_replicate_en : 1;
    uint64_t    tm_replicate_ptr : 16;
    uint64_t    tm_span_session : 8;
    uint64_t    timestamp : 48;
    uint64_t    lif : 11;
    uint64_t    tm_iq : 5;
    uint64_t    tm_oport : 4;
    uint64_t    tm_iport : 4;
} __PACK__;

struct phv_intr_p4_t {
    uint64_t    packet_len : 14;
    uint64_t    recirc : 1;
    uint64_t    no_data : 1;
    uint64_t    frame_size : 14;
    uint64_t    p4_pad : 1;
    uint64_t    parser_err : 1;
    uint64_t    recirc_count : 3;
    uint64_t    len_err : 4;
    uint64_t    crc_err : 1;
} __PACK__;

struct phv_intr_rxdma_t {
    uint64_t    intr_rxdma_rsv : 3;
    uint64_t    intr_rx_splitter_offset : 10;
    uint64_t    intr_qtype : 3;
    uint64_t    intr_qstate_addr : 34;
    uint64_t    intr_dma_cmd_ptr : 6;
    uint64_t    intr_qid : 24;
} __PACK__;

struct phv_intr_txdma_t {
    uint64_t    padding : 8;
    uint64_t    intr_txdma_rsv : 5;
    uint64_t    intr_qtype : 3;
    uint64_t    intr_qstate_addr : 34;
    uint64_t    intr_dma_cmd_ptr : 6;
    uint64_t    intr_qid : 24;
} __PACK__;

typedef struct sw_phv_intr_ {
    uint8_t    padding[32];
    union {
        struct phv_intr_rxdma_t  p4_intr_rxdma;
        struct phv_intr_txdma_t  p4_intr_txdma;
    } __PACK__ dma;
    struct phv_intr_p4_t     p4_intr;
    struct phv_intr_global_t p4_intr_global;
} __PACK__ sw_phv_intr_t;


typedef struct __attribute__ ((__packed__)) swphv_ingress_phv_ {
    //----- Flit 0 -----
    union {
        struct  __attribute__ ((__packed__)) {
            uint32_t app_header_app_type : 4; // BE[248] FLE[263:260]
            uint32_t app_header_table0_valid : 1; // BE[252] FLE[259:259]
            uint32_t app_header_table1_valid : 1; // BE[253] FLE[258:258]
            uint32_t app_header_table2_valid : 1; // BE[254] FLE[257:257]
            uint32_t app_header_table3_valid : 1; // BE[255] FLE[256:256]
            uint8_t app_header_app_data1[32]; // BE[256] FLE[255:0]
        };
        struct  __attribute__ ((__packed__)) {
            uint32_t tcp_app_header_p4plus_app_id : 4; // BE[248] FLE[263:260]
            uint32_t tcp_app_header_table0_valid : 1; // BE[252] FLE[259:259]
            uint32_t tcp_app_header_table1_valid : 1; // BE[253] FLE[258:258]
            uint32_t tcp_app_header_table2_valid : 1; // BE[254] FLE[257:257]
            uint32_t tcp_app_header_table3_valid : 1; // BE[255] FLE[256:256]
            uint32_t tcp_app_header_flags : 8; // BE[256] FLE[255:248]
            uint32_t tcp_app_header_udp_opt_bytes : 8; // BE[264] FLE[247:240]
            uint32_t tcp_app_header_flow_index : 24; // BE[272] FLE[239:216]
            uint32_t tcp_app_header_ip_id_delta : 16; // BE[296] FLE[215:200]
            uint64_t tcp_app_header_tcp_seq_delta : 32; // BE[312] FLE[199:168]
            uint32_t tcp_app_header_gso_start : 14; // BE[344] FLE[167:154]
            uint32_t tcp_app_header_byte_align_pad0 : 2; // BE[358] FLE[153:152]
            uint32_t tcp_app_header_gso_offset : 14; // BE[360] FLE[151:138]
            uint32_t tcp_app_header_flow_index_valid : 1; // BE[374] FLE[137:137]
            uint32_t tcp_app_header_gso_valid : 1; // BE[375] FLE[136:136]
            uint32_t tcp_app_header_vlan_tag : 16; // BE[376] FLE[135:120]
            uint8_t tcp_app_header_bottom_pad_[15]; // FLE[119:0]
        };
    };

    uint32_t p4_txdma_intr_txdma_rsv : 5; // BE[243] FLE[268:264]
    uint32_t p4_txdma_intr_qtype : 3; // BE[240] FLE[271:269]
    uint64_t p4_txdma_intr_qstate_addr : 34; // BE[206] FLE[305:272]
    uint32_t p4_txdma_intr_dma_cmd_ptr : 6; // BE[200] FLE[311:306]
    uint32_t p4_txdma_intr_qid : 24; // BE[176] FLE[335:312]
    uint32_t p4_intr_packet_len : 14; // BE[162] FLE[349:336]
    uint32_t p4_intr_recirc : 1; // BE[161] FLE[350:350]
    uint32_t p4_intr_no_data : 1; // BE[160] FLE[351:351]
    uint32_t p4_intr_frame_size : 14; // BE[146] FLE[365:352]
    uint32_t p4_intr_p4_pad : 1; // BE[145] FLE[366:366]
    uint32_t p4_intr_parser_err : 1; // BE[144] FLE[367:367]
    uint32_t p4_intr_recirc_count : 3; // BE[141] FLE[370:368]
    uint32_t p4_intr_len_err : 4; // BE[137] FLE[374:371]
    uint32_t p4_intr_crc_err : 1; // BE[136] FLE[375:375]
    uint32_t p4_intr_global_tm_instance_type : 4; // BE[132] FLE[379:376]
    uint32_t p4_intr_global_error_bits : 6; // BE[126] FLE[385:380]
    uint32_t p4_intr_global_csum_err : 5; // BE[121] FLE[390:386]
    uint32_t p4_intr_global_debug_trace : 1; // BE[120] FLE[391:391]
    uint32_t p4_intr_global_tm_oq : 5; // BE[115] FLE[396:392]
    uint32_t p4_intr_global_hw_error : 1; // BE[114] FLE[397:397]
    uint32_t p4_intr_global_bypass : 1; // BE[113] FLE[398:398]
    uint32_t p4_intr_global_drop : 1; // BE[112] FLE[399:399]
    uint32_t p4_intr_global_tm_q_depth : 14; // BE[98] FLE[413:400]
    uint32_t p4_intr_global_tm_cpu : 1; // BE[97] FLE[414:414]
    uint32_t p4_intr_global_tm_replicate_en : 1; // BE[96] FLE[415:415]
    uint32_t p4_intr_global_tm_replicate_ptr : 16; // BE[80] FLE[431:416]
    uint32_t p4_intr_global_tm_span_session : 8; // BE[72] FLE[439:432]
    uint64_t p4_intr_global_timestamp : 48; // BE[24] FLE[487:440]
    uint32_t p4_intr_global_lif : 11; // BE[13] FLE[498:488]
    uint32_t p4_intr_global_tm_iq : 5; // BE[8] FLE[503:499]
    uint32_t p4_intr_global_tm_oport : 4; // BE[4] FLE[507:504]
    uint32_t p4_intr_global_tm_iport : 4; // BE[0] FLE[511:508]
} swphv_ingress_phv_t;

#endif /* SWPHV_BIGENDIAN */


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_SWPHV_HPP__

