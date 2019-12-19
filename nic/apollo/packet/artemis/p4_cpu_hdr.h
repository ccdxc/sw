//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// This file contains p4 header interface for rx and tx packets

#ifndef __APOLLO_PACKET_ARTEMIS_P4_CPU_HDR_H__
#define __APOLLO_PACKET_ARTEMIS_P4_CPU_HDR_H___

// Session info containing packet actions
typedef struct session_info_hint_s {
    u32 iflow_tcp_state : 4;
    u32 iflow_tcp_seq_num;
    u32 iflow_tcp_ack_num;
    u32 iflow_tcp_win_sz : 16;
    u32 iflow_tcp_win_scale : 4;
    u32 rflow_tcp_state : 4;
    u32 rflow_tcp_seq_num;
    u32 rflow_tcp_ack_num;
    u32 rflow_tcp_win_sz : 16;
    u32 rflow_tcp_win_scale : 4;

    u64 tx_dst_ip[2];
    u32 tx_dst_l4port : 16;
    u32 nexthop_idx : 20;

    u32 tx_rewrite_flags_unused : 1;
    u32 tx_rewrite_flags_sipo   : 1;
    u32 tx_rewrite_flags_encap  : 1;
    u32 tx_rewrite_flags_dst_ip : 1;
    u32 tx_rewrite_flags_dport  : 1;
    u32 tx_rewrite_flags_src_ip : 2;
    u32 tx_rewrite_flags_dmac   : 1;

    //rx_rewrite_flags : 8;
    u32 rx_rewrite_flags_unused : 2;
    u32 rx_rewrite_flags_dst_ip : 2;
    u32 rx_rewrite_flags_sport  : 1;
    u32 rx_rewrite_flags_src_ip : 2;
    u32 rx_rewrite_flags_smac   : 1;

    u32 tx_policer_idx   : 12;
    u32 rx_policer_idx   : 12;
    u32 meter_idx        : 16;
    u64 timestamp        : 48;

    u8 drop              : 1;
    u8 entry_valid       : 1;
    u8 _pad              : 2;
    u64 u__pad_to_512b;
} __attribute__ ((packed)) session_info_hint_t;

// Meta received from P4 for rx packet
typedef struct p4_rx_cpu_hdr_s {
    uint16_t   packet_len;
    uint16_t   flags;
    uint16_t   local_vnic_tag;
    uint32_t   flow_hash;
    // offsets
    uint8_t  l2_offset;
    uint8_t  l3_offset;
    uint8_t  l4_offset;
    uint8_t  l2_inner_offset;
    uint8_t  l3_inner_offset;
    uint8_t  l4_inner_offset;
    uint8_t  payload_offset;

    uint8_t  assist_info[256];

    uint8_t  ses_info[64];          //session_info_hint_t

} __attribute__ ((packed)) p4_rx_cpu_hdr_t;

// Meta sent to P4 for tx packet
typedef struct p4_tx_cpu_hdr_s {
    union {
        uint8_t flags_octet;
        struct {
#if __BYTE_ORDER == __BIG_ENDIAN
            uint8_t pad0                    : 4;
            uint8_t txdma_drop_event        : 1;
            uint8_t redirect_to_arm         : 1;
            uint8_t lpm_bypass              : 1;
            uint8_t direction               : 1;
#else
            uint8_t direction               : 1;
            uint8_t lpm_bypass              : 1;
            uint8_t redirect_to_arm         : 1;
            uint8_t txdma_drop_event        : 1;
            uint8_t pad0                    : 4;
#endif
        };
    };
} __attribute__ ((packed)) p4_tx_cpu_hdr_t;

#endif     // __APOLLO_PACKET_ARTEMIS_P4_CPU_HDR_H__
