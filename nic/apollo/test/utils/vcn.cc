//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vcn test utility routines implementation
/// This file contains the vcn utility routines for testing
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/vcn.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

namespace api_test {

sdk::sdk_ret_t
vcn_util::create(oci_vcn_id_t vcn_id, std::string pfxstr,
                 oci_vcn_type_t vcn_type)
{
    oci_vcn_t oci_vcn;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)pfxstr.c_str(), &ip_pfx) == 0);
    memset(&oci_vcn, 0, sizeof(oci_vcn));
    oci_vcn.type = vcn_type;
    oci_vcn.key.id = vcn_id;
    oci_vcn.pfx = ip_pfx;
    oci_vcn.pfx.len = 8; // fix this to /8
    oci_vcn.pfx.addr.addr.v4_addr &= 0xFF000000;
    return (oci_vcn_create(&oci_vcn));
}

sdk::sdk_ret_t
vcn_util::many_create(uint32_t num_vcn, std::string pfxstr,
                      oci_vcn_type_t vcn_type)
{
    sdk::sdk_ret_t rv;
    SDK_ASSERT(num_vcn <= 1024);

    for (uint32_t idx = 1; idx <= num_vcn; idx++)
        if ((rv = create(idx, pfxstr, vcn_type)) != sdk::SDK_RET_OK)
            return rv;

    return sdk::SDK_RET_OK;
}

} // namespace api_test
