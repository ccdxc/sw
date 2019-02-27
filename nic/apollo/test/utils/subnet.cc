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

subnet_util::subnet_util(pds_vcn_id_t vcn_id, pds_subnet_id_t id,
                         std::string cidr_str) {
    this->vcn.id = vcn_id;
    this->id = id;
    this->cidr_str = cidr_str;
}

subnet_util::~subnet_util() {}

sdk::sdk_ret_t
subnet_util::create() {
    pds_subnet_spec_t pds_subnet;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->cidr_str.c_str(), &ip_pfx) == 0);
    memset(&pds_subnet, 0, sizeof(pds_subnet_spec_t));
    pds_subnet.vcn.id = this->vcn.id;
    pds_subnet.key.id = this->id;
    pds_subnet.pfx = ip_pfx;
    MAC_UINT64_TO_ADDR(pds_subnet.vr_mac,
                       (uint64_t)pds_subnet.vr_ip.addr.v4_addr);
    pds_subnet.v4_route_table.id = this->v4_route_table;
    pds_subnet.v6_route_table.id = this->v6_route_table;
    return (pds_subnet_create(&pds_subnet));
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
