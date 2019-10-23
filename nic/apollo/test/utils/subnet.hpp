//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_SUBNET_HPP__
#define __TEST_UTILS_SUBNET_HPP__

#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/feeder.hpp"
#include "nic/apollo/api/subnet_utils.hpp"

namespace api_test {

// Subnet test feeder class
class subnet_feeder : public feeder {
public:
    pds_subnet_key_t key;
    pds_vpc_key_t vpc;
    std::string cidr_str;
    ip_prefix_t pfx;
    std::string vr_ip;
    std::string vr_mac;
    pds_route_table_key_t v4_route_table;
    pds_route_table_key_t v6_route_table;
    pds_policy_key_t ing_v4_policy;
    pds_policy_key_t ing_v6_policy;
    pds_policy_key_t egr_v4_policy;
    pds_policy_key_t egr_v6_policy;
    pds_encap_t fabric_encap;

    // Constructor
    subnet_feeder() { };
    subnet_feeder(const subnet_feeder& feeder) {
        init(feeder.key, feeder.vpc, feeder.cidr_str,
             feeder.vr_mac, feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_subnet_key_t key, pds_vpc_key_t vpc_key,
              std::string cidr_str, std::string vrmac_str, 
              int num_subnet = 1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_subnet_key_t *key) const;
    void spec_build(pds_subnet_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_subnet_key_t *key) const;
    bool spec_compare(const pds_subnet_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const subnet_feeder& obj) {
    os << "Subnet feeder =>"
        << " id: " << obj.key.id
        << " vpc: " << obj.vpc.id
        << " cidr_str: " << obj.cidr_str
        << " vr_ip: " << obj.vr_ip
        << " vr_mac: " << obj.vr_mac
        << " v4_rt: " << obj.v4_route_table.id
        << " v6_rt: " << obj.v6_route_table.id
        << " v4_in_pol: " << obj.ing_v4_policy.id
        << " v6_in_pol: " << obj.ing_v6_policy.id
        << " v4_eg_pol: " << obj.egr_v4_policy.id
        << " v6_eg_pol: " << obj.egr_v6_policy.id
        << " vnid: " << obj.fabric_encap.val.vnid;
    return os;
}

// CRUD prototypes
API_CREATE(subnet);
API_READ(subnet);
API_UPDATE(subnet);
API_DELETE(subnet);

// Export variables
extern pds_subnet_key_t k_subnet_key;

// Misc function prototypes
void sample_subnet_setup(pds_batch_ctxt_t bctxt);
void sample_subnet_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api_test

#endif    // __TEST_UTILS_SUBNET_HPP__
