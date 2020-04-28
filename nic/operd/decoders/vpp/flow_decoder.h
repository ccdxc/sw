//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// definitions for structures for flow exports
///
//----------------------------------------------------------------------------

#ifndef __OPERD_DECODERS_VPP_FLOW_DECODER_H__
#define __OPERD_DECODERS_VPP_FLOW_DECODER_H__


#include <stdint.h>
#include "include/sdk/ip.hpp"
#include "include/sdk/eth.hpp"

enum operd_flow_type {
    OPERD_FLOW_TYPE_L2,
    OPERD_FLOW_TYPE_IP4,
    OPERD_FLOW_TYPE_IP6,
};

enum operd_flow_action {
    OPERD_FLOW_ACTION_ALLOW,
    OPERD_FLOW_ACTION_DENY,
};

enum operd_flow_logtype {
    OPERD_FLOW_LOGTYPE_ADD,
    OPERD_FLOW_LOGTYPE_DEL,
    OPERD_FLOW_LOGTYPE_ACTIVE,
};

typedef struct operd_flow_key_v4 {
    uint32_t src;
    uint32_t dst;
    uint16_t sport;
    uint16_t dport;
    uint8_t proto;
    uint16_t lookup_id;
} operd_flow_key_v4_t;

typedef struct operd_flow_key_v6 {
    uint8_t src[IP6_ADDR8_LEN];
    uint8_t dst[IP6_ADDR8_LEN];
    uint16_t sport;
    uint16_t dport;
    uint8_t proto;
    uint16_t lookup_id;
} operd_flow_key_v6_t;

typedef struct operd_flow_key_l2 {
    uint8_t src[ETH_ADDR_LEN];
    uint8_t dst[ETH_ADDR_LEN];
    uint16_t ether_type;
    uint16_t bd_id;
} operd_flow_key_l2_t;

typedef struct operd_flow_stats {
    uint64_t iflow_bytes_count;
    uint64_t iflow_packets_count;
    uint64_t rflow_bytes_count;
    uint64_t rflow_packets_count;
} operd_flow_stats_t;

typedef struct operd_flow_nat_data {
    uint32_t src_nat_addr;
    uint32_t dst_nat_addr;
    uint16_t src_nat_port;
    uint16_t dst_nat_port;
} operd_flow_nat_data_t;

typedef struct operd_flow {
    uint8_t type;               // operd_flow_type
    uint8_t action;             // operd_flow_action
    uint8_t logtype;            // operd_flow_logtype
    uint32_t session_id;
    union {
        operd_flow_key_v4_t v4;
        operd_flow_key_v6_t v6;
        operd_flow_key_l2_t l2;
    };
    operd_flow_nat_data_t nat_data;
    operd_flow_stats_t stats;
} operd_flow_t;

#endif // __OPERD_DECODERS_VPP_FLOW_DECODER_H__
