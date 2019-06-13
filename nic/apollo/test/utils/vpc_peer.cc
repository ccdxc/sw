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
#include "nic/apollo/test/utils/vpc_peer.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// VPC peer feeder class routines
//----------------------------------------------------------------------------

void
vpc_peer_feeder::init(pds_vpc_peer_key_t key, pds_vpc_key_t vpc1,
                      pds_vpc_key_t vpc2, uint32_t num_vpc_peer) {
    this->key.id = key.id;
    this->vpc1.id = vpc1.id;
    this->vpc2.id = vpc2.id;
    this->num_obj = num_vpc_peer;
}

void
vpc_peer_feeder::iter_next(int width) {
    key.id += width;
    vpc1.id = vpc2.id + width;
    vpc2.id = vpc1.id + width;
    cur_iter_pos++;
}

inline std::ostream&
operator<<(std::ostream& os, const vpc_peer_feeder& obj) {
    os << "VPC Peer feeder =>"
        << " id: " << obj.key.id
        << " vpc1 id: " << obj.vpc1.id
        << " vpc2 id: " << obj.vpc2.id
        << std::endl;
    return os;
}

void
vpc_peer_feeder::key_build(pds_vpc_peer_key_t *key) const {
    memset(key, 0, sizeof(pds_vpc_peer_key_t));
    key->id = this->key.id;
}

void
vpc_peer_feeder::spec_build(pds_vpc_peer_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_vpc_peer_spec_t));
    this->key_build(&spec->key);

    spec->key.id = key.id;
    spec->vpc1.id = vpc1.id;
    spec->vpc2.id = vpc2.id;
}

bool
vpc_peer_feeder::key_compare(const pds_vpc_peer_key_t *key) const {
    return true;
    // todo : @sai please check, compare routine not done
    // return (memcmp(key, &this->key, sizeof(pds_vpc_peer_key_t)) == 0);
}

bool
vpc_peer_feeder::spec_compare(const pds_vpc_peer_spec_t *spec) const {
    // todo : @sai please check, compare routine not done 
    return true;

    if (spec->vpc1.id != vpc1.id)
        return false;

    if (spec->vpc2.id != vpc2.id)
        return false;

    return true;
}

sdk::sdk_ret_t
vpc_peer_feeder::info_compare(const pds_vpc_peer_info_t *info) const {
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
// VPC peer test CRUD routines
//----------------------------------------------------------------------------

sdk::sdk_ret_t
create(vpc_peer_feeder& feeder) {
    pds_vpc_peer_spec_t spec;

    feeder.spec_build(&spec);
    return (pds_vpc_peer_create(&spec));
}

sdk::sdk_ret_t
read(vpc_peer_feeder& feeder) {
    sdk_ret_t rv;
    pds_vpc_peer_key_t key;
    pds_vpc_peer_info_t info;

    feeder.key_build(&key);
    if ((rv = pds_vpc_peer_read(&key, &info)) != sdk::SDK_RET_OK)
        return rv;

    return (feeder.info_compare(&info));
}

sdk::sdk_ret_t
update(vpc_peer_feeder& feeder) {
    pds_vpc_peer_spec_t spec;

    feeder.spec_build(&spec);
    // todo: @sai no update support yet
    //return (pds_vpc_peer_update(&spec));
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
del(vpc_peer_feeder& feeder) {
    pds_vpc_peer_key_t key;

    feeder.key_build(&key);
    return (pds_vpc_peer_delete(&key));
}

}    // namespace api_test
