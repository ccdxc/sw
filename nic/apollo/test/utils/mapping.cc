//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the local IP map test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/mapping.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

sdk::sdk_ret_t
mapping_util::create()
{
    oci_mapping_t map;
    struct in_addr ipaddr;

    map.key.vcn.id = vcn_id;
    extract_ip_addr(vnic_ip.c_str(), vnic_ip_af, &map.key.ip_addr);
    map.subnet.vcn_id = vcn_id;
    map.subnet.id     = sub_id;
    map.slot          = mpls_slot;
    inet_aton(tep_ip.c_str(), &ipaddr);
    map.tep.ip_addr = ntohl(ipaddr.s_addr);
    mac_str_to_addr((char *)vnic_mac.c_str(), map.overlay_mac);
    map.vnic.id         = vnic_id;
    map.public_ip_valid = false;
    if (!public_ip.empty()) {
        extract_ip_addr(public_ip.c_str(), public_ip_af, &map.public_ip);
        map.public_ip_valid = true;
    }
    return oci_mapping_create(&map);
}

sdk::sdk_ret_t
mapping_util::many_create(uint32_t num_mappings)
{
    sdk::sdk_ret_t rv;

    for (uint32_t idx = 1; idx <= num_mappings; idx++) {
        if ((rv = create()) != SDK_RET_OK) {
            return rv;
        }
        // TODO increment IP adddress, v4 and v6
    }
    return SDK_RET_OK;
}

#if 0
sdk::sdk_ret_t
mapping_util::destroy()
{
    oci_mapping_key_t key;

    key.vcn.id = vcn_id;
    extract_ip_addr(vnic_ip.c_str(), vnic_ip_af, &key.ip_addr);
    return oci_mapping_delete(&key);
    return SDK_RET_OK;
}
#endif // TODO

} // namespace api_test
