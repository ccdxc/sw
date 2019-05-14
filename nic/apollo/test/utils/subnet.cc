//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the subnet test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include <iostream>

using namespace std;
namespace api_test {

subnet_util::subnet_util(pds_subnet_id_t id) {
    __init();
    this->id = id;
}

subnet_util::subnet_util(pds_vpc_id_t vpc_id, pds_subnet_id_t id,
                         std::string cidr_str) {
    __init();
    this->vpc.id = vpc_id;
    this->id = id;
    this->cidr_str = cidr_str;
}

subnet_util::subnet_util(subnet_util_stepper_seed_t *seed) {
    __init();
    this->vpc.id = seed->vpc.id;
    this->id = seed->key.id;
    this->cidr_str = seed->cidr_str;
    memcpy(&this->pfx, &seed->pfx, sizeof(ip_prefix_t));
    this->v4_route_table.id = seed->v4_route_table.id;
    this->v6_route_table.id = seed->v6_route_table.id;
    this->ing_v4_policy.id = seed->ing_v4_policy.id;
    this->ing_v6_policy.id = seed->ing_v6_policy.id;
    this->egr_v4_policy.id = seed->egr_v4_policy.id;
    this->egr_v6_policy.id = seed->egr_v6_policy.id;
}

subnet_util::subnet_util(pds_vpc_id_t vpc_id, pds_subnet_id_t id,
                         std::string cidr_str, uint32_t v4_route_table,
                         uint32_t v6_route_table, uint32_t ing_v4_policy,
                         uint32_t ing_v6_policy, uint32_t egr_v4_policy,
                         uint32_t  egr_v6_policy) {
    __init();
    this->vpc.id = vpc_id;
    this->id = id;
    this->cidr_str = cidr_str;
    this->v4_route_table.id = v4_route_table;
    this->v6_route_table.id = v6_route_table;
    this->ing_v4_policy.id = ing_v4_policy;
    this->ing_v6_policy.id = ing_v6_policy;
    this->egr_v4_policy.id = egr_v4_policy;
    this->egr_v6_policy.id = egr_v6_policy;
}

subnet_util::~subnet_util() {}


sdk::sdk_ret_t
subnet_util::create() {
    pds_subnet_spec_t spec;

    memset(&spec, 0, sizeof(pds_subnet_spec_t));
    spec.vpc.id = this->vpc.id;
    spec.key.id = this->id;
    spec.v4_pfx.len = this->pfx.len;
    spec.v4_pfx.v4_addr = this->pfx.addr.addr.v4_addr;;
    // Set the subnets IP (virtual router interface IP)
    if (!vr_ip.empty()) {
        extract_ipv4_addr(this->vr_ip.c_str(), &spec.v4_vr_ip);
    }
    // Derive mac address from vr_ip if it has not been configured
    if (vr_mac.empty()) {
        MAC_UINT64_TO_ADDR(spec.vr_mac,
                       (uint64_t)spec.v4_vr_ip);
    } else {
        mac_str_to_addr((char *)vr_mac.c_str(), spec.vr_mac);
    }
    spec.v4_route_table.id = this->v4_route_table.id;
    spec.v6_route_table.id = this->v6_route_table.id;
    spec.ing_v4_policy.id = this->ing_v4_policy.id;
    spec.ing_v6_policy.id = this->ing_v6_policy.id;
    spec.egr_v4_policy.id = this->egr_v4_policy.id;
    spec.egr_v6_policy.id = this->egr_v6_policy.id;

    return (pds_subnet_create(&spec));
}

sdk::sdk_ret_t
subnet_util::read(pds_subnet_info_t *info, bool compare_spec)
{
    sdk_ret_t rv;
    pds_subnet_key_t key;
    mac_addr_t vr_mac;

    memset(&key, 0, sizeof(pds_subnet_key_t));
    memset(info, 0, sizeof(pds_subnet_info_t));
    key.id = this->id;
    rv = pds_subnet_read(&key, info);
    if (rv != sdk::SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {
        if (this->vpc.id && memcmp(&info->spec.vpc, &this->vpc,
                                   sizeof(pds_vpc_key_t))) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.v4_route_table.id != this->v4_route_table.id) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.v6_route_table.id != this->v6_route_table.id) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.ing_v4_policy.id != this->ing_v4_policy.id) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.ing_v6_policy.id != this->ing_v6_policy.id) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.egr_v4_policy.id != this->egr_v4_policy.id) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.egr_v6_policy.id != this->egr_v6_policy.id) {
            return sdk::SDK_RET_ERR;
        }
        if (strcmp(this->vr_mac.c_str(), "") != 0) {
            mac_str_to_addr((char *)this->vr_mac.c_str(), vr_mac);
            if(memcmp(&info->spec.vr_mac, &this->vr_mac, sizeof(mac_addr_t))) {
                return sdk::SDK_RET_ERR;
            }
        }
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
subnet_util::del() {
    pds_subnet_key_t subnet_key = {};
    subnet_key.id = this->id;
    return (pds_subnet_delete(&subnet_key));
}

