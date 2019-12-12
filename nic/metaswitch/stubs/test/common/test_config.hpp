//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//--------------------------------------------------------------
//
#ifndef __TEST_CONFIG_HPP__
#define __TEST_CONFIG_HPP__

namespace pds_ms_test {
typedef struct test_config_s {
    uint32_t    local_ip_addr;
    uint32_t    local_gwip_addr;
    uint32_t    remote_ip_addr;
    uint32_t    local_mai_ip;
    uint32_t    eth_if_index;
    uint32_t    lif_if_index;
    uint32_t    local_asn;
    uint32_t    remote_asn;
    uint32_t    vni;
}test_config_t;

int parse_json_config (test_config_t *conf, uint8_t node);
} // end namepsace pds_ms_test


#endif // __TEST_CONFIG_HPP__
