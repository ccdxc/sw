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
#include "nic/apollo/test/utils/api_base.hpp"

namespace api_test {

// NH test feeder class
class nexthop_feeder : public feeder {
public:
    pds_nexthop_id_t id;
    pds_nh_type_t type;
    ip_addr_t ip;
    uint64_t mac;
    uint16_t vlan;
    pds_vpc_id_t vpc_id;

    // Constructor
    nexthop_feeder() { };
    nexthop_feeder(const nexthop_feeder& feeder) {
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
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const nexthop_feeder& obj) {
    os << "NH feeder =>"
       << " id: " << obj.id
       << " type: " << obj.type
       << " ip: " << obj.ip
       << " mac: " << mac2str(obj.mac)
       << " vlan: " << obj.vlan
       << " vpc: " << obj.vpc_id;
    return os;
}

// CRUD prototypes
API_CREATE(nexthop);
API_READ(nexthop);
API_UPDATE(nexthop);
API_DELETE(nexthop);

// Misc function prototypes
void sample_nexthop_setup(void);
void sample_nexthop_teardown(void);

}    // namespace api_test

#endif    // __TEST_UTILS_NH_HPP__
