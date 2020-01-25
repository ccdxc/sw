//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//--------------------------------------------------------------
//
#ifndef __TEST_CONFIG_HPP__
#define __TEST_CONFIG_HPP__

namespace pds_ms_test {
typedef struct test_config_s {
    uint32_t    local_ip_addr;
    uint32_t    local_ip_addr_2;
    uint32_t    local_lo_ip_addr;
    uint32_t    local_gwip_addr;
    uint32_t    remote_ip_addr;
    uint32_t    remote_ip_addr_2;
    uint32_t    remote_lo_ip_addr;
    uint32_t    remote_lo_ip_addr_2;
    uint32_t    local_mai_ip;
    uint32_t    eth_if_index;
    uint32_t    eth_if_index_2;
    uint32_t    lif_if_index;
    uint32_t    local_asn;
    uint32_t    remote_asn;
    uint32_t    vni;
    uint32_t    route_dest_ip;
    uint32_t    route_nh_ip;
    uint32_t    route_prefix_len;;
    uint8_t     manual_rd;
    uint8_t     manual_rt;
    uint8_t     rt[8];
    uint8_t     rd[8];
}test_config_t;

int parse_json_config (test_config_t *conf, uint8_t node);
} // end namepsace pds_ms_test

extern int g_node_id;

#endif // __TEST_CONFIG_HPP__
