#include <stdint.h>

typedef struct operd_flow_v4 {
    uint32_t src;
    uint32_t dst;
    uint16_t sport;
    uint16_t dport;
} operd_flow_v4_t;

typedef struct operd_flow_v6 {
} operd_flow_v6_t;

typedef struct operd_flow {
    uint8_t version;
    union {
        operd_flow_v4_t v4;
        operd_flow_v4_t v6;
    };
} operd_flow_t;
