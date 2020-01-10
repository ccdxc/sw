//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_VPC_PEER_HPP__
#define __TEST_API_UTILS_VPC_PEER_HPP__

#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// VPC peer test feeder class
class vpc_peer_feeder : public feeder {
public:
    pds_vpc_peer_key_t key;
    pds_vpc_key_t vpc1;
    pds_vpc_key_t vpc2;

    // Constructor
    vpc_peer_feeder() { };
    vpc_peer_feeder(const vpc_peer_feeder& feeder) {
        init(feeder.key, feeder.vpc1, feeder.vpc2, feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_vpc_peer_key_t key, pds_vpc_key_t vpc1, pds_vpc_key_t vpc2,
              uint32_t num_vpc_peer = 1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_vpc_peer_key_t *key) const;
    void spec_build(pds_vpc_peer_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_vpc_peer_key_t *key) const;
    bool spec_compare(const pds_vpc_peer_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_peer_key_t *key) {
    os << " id: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_peer_spec_t *spec) {
    os << &spec->key
       << " vpc1: " << spec->vpc1.tostr()
       << " vpc2: " << spec->vpc2.tostr();
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_peer_status_t *status) {
    os << " HW id1: " << status->hw_id1
       << " HW id2: " << status->hw_id2;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_peer_info_t *obj) {
    os << "VPC Peer info =>"
       << &obj->spec
       << &obj->status
       << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const vpc_peer_feeder& obj) {
    os << "VPC Peer feeder =>"
        << " id: " << obj.key.id
        << " vpc1 : " << obj.vpc1.tostr()
        << " vpc2 : " << obj.vpc2.tostr() << " ";
    return os;
}

// CRUD prototypes
API_CREATE(vpc_peer);
API_READ(vpc_peer);
API_UPDATE(vpc_peer);
API_DELETE(vpc_peer);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_VPC_PEER_HPP__
