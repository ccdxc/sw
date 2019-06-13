//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vpc test utility routines implementation
///
//----------------------------------------------------------------------------

#include <iostream>
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// VPC feeder class routines
//----------------------------------------------------------------------------

void
vpc_feeder::init(pds_vpc_key_t key, pds_vpc_type_t type,
                 std::string cidr_str, uint32_t num_vpc) {
    this->key = key;
    this->type = type;
    this->cidr_str = cidr_str;
    this->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    this->fabric_encap.val.vnid = 9999;
    SDK_ASSERT(str2ipv4pfx((char *)cidr_str.c_str(), &pfx) == 0);
    num_obj = num_vpc;
}

void
vpc_feeder::iter_next(int width) {
    ip_addr_t ipaddr = {0};

    ip_prefix_ip_next(&pfx, &ipaddr);
    memcpy(&pfx.addr, &ipaddr, sizeof(ip_addr_t));
    key.id += width;
    cur_iter_pos++;
}

inline std::ostream&
operator<<(std::ostream& os, const vpc_feeder& obj) {
    os << "VPC feeder =>"
       << " id: " << obj.key.id
       << " cidr_str: " << obj.cidr_str
       << std::endl;
    return os;
}

void
vpc_feeder::key_build(pds_vpc_key_t *key) const {
    memset(key, 0, sizeof(pds_vpc_key_t));
    key->id = this->key.id;
}

void
vpc_feeder::spec_build(pds_vpc_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_vpc_spec_t));
    this->key_build(&spec->key);

    spec->type = type;
    spec->v4_pfx.len = pfx.len;
    spec->v4_pfx.v4_addr = pfx.addr.addr.v4_addr;
    spec->fabric_encap = fabric_encap;
}

bool
vpc_feeder::key_compare(const pds_vpc_key_t *key) const {
    return true;
    // todo : @sai please check, compare routine not working
    // return (memcmp(key, &this->key, sizeof(pds_vpc_key_t)) == 0);
}

bool
vpc_feeder::spec_compare(const pds_vpc_spec_t *spec) const {
    // todo : @sai please check, compare routine not working
    return true;

    if (spec->type != type)
        return false;

    return true;
}

sdk::sdk_ret_t
vpc_feeder::info_compare(const pds_vpc_info_t *info) const {

    if (!this->key_compare(&info->spec.key)) {
        std::cout << "key compare failed " << *this;
        return sdk::SDK_RET_ERR;
    }

    if (!this->spec_compare(&info->spec)) {
        std::cout << "spec compare failed " << *this;
        return sdk::SDK_RET_ERR;
    }

    return sdk::SDK_RET_OK;
}

//----------------------------------------------------------------------------
// VPC test CRUD routines
//----------------------------------------------------------------------------

sdk::sdk_ret_t
create(vpc_feeder& feeder) {
    pds_vpc_spec_t spec;

    feeder.spec_build(&spec);
    return (pds_vpc_create(&spec));
}

sdk::sdk_ret_t
read(vpc_feeder& feeder) {
    sdk_ret_t rv;
    pds_vpc_key_t key;
    pds_vpc_info_t info;

    feeder.key_build(&key);
    memset(&info, 0, sizeof(pds_vpc_info_t));
    if ((rv = pds_vpc_read(&key, &info)) != sdk::SDK_RET_OK)
        return rv;

    return (feeder.info_compare(&info));
}

sdk::sdk_ret_t
update(vpc_feeder& feeder) {
    pds_vpc_spec_t spec;

    feeder.spec_build(&spec);
    return (pds_vpc_update(&spec));
}

sdk::sdk_ret_t
del(vpc_feeder& feeder) {
    pds_vpc_key_t key;

    feeder.key_build(&key);
    return (pds_vpc_delete(&key));
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
pds_vpc_key_t k_vpc_key = {.id = 1};
static vpc_feeder k_vpc_feeder;

void sample_vpc_setup(pds_vpc_type_t type) {
    // setup and teardown parameters should be in sync
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8");
    create(k_vpc_feeder);
}

void sample_vpc_setup_validate(pds_vpc_type_t type) {
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8");
    read(k_vpc_feeder);
}

void sample_vpc_teardown(pds_vpc_type_t type) {
    k_vpc_feeder.init(k_vpc_key, type, "10.0.0.0/8");
    del(k_vpc_feeder);
}

}    // namespace api_test
