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

namespace api_test {

subnet_util::subnet_util(pds_subnet_id_t subnet_id) {
    this->id = id;
    this->cidr_str = "";
}

subnet_util::subnet_util(pds_vcn_id_t vcn_id, pds_subnet_id_t id,
                         std::string cidr_str) {
    this->vcn.id = vcn_id;
    this->id = id;
    this->cidr_str = cidr_str;
}

subnet_util::subnet_util(pds_vcn_id_t vcn_id, pds_subnet_id_t id,
                         std::string cidr_str, uint32_t v4_route_table,
                         uint32_t v6_route_table) {
    this->vcn.id = vcn_id;
    this->id = id;
    this->cidr_str = cidr_str;
    this->v4_route_table = v4_route_table;
    this->v6_route_table = v6_route_table;
}

subnet_util::~subnet_util() {}

sdk::sdk_ret_t
subnet_util::create() {
    pds_subnet_spec_t spec;
    ip_prefix_t ip_pfx;

    extract_ip_pfx(this->cidr_str.c_str(), &ip_pfx);
    memset(&spec, 0, sizeof(pds_subnet_spec_t));
    spec.vcn.id = this->vcn.id;
    spec.key.id = this->id;
    spec.pfx = ip_pfx;
    // Set the subnets IP (virtual router interface IP)
    if (!vr_ip.empty()) {
        extract_ip_addr(this->vr_ip.c_str(), &spec.vr_ip);
    }
    // Derive mac address from vr_ip if it has not been configured
    if (vr_mac.empty()) {
        MAC_UINT64_TO_ADDR(spec.vr_mac,
                       (uint64_t)spec.vr_ip.addr.v4_addr);
    } else {
        mac_str_to_addr((char *)vr_mac.c_str(), spec.vr_mac);
    }
    spec.v4_route_table.id = this->v4_route_table;
    spec.v6_route_table.id = this->v6_route_table;
    return (pds_subnet_create(&spec));
}

sdk::sdk_ret_t
subnet_util::read(pds_subnet_info_t *info, bool compare_spec)
{
    sdk_ret_t rv;
    pds_subnet_key_t key;

    memset(&key, 0, sizeof(pds_subnet_key_t));
    memset(info, 0, sizeof(pds_subnet_info_t));
    key.id = info->spec.key.id;
    rv = pds_subnet_read(&key, info);
    if (rv != sdk::SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {

    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
subnet_util::del() {
    pds_subnet_key_t subnet_key = {};
    subnet_key.id = this->id;
    return (pds_subnet_delete(&subnet_key));
}

static inline sdk::sdk_ret_t
subnet_util_object_stepper (pds_subnet_key_t start_key, pds_vcn_key_t vcn_key,
                            std::string start_pfxstr, uint32_t num_objs,
                            utils_op_t op, sdk_ret_t expected_result)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    ip_prefix_t ip_pfx = {};
    pds_subnet_info_t info = {};
    uint32_t addr;

    if (start_key.id == 0) start_key.id = 1;
    if (op == OP_MANY_CREATE) {
        SDK_ASSERT(str2ipv4pfx((char *)start_pfxstr.c_str(), &ip_pfx) == 0);
        addr = ip_pfx.addr.addr.v4_addr;
    }
    for (uint32_t idx = start_key.id; idx <= start_key.id + num_objs; idx++) {
        subnet_util subnet_obj(idx, vcn_key.id, ippfx2str(&ip_pfx));
        ip_pfx.addr.addr.v4_addr = addr;
        switch (op) {
        case OP_MANY_CREATE:
            SDK_ASSERT((rv = subnet_obj.create()) == sdk::SDK_RET_OK);
            break;
        case OP_MANY_DELETE:
            rv = subnet_obj.del();
            break;
        case OP_MANY_READ:
            rv = subnet_obj.read(&info, TRUE);
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return rv;
        }
        addr = api_test::pds_get_next_addr16(addr);
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
subnet_util::many_create(pds_subnet_key_t start_key, pds_vcn_key_t vcn_key,
                         std::string start_pfxstr, uint32_t num_subnets) {
    return (subnet_util_object_stepper(start_key, vcn_key, start_pfxstr,
                                       num_subnets, OP_MANY_CREATE,
                                       sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
subnet_util::many_read(pds_subnet_key_t start_key, uint32_t num_subnets,
                       sdk::sdk_ret_t expected_result) {
    pds_vcn_key_t vcn_key = {};

    return (subnet_util_object_stepper(start_key, vcn_key, "", num_subnets,
                                       OP_MANY_READ, expected_result));
}

sdk::sdk_ret_t
subnet_util::many_delete(pds_subnet_key_t start_key, uint32_t num_subnets) {
    pds_vcn_key_t vcn_key = {};

    return (subnet_util_object_stepper(start_key, vcn_key, "", num_subnets,
                                       OP_MANY_DELETE, sdk::SDK_RET_OK));
}

}    // namespace api_test
