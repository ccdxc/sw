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

enum operd_flow_operation {
    OPERD_FLOW_OPERATION_ADD,
    OPERD_FLOW_OPERATION_DEL,
};

typedef struct operd_flow_v4 {
    uint32_t src;
    uint32_t dst;
    uint16_t sport;
    uint16_t dport;
    uint8_t proto;
    uint16_t lookup_id;
} operd_flow_v4_t;

typedef struct operd_flow_v6 {
    uint8_t src[IP6_ADDR8_LEN];
    uint8_t dst[IP6_ADDR8_LEN];
    uint16_t sport;
    uint16_t dport;
    uint8_t proto;
    uint16_t lookup_id;
} operd_flow_v6_t;

typedef struct operd_flow_l2 {
    uint8_t src[ETH_ADDR_LEN];
    uint8_t dst[ETH_ADDR_LEN];
    uint16_t ether_type;
    uint16_t bd_id;
} operd_flow_l2_t;

typedef struct operd_flow {
    uint8_t type;               // operd_flow_type
    uint8_t action;             // operd_flow_action
    uint8_t op;                 // operd_flow_operation
    union {
        operd_flow_v4_t v4;
        operd_flow_v6_t v6;
        operd_flow_l2_t l2;
    };
} operd_flow_t;
