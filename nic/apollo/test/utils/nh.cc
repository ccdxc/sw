//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the nh test utility routines implementation
///
//----------------------------------------------------------------------------

#include<iostream>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/nexthop_api.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/nh.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// VPC feeder class routines
//----------------------------------------------------------------------------

void
nh_feeder::init(std::string ip_str, uint64_t mac, uint32_t num_obj,
                pds_nexthop_id_t id, pds_nh_type_t type, uint16_t vlan,
                pds_vpc_id_t vpc_id) {
    this->id = id;
    this->type = type;
    extract_ip_addr(ip_str.c_str(), &this->ip);
    this->mac = mac;
    this->vlan = vlan;
    this->vpc_id = vpc_id;
    this->num_obj = num_obj;
}

void
nh_feeder::iter_next(int width) {
    ip.addr.v4_addr += width;
    mac += width;
    vlan += width;
    id += width;
    cur_iter_pos++;
}

void
nh_feeder::key_build(pds_nexthop_key_t *key) const {
    memset(key, 0, sizeof(pds_nexthop_key_t));
    *key = this->id;
}

void
nh_feeder::spec_build(pds_nexthop_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_nexthop_spec_t));
    this->key_build(&spec->key);

    spec->type = this->type;
    spec->vpc.id = this->vpc_id;
    spec->ip = this->ip;
    spec->vlan = this->vlan;
    MAC_UINT64_TO_ADDR(spec->mac, this->mac);
}

bool
nh_feeder::key_compare(const pds_nexthop_key_t *key) const {
    return (this->id == *key);
}

bool
nh_feeder::spec_compare(const pds_nexthop_spec_t *spec) const {
    ip_addr_t nh_ip = this->ip, spec_nh_ip = spec->ip;

    // validate NH type
    if (this->type != spec->type)
        return sdk::SDK_RET_ERR;

    // nothing much to check for blackhole nh
    if (this->type == PDS_NH_TYPE_BLACKHOLE)
        return sdk::SDK_RET_OK;

    // validate NH vlan
    if (this->vlan != spec->vlan)
        return sdk::SDK_RET_ERR;

    // validate NH MAC
    if (this->mac != MAC_TO_UINT64(spec->mac))
        return sdk::SDK_RET_ERR;

    if (this->type != PDS_NH_TYPE_IP) {
        // validate NH vpc
        if (this->vpc_id != spec->vpc.id)
            return sdk::SDK_RET_ERR;

        // validate NH ip
        if (!IPADDR_EQ(&nh_ip, &spec_nh_ip))
            return sdk::SDK_RET_ERR;
    }

    return true;
}

inline std::ostream&
operator<<(std::ostream& os, const nh_feeder *obj) {
    os << "NH feeder =>"
       << " id: " << obj->id
       << " type: " << obj->type
       << " ip: " << obj->ip
       << " mac: " << mac2str(obj->mac)
       << " vlan: " << obj->vlan
       << " vpc: " << obj->vpc_id
       << std::endl;
    return os;
}

sdk::sdk_ret_t
nh_feeder::info_compare(const pds_nexthop_info_t *info) const {
    if (!this->key_compare(&info->spec.key)) {
        std::cout << "key compare failed " << (nh_feeder *)this << info;
        return sdk::SDK_RET_ERR;
    }

    if (!this->spec_compare(&info->spec)) {
        std::cout << "spec compare failed " << (nh_feeder *)this << info;
        return sdk::SDK_RET_ERR;
    }

    return sdk::SDK_RET_OK;
}

//----------------------------------------------------------------------------
// NH test CRUD routines
//----------------------------------------------------------------------------

sdk::sdk_ret_t
create(nh_feeder& feeder) {
    pds_nexthop_spec_t spec;

    feeder.spec_build(&spec);
    return (pds_nexthop_create(&spec));
}

sdk::sdk_ret_t
read(nh_feeder& feeder) {
    sdk_ret_t rv;
    pds_nexthop_key_t key;
    pds_nexthop_info_t info;

    feeder.key_build(&key);
    memset(&info, 0, sizeof(pds_nexthop_info_t));
    if ((rv = pds_nexthop_read(&key, &info)) != sdk::SDK_RET_OK)
        return rv;

    return (feeder.info_compare(&info));
}

sdk::sdk_ret_t
update(nh_feeder& feeder) {
    pds_nexthop_spec_t spec;

    feeder.spec_build(&spec);
    return (pds_nexthop_update(&spec));
}

sdk::sdk_ret_t
del(nh_feeder& feeder) {
    pds_nexthop_key_t key;

    feeder.key_build(&key);
    return (pds_nexthop_delete(&key));
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static nh_feeder k_nh_feeder;

void sample_nh_setup(void) {
    // setup and teardown parameters should be in sync
    k_nh_feeder.init("30.30.30.1");
    many_create(k_nh_feeder);
}

void sample_nh_teardown(void) {
    k_nh_feeder.init("30.30.30.1");
    many_delete(k_nh_feeder);
}

}    // namespace api_test
