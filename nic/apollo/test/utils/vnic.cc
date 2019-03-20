//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vnic test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vnic.hpp"

namespace api_test {

vnic_util::vnic_util(uint32_t vnic_id, uint16_t vlan_tag,
                     uint32_t mpls_slot, uint64_t mac_u64) {
    this->vcn_id = 1;
    this->vnic_id = vnic_id;
    this->sub_id = 1;
    this->vlan_tag = vlan_tag;
    this->mpls_slot = mpls_slot;
    //this->src_dst_check = spec.src_dst_check;
    //this->rsc_pool_id = spec.rsc_pool_id;
    this->mac_u64 = mac_u64;
}

vnic_util::vnic_util() {
    this->vcn_id = PDS_MAX_VCN + 1;
    this->sub_id = PDS_MAX_SUBNET + 1;
    this->vnic_id = PDS_MAX_VNIC + 1;
    this->vlan_tag = -1;
    this->mpls_slot = -1;
    this->rsc_pool_id = -1;
    this->src_dst_check = false;
}

vnic_util::vnic_util(pds_vcn_id_t vcn_id, pds_vnic_id_t vnic_id,
                     pds_subnet_id_t sub_id, std::string vnic_mac,
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
    pds_vnic_spec_t spec = {0};

    spec.vcn.id = vcn_id;
    spec.subnet.id = sub_id;
    spec.key.id = vnic_id;
    spec.wire_vlan = vlan_tag;
    spec.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
    spec.fabric_encap.val.mpls_tag = mpls_slot;
    mac_str_to_addr((char *)vnic_mac.c_str(), spec.mac_addr);
    spec.rsc_pool_id = rsc_pool_id;
    spec.src_dst_check = src_dst_check;
    return pds_vnic_create(&spec);
}

sdk::sdk_ret_t
vnic_util::read(pds_vnic_info_t *info, bool compare_spec) {
    sdk_ret_t rv;
    pds_vnic_key_t key;

    key.id = vnic_id;
    memset(info, 0, sizeof(*info));
    rv = pds_vnic_read(&key, info);
    if (rv != SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {
#if 0
        mac_addr_t mac;
        mac_str_to_addr((char *)vnic_mac.c_str(), mac);
        SDK_ASSERT(vnic_id == info->spec.key.id);
        //SDK_ASSERT(vcn_id == info->spec.vcn.id);  //This is hw_id during read
        //SDK_ASSERT(sub_id == info->spec.subnet.id); //This is hw_id during read
        SDK_ASSERT(vlan_tag == info->spec.wire_vlan);
        SDK_ASSERT(PDS_ENCAP_TYPE_MPLSoUDP == info->spec.fabric_encap.type);
        SDK_ASSERT(mpls_slot == info->spec.fabric_encap.val.mpls_tag);
        SDK_ASSERT(memcmp(mac, info->spec.mac_addr, sizeof(mac)) == 0);
        SDK_ASSERT(rsc_pool_id == info->spec.rsc_pool_id);
        SDK_ASSERT(src_dst_check == info->spec.src_dst_check);
#endif
    }
    return rv;
}

sdk::sdk_ret_t
vnic_util::update(void) {
    // TODO
    return SDK_RET_ERR;
}

sdk::sdk_ret_t
vnic_util::del(void) {
    pds_vnic_key_t key;

    key.id = vnic_id;
    return pds_vnic_delete(&key);
}

static inline sdk::sdk_ret_t
vnic_util_object_stepper(vnic_minimal_key_t start_key, uint32_t num_vnics,
                         utils_op_t op,
                         sdk_ret_t expected_result = sdk::SDK_RET_OK)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_vnic_info_t info = {};
    uint32_t vlan_tag = start_key.vlan_tag;
    uint32_t mpls_slot = start_key.mpls_slot;;
    uint32_t mac_u64 = start_key.mac_u64;

    if (start_key.id == 0) start_key.id = 1;
    for (uint32_t idx = start_key.id; idx < start_key.id + num_vnics; idx++) {
        vnic_util vnic_obj(idx, vlan_tag, mpls_slot, mac_u64);
        switch (op) {
        case OP_MANY_CREATE:
            rv = vnic_obj.create();
            break;
        case OP_MANY_DELETE:
            rv = vnic_obj.del();
            break;
        case OP_MANY_READ:
            rv = vnic_obj.read(&info, FALSE);
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return rv;
        }
        vlan_tag++;
        mpls_slot++;
        mac_u64++;
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vnic_util::many_create(vnic_minimal_key_t key, uint32_t num_vnics) {
    return (vnic_util_object_stepper(key, num_vnics, OP_MANY_CREATE));
}

sdk::sdk_ret_t
vnic_util::many_read(pds_vnic_key_t key, uint32_t num_vnics,
                     sdk::sdk_ret_t expected_result) {
    vnic_minimal_key_t min_key = {};

    min_key.id = key.id;
    return (vnic_util_object_stepper(min_key, num_vnics, OP_MANY_READ,
                                     expected_result));

}

sdk::sdk_ret_t
vnic_util::many_delete(pds_vnic_key_t key, uint32_t num_vnics) {
    vnic_minimal_key_t min_key = {};

    min_key.id = key.id;
    return (vnic_util_object_stepper(min_key, num_vnics, OP_MANY_DELETE));
}


} // namespace api_test
