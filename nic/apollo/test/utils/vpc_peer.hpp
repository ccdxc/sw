//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vpc peer test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_VPC_PEER_HPP__
#define __TEST_UTILS_VPC_PEER_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/test/utils/feeder.hpp"
#include "nic/apollo/test/utils/api_base.hpp"

namespace api_test {

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
    sdk::sdk_ret_t info_compare(const pds_vpc_peer_info_t *info) const;
};

// VPC peer test CRUD routines

API_CREATE(vpc_peer);
API_READ(vpc_peer);
API_UPDATE(vpc_peer);
API_DELETE(vpc_peer);

}    // namespace api_test

#endif    // __TEST_UTILS_VPC_PEER_HPP__
