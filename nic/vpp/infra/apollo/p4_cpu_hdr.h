//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// This file contains p4 header interface for rx and tx packets

#ifndef __VPP_INFRA_APOLLO_P4_CPU_HDR_H__
#define __VPP_INFRA_APOLLO_P4_CPU_HDR_H__

#include <nic/apollo/p4/include/defines.h>
#include <vppinfra/clib.h>

#define VPP_CPU_FLAGS_VLAN_VALID           APOLLO_CPU_FLAGS_VLAN_VALID
#define VPP_CPU_FLAGS_IPV4_1_VALID         APOLLO_CPU_FLAGS_IPV4_1_VALID
#define VPP_CPU_FLAGS_IPV6_1_VALID         APOLLO_CPU_FLAGS_IPV6_1_VALID
#define VPP_CPU_FLAGS_ETH_2_VALID          APOLLO_CPU_FLAGS_ETH_2_VALID
#define VPP_CPU_FLAGS_IPV4_2_VALID         APOLLO_CPU_FLAGS_IPV4_2_VALID
#define VPP_CPU_FLAGS_IPV6_2_VALID         APOLLO_CPU_FLAGS_IPV6_2_VALID
#define VPP_CPU_FLAGS_DIRECTION            APOLLO_CPU_FLAGS_DIRECTION
#define VPP_ARM_TO_P4_HDR_SZ               APOLLO_PREDICATE_HDR_SZ
#define VPP_P4_TO_ARM_HDR_SZ               APOLLO_P4_TO_ARM_HDR_SZ

// Meta received from P4 for rx packet
typedef CLIB_PACKED(struct p4_rx_cpu_hdr_s {
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
}) p4_rx_cpu_hdr_t;

// Meta sent to P4 for tx packet
typedef CLIB_PACKED(struct p4_tx_cpu_hdr_s {
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
}) p4_tx_cpu_hdr_t;

#endif     // __VPP_INFRA_APOLLO_P4_CPU_HDR_H__

