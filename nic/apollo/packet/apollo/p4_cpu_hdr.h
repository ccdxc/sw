//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// This file contains p4 header interface for rx and tx packets

#ifndef __APOLLO_PACKET_P4_CPU_HDR_H__
#define __APOLLO_PACKET_P4_CPU_HDR_H__

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

#endif     // __APOLLO_PACKET_P4_CPU_HDR_H__