static inline void
subnet_util_stepper_seed_increment (subnet_util_stepper_seed_t *seed, int width)
{
    ip_addr_t ipaddr = {0};

    ip_prefix_ip_next(&seed->pfx, &ipaddr);
    memcpy(&seed->pfx.addr, &ipaddr, sizeof(ip_addr_t));
    seed->key.id += width;
    seed->v4_route_table.id += width;
    seed->v6_route_table.id += width;
    seed->ing_v4_policy.id += width;
    seed->ing_v6_policy.id += width;
    seed->egr_v4_policy.id += width;
    seed->egr_v6_policy.id += width;
}

sdk::sdk_ret_t
subnet_util::stepper_seed_init(subnet_util_stepper_seed_t *seed,
                               pds_subnet_key_t start_key,
                               pds_vpc_key_t vpc_key, std::string start_pfxstr)
{
    if (seed == NULL) {
        cout << "seed is NULL";
        return sdk::SDK_RET_ERR;
    }

    if (start_key.id == 0) start_key.id = 1;
    seed->key.id = start_key.id;
    seed->vpc.id = vpc_key.id;
    seed->v4_route_table.id = start_key.id;
    seed->v6_route_table.id = start_key.id + 1024; // Unique id, 1-1024 reserved
                                                   // for IPv4 rt table
    seed->ing_v4_policy.id = start_key.id;
    seed->ing_v6_policy.id = start_key.id + 1024;
    seed->egr_v4_policy.id = start_key.id + 2048;
    seed->egr_v6_policy.id = start_key.id + 3072;
    seed->cidr_str = start_pfxstr;
    SDK_ASSERT(str2ipv4pfx((char *)start_pfxstr.c_str(), &seed->pfx) == 0);

    return sdk::SDK_RET_OK;
}


static inline sdk::sdk_ret_t
subnet_util_object_stepper (subnet_util_stepper_seed_t *init_seed,
                            uint32_t num_objs,
                            utils_op_t op, sdk_ret_t expected_result)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    subnet_util_stepper_seed_t seed = {0};
    pds_subnet_info_t info = {};
    uint32_t start_key = init_seed->key.id;
    uint32_t width = 1;

    subnet_util::stepper_seed_init(&seed, init_seed->key, init_seed->vpc,
                                   ippfx2str(&init_seed->pfx));
    for (uint32_t idx = start_key;
         idx < start_key + num_objs; idx = idx + width) {
        subnet_util subnet_obj(&seed);
        switch (op) {
        case OP_MANY_CREATE:
            SDK_ASSERT((rv = subnet_obj.create()) == sdk::SDK_RET_OK);
            break;
        case OP_MANY_READ:
            rv = subnet_obj.read(&info, TRUE);
            break;
        case OP_MANY_UPDATE:
            // SDK_ASSERT((rv = subnet_obj.update()) == sdk::SDK_RET_OK);
            break;
        case OP_MANY_DELETE:
            rv = subnet_obj.del();
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return sdk::SDK_RET_ERR;
        }
        subnet_util_stepper_seed_increment(&seed, width);
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
subnet_util::many_create(subnet_util_stepper_seed_t *seed,
                         uint32_t num_subnets) {
    return (subnet_util_object_stepper(seed, num_subnets, OP_MANY_CREATE,
                                       sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
subnet_util::many_read(subnet_util_stepper_seed_t *seed, uint32_t num_subnets,
                       sdk::sdk_ret_t expected_result) {
    return (subnet_util_object_stepper(seed, num_subnets, OP_MANY_READ,
                                       expected_result));
}

sdk::sdk_ret_t
subnet_util::many_update(subnet_util_stepper_seed_t *seed,
                         uint32_t num_subnets) {
    return (subnet_util_object_stepper(seed, num_subnets, OP_MANY_UPDATE,
                                       sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
subnet_util::many_delete(subnet_util_stepper_seed_t *seed,
                         uint32_t num_subnets) {
    return (subnet_util_object_stepper(seed, num_subnets,
                                       OP_MANY_DELETE, sdk::SDK_RET_OK));
}

ostream& operator << (ostream& os, subnet_util& obj)
{
    os << "subnet id : " << obj.id << " vpc : " << obj.vpc.id <<
         " cidr_str : " << obj.cidr_str.c_str() << " vr_ip : " <<
         obj.vr_ip.c_str() << " vr_mac : " << obj.vr_mac.c_str() <<
         " v4_rt : " << obj.v4_route_table.id << " v6_rt : " <<
         obj.v6_route_table.id << " v4_in_pol : " <<
         obj.ing_v4_policy.id << " v6_in_pol : " << obj.ing_v6_policy.id <<
         " v4_eg_pol : " << obj.egr_v4_policy.id << " v6_eg_pol : " <<
         obj.egr_v6_policy.id;
    return os;
}

void subnet_util::__init()
{
    this->id = 0;
    this->cidr_str = "";
    this->vr_ip = "";
    this->vr_mac = "";
    memset(&this->vpc, 0, sizeof(pds_vpc_key_t));
    memset(&this->v4_route_table, 0, sizeof(pds_route_table_key_t));
    memset(&this->v6_route_table, 0, sizeof(pds_route_table_key_t));
    memset(&this->ing_v4_policy, 0, sizeof(pds_policy_key_t));
    memset(&this->ing_v6_policy, 0, sizeof(pds_policy_key_t));
    memset(&this->egr_v4_policy, 0, sizeof(pds_policy_key_t));
    memset(&this->egr_v6_policy, 0, sizeof(pds_policy_key_t));
}

}    // namespace api_test
