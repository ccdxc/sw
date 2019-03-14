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
    this->vcn_id = PDS_MAX_VCN + 1;
    this->sub_id = PDS_MAX_SUBNET + 1;
    this->mpls_slot = -1;
    this->vnic_id = PDS_MAX_VNIC + 1;
}

mapping_util::mapping_util(pds_vcn_id_t vcn_id, std::string vnic_rem_ip,
                           std::string vnic_rem_mac) {
    this->vcn_id = vcn_id;
    this->vnic_ip = vnic_rem_ip;
    this->vnic_mac = vnic_rem_mac;
}

mapping_util::mapping_util(pds_vcn_id_t vcn_id, std::string vnic_ip,
                           pds_vnic_id_t vnic_id, std::string vnic_mac) {
    this->vcn_id = vcn_id;
    this->vnic_ip = vnic_ip;
    this->vnic_mac = vnic_mac;
    this->vnic_id = vnic_id;
}

mapping_util::~mapping_util() {}

sdk::sdk_ret_t
mapping_util::create(void) {
    pds_mapping_spec_t spec = {0};
    struct in_addr ipaddr;

    spec.key.vcn.id = vcn_id;
    extract_ip_addr(vnic_ip.c_str(), &spec.key.ip_addr);
    spec.subnet.id     = sub_id;
    spec.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
    spec.fabric_encap.val.mpls_tag = mpls_slot;
    inet_aton(tep_ip.c_str(), &ipaddr);
    spec.tep.ip_addr = ntohl(ipaddr.s_addr);
    mac_str_to_addr((char *)vnic_mac.c_str(), spec.overlay_mac);
    spec.vnic.id         = vnic_id;
    spec.public_ip_valid = false;
    if (!public_ip.empty()) {
        extract_ip_addr(public_ip.c_str(), &spec.public_ip);
        spec.public_ip_valid = true;
    }
    return pds_mapping_create(&spec);
}

sdk::sdk_ret_t
mapping_util::read(pds_mapping_info_t *info, bool compare_spec) {
    sdk_ret_t rv;
    pds_mapping_key_t key = {0};

    key.vcn.id = vcn_id;
    extract_ip_addr(vnic_ip.c_str(), &key.ip_addr);
    memset(info, 0, sizeof(*info));
    rv = pds_mapping_read(&key, info);
    if (rv != SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {
        mac_addr_t mac;
        mac_str_to_addr((char *)vnic_mac.c_str(), mac);
        // SDK_ASSERT(vcn_id == info->spec.key.vcn.id); // Read returns hw_id
        // SDK_ASSERT(sub_id == info->spec.subnet.id); // This is hw_id during read
        SDK_ASSERT(strcmp(vnic_ip.c_str(), ipaddr2str(&info->spec.key.ip_addr)) == 0);
        SDK_ASSERT(PDS_ENCAP_TYPE_MPLSoUDP == info->spec.fabric_encap.type);
        SDK_ASSERT(mpls_slot == info->spec.fabric_encap.val.mpls_tag);
        SDK_ASSERT(memcmp(mac, info->spec.overlay_mac, sizeof(mac)) == 0);
        SDK_ASSERT(strcmp(tep_ip.c_str(), ipv4addr2str(info->spec.tep.ip_addr)) == 0);
        if (!public_ip.empty()) {
            SDK_ASSERT(true == info->spec.public_ip_valid);
            SDK_ASSERT(strcmp(public_ip.c_str(), ipaddr2str(&info->spec.public_ip)) == 0);
            // SDK_ASSERT(vnic_id == info->spec.vnic.id); // Read returns hw_id
        } else {
            SDK_ASSERT(false == info->spec.public_ip_valid);
        }
    }
    return SDK_RET_OK;
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
mapping_util::del(void) {
    pds_mapping_key_t key;

    key.vcn.id = vcn_id;
    extract_ip_addr(vnic_ip.c_str(), &key.ip_addr);
    return pds_mapping_delete(&key);
}

} // namespace api_test
