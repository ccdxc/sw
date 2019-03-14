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

subnet_util::subnet_util(pds_vcn_id_t vcn_id, pds_subnet_id_t id,
                         std::string cidr_str) {
    this->vcn.id = vcn_id;
    this->id = id;
    this->cidr_str = cidr_str;
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
subnet_util::many_create(uint32_t num_subnet, pds_vcn_id_t vcn_id,
                         std::string pfxstr) {
#if 0
    sdk::sdk_ret_t rv;
    SDK_ASSERT(num_subnet <= 1024);

    for (uint32_t idx = 1; idx <= num_subnet; idx++)
        if ((rv = create()) != sdk::SDK_RET_OK)
            return rv;

#endif
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
subnet_util::del() {
    pds_subnet_key_t subnet_key = {};
    subnet_key.id = this->id;
    return (pds_subnet_delete(&subnet_key));
}

sdk::sdk_ret_t
subnet_util::get(pds_vcn_id_t vcn_id, pds_subnet_id_t subnet_id) {

    return sdk::SDK_RET_OK;
}

}    // namespace api_test
