//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// This file contains p4 header interface for rx and tx packets

#ifndef __APOLLO_PACKET_APULU_P4_CPU_HDR_H__
#define __APOLLO_PACKET_APULU_P4_CPU_HDR_H__

// Meta received from P4 for rx packet
typedef struct p4_rx_cpu_hdr_s {
    uint16_t packet_len;
    uint8_t  nacl_data;
    uint8_t  flags;
    uint16_t ingress_bd_id;
    uint32_t flow_hash;

    // offsets
    uint8_t  l2_offset;
    uint8_t  l3_offset;
    uint8_t  l4_offset;
    uint8_t  l2_inner_offset;
    uint8_t  l3_inner_offset;
    uint8_t  l4_inner_offset;
    uint8_t  payload_offset;
    uint8_t  tcp_flags;

    uint32_t session_id;
    uint16_t lif;
    uint16_t egress_bd_id;
    uint16_t service_xlate_id;
    uint16_t mapping_xlate_id;
    uint16_t tx_meter_id;
    uint16_t nexthop_id;
    uint16_t vpc_id;
    uint16_t vnic_id;
    uint16_t dnat_id;
    union {
        uint16_t flags_short;
        struct {
#if __BYTE_ORDER == __BIG_ENDIAN
            uint8_t  pad                : 1;
            uint8_t  rx_packet          : 1;
            uint8_t  flow_hit           : 1;
            uint8_t  flow_role          : 1;
            uint8_t  is_local           : 1;
            uint8_t  snat_type          : 2;
            uint8_t  dnat_en            : 1;
#else
            uint8_t  dnat_en            : 1;
            uint8_t  snat_type          : 2;
            uint8_t  is_local           : 1;
            uint8_t  flow_role          : 1;
            uint8_t  flow_hit           : 1;
            uint8_t  rx_packet          : 1;
            uint8_t  pad                : 1;
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
            uint8_t  mapping_hit        : 1;
            uint8_t  sacl_action        : 1;
            uint8_t  sacl_root          : 3;
            uint8_t  nexthop_type       : 2;
            uint8_t  drop               : 1;
#else
            uint8_t  drop               : 1;
            uint8_t  nexthop_type       : 2;
            uint8_t  sacl_root          : 3;
            uint8_t  sacl_action        : 1;
            uint8_t  mapping_hit        : 1;
#endif
        };
    };
} __attribute__ ((packed)) p4_rx_cpu_hdr_t;

// Meta sent to P4 for tx packet
typedef struct p4_tx_cpu_hdr_s {
    union {
        uint16_t lif_flags;
        struct {
            uint16_t lif_sbit0_ebit7    : 8;
            uint16_t lif_sbit8_ebit10   : 3;
            uint16_t nexthop_valid      : 1;
            uint16_t pad                : 4;
        };
    };
    uint8_t nexthop_type;
    uint16_t nexthop_id;
} __attribute__ ((packed)) p4_tx_cpu_hdr_t;

#endif     // __APOLLO_PACKET_APULU_P4_CPU_HDR_H__
