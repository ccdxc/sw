//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vpc test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_VPC_HPP__
#define __TEST_UTILS_VPC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

namespace api_test {

// VPC test feeder class
class vpc_feeder {
public:
    pds_vpc_key_t key;
    pds_vpc_type_t type;
    std::string cidr_str;
    ip_prefix_t pfx;
    uint32_t num_obj;

    // Constructor
    vpc_feeder() { };
    vpc_feeder(const vpc_feeder& feeder) {
        init(feeder.key, feeder.type, feeder.cidr_str, feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_vpc_key_t key, pds_vpc_type_t type, std::string cidr_str,
              uint32_t num_vpc = 1);

    // Iterate helper routines
    void iter_init() { cur_iter_pos = 0; }
    bool iter_more() { return (cur_iter_pos < num_obj); }
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_vpc_key_t *key);
    void spec_build(pds_vpc_spec_t *spec);

    // Compare routines
    bool key_compare(pds_vpc_key_t *key);
    bool spec_compare(pds_vpc_spec_t *spec);
    sdk::sdk_ret_t info_compare(pds_vpc_info_t *info);

private:
    uint32_t cur_iter_pos;
};

// Export variables
extern pds_vpc_key_t k_vpc_key;

// Function prototypes
sdk::sdk_ret_t create(vpc_feeder& feeder);
sdk::sdk_ret_t read(vpc_feeder& feeder);
sdk::sdk_ret_t update(vpc_feeder& feeder);
sdk::sdk_ret_t del(vpc_feeder& feeder);

void sample_vpc_setup(pds_vpc_type_t type);
void sample_vpc_setup_validate(pds_vpc_type_t type);
void sample_vpc_teardown(pds_vpc_type_t type);

}    // namespace api_test

#endif    // __TEST_UTILS_VPC_HPP__
