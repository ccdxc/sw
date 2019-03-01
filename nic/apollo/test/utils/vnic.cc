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

vnic_util::vnic_util() {
    this->vcn_id = PDS_MAX_VCN + 1;
    this->sub_id = PDS_MAX_SUBNET + 1;
    this->vnic_id = PDS_MAX_VNIC + 1;
    this->vlan_tag = -1;
    this->mpls_slot = -1;
    this->rsc_pool_id = -1;
    this->src_dst_check = false;
}

vnic_util::vnic_util(pds_vcn_id_t vcn_id, pds_subnet_id_t sub_id,
                     pds_vnic_id_t vnic_id, std::string vnic_mac,
                     bool src_dst_check) {
    this->vcn_id = vcn_id;
    this->sub_id = sub_id;
    this->vnic_id = vnic_id;
    this->vnic_mac = vnic_mac;
    this->src_dst_check = src_dst_check;
}

vnic_util::~vnic_util() {}

sdk::sdk_ret_t
vnic_util::create(void) {
    pds_vnic_spec_t vnic = {0};

    vnic.vcn.id = vcn_id;
    vnic.subnet.id = sub_id;
    vnic.key.id = vnic_id;
    vnic.wire_vlan = vlan_tag;
    vnic.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
    vnic.fabric_encap.val.mpls_tag = mpls_slot;
    mac_str_to_addr((char *)vnic_mac.c_str(), vnic.mac_addr);
    vnic.rsc_pool_id = rsc_pool_id;
    vnic.src_dst_check = src_dst_check;
    return pds_vnic_create(&vnic);
}

sdk::sdk_ret_t
vnic_util::read(pds_vnic_id_t vnic_id, pds_vnic_info_t *info) {
    pds_vnic_key_t key;

    key.id = vnic_id;
    memset(info, 0, sizeof(*info));
    return pds_vnic_read(&key, info);
}

sdk::sdk_ret_t
vnic_util::update(void) {
    // TODO
    return SDK_RET_ERR;
}

sdk::sdk_ret_t
vnic_util::remove(void) {
    // TODO
    return SDK_RET_ERR;
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
