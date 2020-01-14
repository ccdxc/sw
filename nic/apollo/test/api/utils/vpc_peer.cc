//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/vpc_peer.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// VPC peer feeder class routines
//----------------------------------------------------------------------------

void
vpc_peer_feeder::init(pds_obj_key_t key, pds_obj_key_t vpc1,
                      pds_obj_key_t vpc2, uint32_t num_vpc_peer) {
    this->key = key;
    this->vpc1 = vpc1;
    this->vpc2 = vpc2;
    this->num_obj = num_vpc_peer;
}

void
vpc_peer_feeder::iter_next(int width) {
    this->key = int2pdsobjkey(pdsobjkey2int(this->key) + width);
    vpc2 = int2pdsobjkey(pdsobjkey2int(vpc2) + width);
    cur_iter_pos++;
}

void
vpc_peer_feeder::key_build(pds_obj_key_t *key) const {
    *key = this->key;
}

void
vpc_peer_feeder::spec_build(pds_vpc_peer_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_vpc_peer_spec_t));
    this->key_build(&spec->key);

    spec->key = key;
    spec->vpc1 = vpc1;
    spec->vpc2 = vpc2;
}

bool
vpc_peer_feeder::key_compare(const pds_obj_key_t *key) const {
    return (this->key == *key);
}

bool
vpc_peer_feeder::spec_compare(const pds_vpc_peer_spec_t *spec) const {
    // todo spec data is not maintained
    return true;
}

}    // namespace api
}    // namespace test
