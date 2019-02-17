//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the subnet test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

namespace api_test {

sdk::sdk_ret_t
subnet_util::create(oci_vcn_id_t vcn_id, oci_subnet_id_t subnet_id,
                    std::string pfxstr)
{
    oci_subnet_t oci_subnet;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)pfxstr.c_str(), &ip_pfx) == 0);
    memset(&oci_subnet, 0, sizeof(oci_subnet_t));
    oci_subnet.key.vcn_id = vcn_id;
    oci_subnet.key.id     = subnet_id;
    oci_subnet.pfx        = ip_pfx;
    MAC_UINT64_TO_ADDR(oci_subnet.vr_mac,
                       (uint64_t)oci_subnet.vr_ip.addr.v4_addr);
    oci_subnet.route_table.id = subnet_id;
    return (oci_subnet_create(&oci_subnet));
}

sdk::sdk_ret_t
subnet_util::many_create(uint32_t num_subnet, oci_vcn_id_t vcn_id,
                         std::string pfxstr)
{
    sdk::sdk_ret_t rv;
    SDK_ASSERT(num_subnet <= 1024);

    for (uint32_t idx = 1; idx <= num_subnet; idx++)
        // TODO increment prefix, v4 and v6
        if ((rv = create(vcn_id, idx, pfxstr)) != sdk::SDK_RET_OK)
            return rv;

    return sdk::SDK_RET_OK;
}

} // namespace api_test
