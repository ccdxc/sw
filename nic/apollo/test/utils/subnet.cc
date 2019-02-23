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

namespace api_test {

subnet_util::subnet_util(oci_vcn_id_t vcn_id, oci_subnet_id_t id,
                         std::string cidr_str) {
    this->vcn.id = vcn_id;
    this->id = id;
    this->cidr_str = cidr_str;
}

subnet_util::~subnet_util() {}

sdk::sdk_ret_t
subnet_util::create() {
    oci_subnet_spec_t oci_subnet;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->cidr_str.c_str(), &ip_pfx) == 0);
    memset(&oci_subnet, 0, sizeof(oci_subnet_spec_t));
    oci_subnet.vcn.id = this->vcn.id;
    oci_subnet.key.id = this->id;
    oci_subnet.pfx = ip_pfx;
    MAC_UINT64_TO_ADDR(oci_subnet.vr_mac,
                       (uint64_t)oci_subnet.vr_ip.addr.v4_addr);
    oci_subnet.v4_route_table.id = this->v4_route_table;
    oci_subnet.v6_route_table.id = this->v6_route_table;
    return (oci_subnet_create(&oci_subnet));
}

sdk::sdk_ret_t
subnet_util::many_create(uint32_t num_subnet, oci_vcn_id_t vcn_id,
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
    oci_subnet_key_t subnet_key = {};
    subnet_key.id = this->id;
    return (oci_subnet_delete(&subnet_key));
}

sdk::sdk_ret_t
subnet_util::get(oci_vcn_id_t vcn_id, oci_subnet_id_t subnet_id) {

    return sdk::SDK_RET_OK;
}

}    // namespace api_test
