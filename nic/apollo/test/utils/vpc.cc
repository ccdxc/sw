//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vpc test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "iostream"

using namespace std;
namespace api_test {

vpc_util::vpc_util(vpc_stepper_seed_t *seed) {
    this->type = seed->type;
    this->id = seed->key.id;
    this->cidr_str = ippfx2str(&seed->pfx);
}

vpc_util::vpc_util(pds_vpc_id_t id) {
    this->type = PDS_VPC_TYPE_TENANT;
    this->id = id;
    this->cidr_str = "";
}

vpc_util::vpc_util(pds_vpc_id_t id, std::string cidr_str) {
    this->type = PDS_VPC_TYPE_TENANT;
    this->id = id;
    this->cidr_str = cidr_str;
}

vpc_util::vpc_util(pds_vpc_type_t type, pds_vpc_id_t id, std::string cidr_str) {
    this->type = type;
    this->id = id;
    this->cidr_str = cidr_str;
}

vpc_util::~vpc_util() {}

sdk::sdk_ret_t
vpc_util::create(void) const {
    pds_vpc_spec_t spec;
    ip_prefix_t ip_pfx;

    extract_ip_pfx(this->cidr_str.c_str(), &ip_pfx);

    SDK_ASSERT(TRUE);
    memset(&spec, 0, sizeof(spec));
    spec.type = this->type;
    spec.key.id = this->id;
    spec.v4_pfx.len = ip_pfx.len;
    spec.v4_pfx.v4_addr = ip_pfx.addr.addr.v4_addr;
    return (pds_vpc_create(&spec));
}

sdk::sdk_ret_t
vpc_util::read(pds_vpc_info_t *info, bool compare_spec) const {
    sdk_ret_t rv;
    pds_vpc_key_t key;

    memset(&key, 0, sizeof(pds_vpc_key_t));
    memset(info, 0, sizeof(pds_vpc_info_t));
    key.id = this->id;
    rv = pds_vpc_read(&key, info);
    //cout << "vpc : key : " << key.id << ", id : " << info->spec.key.id << ", addr : "
         //<< ipv4pfx2str(&info->spec.v4_pfx) << "\n";
    if (rv != sdk::SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {

    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vpc_util::update(void) const {
    pds_vpc_spec_t spec;
    ip_prefix_t ip_pfx;

    extract_ip_pfx(this->cidr_str.c_str(), &ip_pfx);

    SDK_ASSERT(TRUE);
    memset(&spec, 0, sizeof(spec));
    spec.type = this->type;
    spec.key.id = this->id;
    spec.v4_pfx.len = ip_pfx.len;
    spec.v4_pfx.v4_addr = ip_pfx.addr.addr.v4_addr;
    return (pds_vpc_update(&spec));
}

sdk::sdk_ret_t
vpc_util::del(void) const {
    pds_vpc_key_t key = {};

    key.id = this->id;
    return (pds_vpc_delete(&key));
}

void
vpc_util::stepper_seed_init (vpc_stepper_seed_t *seed,
                             pds_vpc_key_t key,
                             pds_vpc_type_t type,
                             std::string start_pfx,
                             uint32_t num_vpcs) {
    if (seed == NULL) {
        cout << "vpc seed is NULL";
    }
    seed->key.id = key.id;
    seed->type = type;
    seed->num_vpcs = num_vpcs;
    SDK_ASSERT(str2ipv4pfx((char *)start_pfx.c_str(), &seed->pfx) == 0);
}

static inline void
vpc_stepper_seed_increment (vpc_stepper_seed_t *seed, int width)
{
    ip_addr_t ipaddr = {0};

    ip_prefix_ip_next(&seed->pfx, &ipaddr);
    memcpy(&seed->pfx.addr, &ipaddr, sizeof(ip_addr_t));
    seed->key.id += width;
}

static inline sdk::sdk_ret_t
vpc_util_object_stepper (vpc_stepper_seed_t *init_seed,
                         utils_op_t op, sdk_ret_t expected_result)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    vpc_stepper_seed_t seed = {0};
    pds_vpc_info_t info = {};
    uint32_t start_key = init_seed->key.id;
    uint32_t width = 1;
    uint32_t num_objs = init_seed->num_vpcs;

    vpc_util::stepper_seed_init(&seed, init_seed->key, init_seed->type,
                                ippfx2str(&init_seed->pfx),
                                init_seed->num_vpcs);

    for (uint32_t idx = start_key; idx < start_key + num_objs; idx++) {
        vpc_util vpc_obj(&seed);
        switch (op) {
        case OP_MANY_CREATE:
            rv = vpc_obj.create();
            break;
        case OP_MANY_READ:
            rv = vpc_obj.read(&info, TRUE);
            break;
        case OP_MANY_UPDATE:
            rv = vpc_obj.update();
            break;
        case OP_MANY_DELETE:
            rv = vpc_obj.del();
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return sdk::SDK_RET_ERR;
        }
        vpc_stepper_seed_increment(&seed, width);
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vpc_util::many_create(vpc_stepper_seed_t *seed) {
    return (vpc_util_object_stepper(seed, OP_MANY_CREATE, sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
vpc_util::many_read(vpc_stepper_seed_t *seed, sdk_ret_t expected_res) {
    return (vpc_util_object_stepper(seed, OP_MANY_READ, expected_res));
}

sdk::sdk_ret_t
vpc_util::many_update(vpc_stepper_seed_t *seed) {
    return (vpc_util_object_stepper(seed, OP_MANY_UPDATE, sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
vpc_util::many_delete(vpc_stepper_seed_t *seed) {
    return (vpc_util_object_stepper(seed, OP_MANY_DELETE, sdk::SDK_RET_OK));
}

}    // namespace api_test
