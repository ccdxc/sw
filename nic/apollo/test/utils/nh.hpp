//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the nexthop test utility routines
///
//----------------------------------------------------------------------------
#ifndef __TEST_UTILS_NH_HPP__
#define __TEST_UTILS_NH_HPP__

#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/test/utils/feeder.hpp"

namespace api_test {

// NH test feeder class
class nh_feeder : public feeder {
public:
    pds_nexthop_id_t id;
    pds_nh_type_t type;
    ip_addr_t ip;
    uint64_t mac;
    uint16_t vlan;
    pds_vpc_id_t vpc_id;

    // Constructor
    nh_feeder() { };
    nh_feeder(const nh_feeder& feeder) {
        init(ipaddr2str(&feeder.ip), feeder.mac, feeder.num_obj,
             feeder.id, feeder.type, feeder.vlan, feeder.vpc_id);
    }

    // Initialize feeder with the base set of values
    void init(std::string ip_str="0.0.0.0",
              uint64_t mac=0x0E0D0A0B0200,
              uint32_t num_obj=PDS_MAX_NEXTHOP,
              pds_nexthop_id_t id=1,
              pds_nh_type_t type=PDS_NH_TYPE_IP,
              uint16_t vlan=1, pds_vpc_id_t vpc_id=1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_nexthop_key_t *key) const;
    void spec_build(pds_nexthop_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_nexthop_key_t *key) const;
    bool spec_compare(const pds_nexthop_spec_t *spec) const;
    sdk::sdk_ret_t info_compare(const pds_nexthop_info_t *info) const;
};

// Function prototypes
sdk::sdk_ret_t create(nh_feeder& feeder);
sdk::sdk_ret_t read(nh_feeder& feeder);
sdk::sdk_ret_t update(nh_feeder& feeder);
sdk::sdk_ret_t del(nh_feeder& feeder);

void sample_nh_setup(void);
void sample_nh_teardown(void);

}    // namespace api_test

#endif    // __TEST_UTILS_NH_HPP__
