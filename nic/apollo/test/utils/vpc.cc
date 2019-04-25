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
vpc_util::create() {
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
vpc_util::read(pds_vpc_info_t *info, bool compare_spec)
{
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
vpc_util::update(pds_vpc_spec_t *vpc_spec)
{
    return (pds_vpc_update(vpc_spec));
}

sdk::sdk_ret_t
vpc_util::del() {
    pds_vpc_key_t key = {};
    key.id = this->id;
    return (pds_vpc_delete(&key));
}

static inline sdk::sdk_ret_t
vpc_util_object_stepper(pds_vpc_key_t start_key, std::string start_pfxstr,
                        uint32_t num_vpcs, utils_op_t op,
                        pds_vpc_type_t type, sdk_ret_t expected_result)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    ip_prefix_t ip_pfx = {0};
    ip_addr_t ipaddr_next;
    uint32_t addr = 0;
    pds_vpc_info_t info = {};

    if (start_key.id == 0) start_key.id = 1;
    if (op == OP_MANY_CREATE) {
        SDK_ASSERT(str2ipv4pfx((char *)start_pfxstr.c_str(), &ip_pfx) == 0);
        addr = ip_pfx.addr.addr.v4_addr;
    }
    for (uint32_t idx = start_key.id; idx < start_key.id + num_vpcs; idx++) {
        ip_pfx.addr.addr.v4_addr = addr;
        vpc_util vpc_obj(type, idx, ippfx2str(&ip_pfx));
        switch (op) {
        case OP_MANY_CREATE:
            rv = vpc_obj.create();
            break;
        case OP_MANY_DELETE:
            rv = vpc_obj.del();
            break;
        case OP_MANY_READ:
            rv = vpc_obj.read(&info, TRUE);
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return rv;
        }
        ip_prefix_ip_next(&ip_pfx, &ipaddr_next);
        addr = ipaddr_next.addr.v4_addr;
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vpc_util::many_create(pds_vpc_key_t start_key, std::string start_pfxstr,
                      uint32_t num_vpcs, pds_vpc_type_t type) {
    return (vpc_util_object_stepper(start_key, start_pfxstr, num_vpcs,
                                    OP_MANY_CREATE, PDS_VPC_TYPE_TENANT,
                                    sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
vpc_util::many_read(pds_vpc_key_t start_key, uint32_t num_vpcs,
                    sdk::sdk_ret_t expected_result) {
    return (vpc_util_object_stepper(start_key, "", num_vpcs,
                                    OP_MANY_READ, PDS_VPC_TYPE_TENANT,
                                    expected_result));
}

sdk::sdk_ret_t
vpc_util::many_delete(pds_vpc_key_t start_key, uint32_t num_vpcs) {
    return (vpc_util_object_stepper(start_key, "", num_vpcs,
                                    OP_MANY_DELETE, PDS_VPC_TYPE_TENANT,
                                    sdk::SDK_RET_OK));
}

}    // namespace api_test
