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

mapping_util::mapping_util() {
    this->vcn_id = OCI_MAX_VCN + 1;
    this->sub_id = OCI_MAX_SUBNET + 1;
    this->mpls_slot = -1;
    this->vnic_id = OCI_MAX_VNIC + 1;
}

mapping_util::mapping_util(oci_vcn_id_t vcn_id, std::string vnic_ip,
                           std::string vnic_mac, uint8_t vnic_ip_af) {
    this->vcn_id = vcn_id;
    this->vnic_ip = vnic_ip;
    this->vnic_mac = vnic_mac;
    this->vnic_ip_af = vnic_ip_af;
}

mapping_util::mapping_util(oci_vcn_id_t vcn_id, std::string vnic_ip,
                           std::string vnic_mac, oci_vnic_id_t vnic_id,
                           uint8_t vnic_ip_af) {
    this->vcn_id = vcn_id;
    this->vnic_ip = vnic_ip;
    this->vnic_mac = vnic_mac;
    this->vnic_ip_af = vnic_ip_af;
    this->vnic_id = vnic_id;
}

mapping_util::~mapping_util() {}

sdk::sdk_ret_t
mapping_util::create(void) {
    oci_mapping_spec_t map = {0};
    struct in_addr ipaddr;

    map.key.vcn.id = vcn_id;
    extract_ip_addr(vnic_ip.c_str(), vnic_ip_af, &map.key.ip_addr);
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
mapping_util::read(oci_vcn_id_t vcn_id, std::string vnic_ip, uint8_t vnic_ip_af,
                   oci_mapping_info_t *info) {
    oci_mapping_key_t key = {0};

    key.vcn.id = vcn_id;
    extract_ip_addr(vnic_ip.c_str(), vnic_ip_af, &key.ip_addr);
    memset(info, 0, sizeof(*info));
    return oci_mapping_read(&key, info);
}

sdk::sdk_ret_t
mapping_util::update(void) {
    // TODO
    return SDK_RET_ERR;
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

sdk::sdk_ret_t
mapping_util::remove(void) {
#if 0 // TODO
    oci_mapping_key_t key;

    key.vcn.id = vcn_id;
    extract_ip_addr(vnic_ip.c_str(), vnic_ip_af, &key.ip_addr);
    return oci_mapping_delete(&key);
#endif
    return SDK_RET_ERR;
}

} // namespace api_test
