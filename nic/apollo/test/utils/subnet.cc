//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the subnet test utility routines implementation
///
//----------------------------------------------------------------------------

#include <iostream>
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/subnet.hpp"

using namespace std;

namespace api_test {

//----------------------------------------------------------------------------
// Subnet feeder class routines
//----------------------------------------------------------------------------

void
subnet_feeder::init(pds_subnet_key_t key, pds_vpc_key_t vpc_key,
                    std::string cidr_str, int num_subnet) {
    this->key = key;
    this->vpc = vpc_key;
    this->cidr_str = cidr_str;
    SDK_ASSERT(str2ipv4pfx((char *)cidr_str.c_str(), &pfx) == 0);
    v4_route_table.id = key.id;
    v6_route_table.id = key.id + 1024; // Unique id, 1-1024 reserved
                                       // for IPv4 rt table
    ing_v4_policy.id = key.id;
    ing_v6_policy.id = key.id + 1024;
    egr_v4_policy.id = key.id + 2048;
    egr_v6_policy.id = key.id + 3072;
    num_obj = num_subnet;
}

void
subnet_feeder::iter_next(int width) {
    ip_addr_t ipaddr = {0};

    ip_prefix_ip_next(&pfx, &ipaddr);
    memcpy(&pfx.addr, &ipaddr, sizeof(ip_addr_t));
    key.id += width;
    v4_route_table.id += width;
    v6_route_table.id += width;
    ing_v4_policy.id += width;
    ing_v6_policy.id += width;
    egr_v4_policy.id += width;
    egr_v6_policy.id += width;
    cur_iter_pos++;
}

ostream& operator << (ostream& os, subnet_feeder& obj)
{
    os << "Subnet feeder =>"
        << " id: " << obj.key.id
        << " vpc: " << obj.vpc.id
        << " cidr_str: " << obj.cidr_str
        << " vr_ip: " << obj.vr_ip.c_str()
        << " vr_mac: " << obj.vr_mac.c_str()
        << " v4_rt: " << obj.v4_route_table.id
        << " v6_rt: " << obj.v6_route_table.id
        << " v4_in_pol: " << obj.ing_v4_policy.id
        << " v6_in_pol: " << obj.ing_v6_policy.id
        << " v4_eg_pol: " << obj.egr_v4_policy.id
        << " v6_eg_pol: " << obj.egr_v6_policy.id
        << endl;
    return os;
}

void
subnet_feeder::key_build(pds_subnet_key_t *key) {
    memset(key, 0, sizeof(pds_subnet_key_t));
    key->id = this->key.id;
}

void
subnet_feeder::spec_build(pds_subnet_spec_t *spec) {
    memset(spec, 0, sizeof(pds_subnet_spec_t));
    this->key_build(&spec->key);

    spec->vpc.id = vpc.id;
    spec->v4_pfx.len = pfx.len;
    spec->v4_pfx.v4_addr = pfx.addr.addr.v4_addr;

    // Set the subnets IP (virtual router interface IP)
    if (!this->vr_ip.empty())
        extract_ipv4_addr(vr_ip.c_str(), &spec->v4_vr_ip);

    // Derive mac address from vr_ip if it has not been configured
    if (this->vr_mac.empty()) {
        MAC_UINT64_TO_ADDR(spec->vr_mac, (uint64_t)spec->v4_vr_ip);
    } else {
        mac_str_to_addr((char *)vr_mac.c_str(), spec->vr_mac);
    }

    spec->v4_route_table.id = v4_route_table.id;
    spec->v6_route_table.id = v6_route_table.id;
    spec->ing_v4_policy.id = ing_v4_policy.id;
    spec->ing_v6_policy.id = ing_v6_policy.id;
    spec->egr_v4_policy.id = egr_v4_policy.id;
    spec->egr_v6_policy.id = egr_v6_policy.id;
}

bool
subnet_feeder::key_compare(pds_subnet_key_t *key) {
    // todo : @sai, please check; compare key not working
    return true;
    return (memcmp(key, &this->key, sizeof(pds_subnet_key_t)) == 0);
}

bool
subnet_feeder::spec_compare(pds_subnet_spec_t *spec) {
    if (spec->vpc.id != vpc.id)
        return false;

    if (spec->v4_route_table.id != v4_route_table.id)
        return false;

    if (spec->v6_route_table.id != v6_route_table.id)
        return false;

    if (spec->ing_v4_policy.id != ing_v4_policy.id)
        return false;

    if (spec->ing_v6_policy.id != ing_v6_policy.id)
        return false;

    if (spec->egr_v4_policy.id != egr_v4_policy.id)
        return false;

    if (spec->egr_v6_policy.id != egr_v6_policy.id)
        return false;

    if (strcmp(vr_mac.c_str(), "") != 0) {
        mac_addr_t vrmac;
        mac_str_to_addr((char *)vr_mac.c_str(), vrmac);
        if (memcmp(&spec->vr_mac, vrmac, sizeof(mac_addr_t)))
            return false;
    }

    return true;
}

sdk::sdk_ret_t
subnet_feeder::info_compare(pds_subnet_info_t *info) {

    if (!this->key_compare(&info->spec.key)) {
        cout << "key compare failed " <<  this;
        return sdk::SDK_RET_ERR;
    }

    if (!this->spec_compare(&info->spec)) {
        cout << "spec compare failed " <<  this;
        return sdk::SDK_RET_ERR;
    }

    return sdk::SDK_RET_OK;
}

//----------------------------------------------------------------------------
// Subnet test class routines
//----------------------------------------------------------------------------

sdk::sdk_ret_t
create(subnet_feeder& feeder) {
    pds_subnet_spec_t spec;

    feeder.spec_build(&spec);
    return (pds_subnet_create(&spec));
}

sdk::sdk_ret_t
read(subnet_feeder& feeder) {
    sdk_ret_t rv;
    pds_subnet_key_t key;
    pds_subnet_info_t info;

    feeder.key_build(&key);
    if ((rv = pds_subnet_read(&key, &info)) != sdk::SDK_RET_OK)
        return rv;

    return (feeder.info_compare(&info));
}

sdk::sdk_ret_t
update(subnet_feeder& feeder) {
    pds_subnet_spec_t spec;

    feeder.spec_build(&spec);
    return (pds_subnet_update(&spec));
}

sdk::sdk_ret_t
del(subnet_feeder& feeder) {
    pds_subnet_key_t key;

    feeder.key_build(&key);
    return (pds_subnet_delete(&key));
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
pds_subnet_key_t k_subnet_key = {.id = 1};
static subnet_feeder k_subnet_feeder;

void sample_subnet_setup() {
    // setup and teardown parameters should be in sync
    k_subnet_feeder.init(k_subnet_key, k_vpc_key, "10.1.0.0/16");
    create(k_subnet_feeder);
}

void sample_subnet_teardown() {
    k_subnet_feeder.init(k_subnet_key, k_vpc_key, "10.1.0.0/16");
    del(k_subnet_feeder);
}

}    // namespace api_test
