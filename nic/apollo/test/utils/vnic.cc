//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vnic test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

namespace api_test {

sdk::sdk_ret_t
vnic_util::create()
{
    oci_vnic_t vnic;

    vnic.vcn.id        = vcn_id;
    vnic.subnet.id     = sub_id;
    vnic.key.id        = vnic_id;
    vnic.wire_vlan     = vlan_tag;
    vnic.slot          = mpls_slot;
    mac_str_to_addr((char *)vnic_mac.c_str(), vnic.mac_addr);
    vnic.rsc_pool_id   = rsc_pool_id;
    vnic.src_dst_check = src_dst_check;
    return oci_vnic_create(&vnic);
}

sdk::sdk_ret_t
vnic_util::many_create(uint32_t num_vnics)
{
    sdk::sdk_ret_t rv;
    mac_addr_t mac;
    uint64_t mac_u64;

    mac_str_to_addr((char *)vnic_mac.c_str(), mac);
    mac_u64 = MAC_TO_UINT64(mac);

    for (uint32_t idx = 1; idx <= num_vnics; idx++) {
        MAC_UINT64_TO_ADDR(mac, mac_u64);
        vnic_mac = macaddr2str(mac);
        if ((rv = create()) != SDK_RET_OK) {
            return rv;
        }
        vnic_id++;
        vlan_tag++;
        mpls_slot++;
        mac_u64++;
    }
    return SDK_RET_OK;
}

} // namespace api_test
