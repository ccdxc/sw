
#ifndef _FTE_EXAMPLE_H_
#define _FTE_EXAMPLE_H_

/* Version 1 of the flow_key */
struct flow_key_t {
    uint32_t tid;
    uint32_t sip;
    uint32_t dip;
    uint16_t proto;
    uint16_t sport;
    uint16_t dport;
};

struct flow_t {
    struct flow_key_t floy_key;

    struct {
        uint8_t key:1;
        uint8_t attrs:1;
        uint8_t action:1;
        uint8_t flow_state:1;
        uint8_t fwding:1;
        uint8_t mcast_info:1;
        uint8_t ingress_info:1;
        uint8_t mirror_info:1;
        uint8_t qos_info:1;
        uint8_t lkp_info:1;
    } valid_;

}
#endif
