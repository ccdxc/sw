//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_VPC_HPP__
#define __TEST_API_UTILS_VPC_HPP__

#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// Export variables
extern pds_vpc_key_t k_vpc_key;

// VPC test feeder class
class vpc_feeder : public feeder {
public:
    pds_vpc_key_t key;
    pds_vpc_type_t type;
    pds_encap_t fabric_encap;
    std::string cidr_str;
    ip_prefix_t pfx;
    std::string vr_mac;

    // Constructor
    vpc_feeder() { };
    vpc_feeder(const vpc_feeder& feeder) {
        init(feeder.key, feeder.type, feeder.cidr_str,
             feeder.vr_mac, feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_vpc_key_t key, pds_vpc_type_t type, std::string cidr_str,
              std::string vr_mac, uint32_t num_vpc = 1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_vpc_key_t *key) const;
    void spec_build(pds_vpc_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_vpc_key_t *key) const;
    bool spec_compare(const pds_vpc_spec_t *spec) const;

};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_key_t *key) {
    os << " id: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_spec_t *spec) {
    os << &spec->key
       << " type: " << spec->type
       << " v4 cidr: " << ipv4pfx2str(&spec->v4_prefix)
       << " v6 cidr: " << ippfx2str(&spec->v6_prefix)
       << " vr mac: " << macaddr2str(spec->vr_mac)
       << " fabric encap: " << pds_encap2str(&spec->fabric_encap)
       << " v4 rt table: " << spec->v4_route_table.id
       << " v6 rt table: " << spec->v6_route_table.id
       << " nat46 pfx: " << ippfx2str(&spec->nat46_prefix);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_status_t *status) {
    os << " HW id: " << status->hw_id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_info_t *obj) {
    os << "VPC info =>"
       << &obj->spec
       << &obj->status
       << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const vpc_feeder& obj) {
    os << "VPC feeder =>"
       << " id: " << obj.key.id
       << " cidr_str: " << obj.cidr_str
       << " vnid: " << obj.fabric_encap.val.vnid
       << " rmac: " << obj.vr_mac << " ";
    return os;
}

// CRUD prototypes
API_CREATE(vpc);
API_READ(vpc);
API_UPDATE(vpc);
API_DELETE(vpc);

// Misc function prototypes
void sample_vpc_setup(pds_batch_ctxt_t bctxt, pds_vpc_type_t type);
void sample_vpc_setup_validate(pds_vpc_type_t type);
void sample_vpc_teardown(pds_batch_ctxt_t bctxt, pds_vpc_type_t type);
void sample1_vpc_setup(pds_batch_ctxt_t bctxt, pds_vpc_type_t type);
void sample1_vpc_setup_validate(pds_vpc_type_t type);
void sample1_vpc_teardown(pds_batch_ctxt_t bctxt, pds_vpc_type_t type);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_VPC_HPP__
