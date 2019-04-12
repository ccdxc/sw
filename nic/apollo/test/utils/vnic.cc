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

vnic_util::vnic_util(uint32_t vnic_id, pds_encap_t host_encap,
                     pds_encap_t fabric_encap, uint64_t mac_u64) {
    mac_addr_t mac;
    this->vcn_id = 1;
    this->vnic_id = vnic_id;
    this->sub_id = 1;
    this->host_encap = host_encap;
    this->fabric_encap = fabric_encap;
    this->mac_u64 = mac_u64;
    this->rsc_pool_id = 0;
    this->src_dst_check = true;;
    MAC_UINT64_TO_ADDR(mac, this->mac_u64);
    this->vnic_mac = macaddr2str(mac);
}

vnic_util::vnic_util() {
    this->vcn_id = PDS_MAX_VCN + 1;
    this->sub_id = PDS_MAX_SUBNET + 1;
    this->vnic_id = PDS_MAX_VNIC + 1;
    this->host_encap.type = PDS_ENCAP_TYPE_DOT1Q;
    this->host_encap.val.value = -1;
    this->fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
    this->fabric_encap.val.value = -1;
    this->rsc_pool_id = 0;
    this->src_dst_check = true;
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
    spec.host_encap = host_encap;
    spec.fabric_encap = fabric_encap;
    if (mac_u64 != 0) {
        MAC_UINT64_TO_ADDR(spec.mac_addr, mac_u64);
    } else if (!vnic_mac.empty()) {
        mac_str_to_addr((char *)vnic_mac.c_str(), spec.mac_addr);
    }
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
        mac_addr_t mac = {0};
        SDK_ASSERT(vnic_id == info->spec.key.id);
        //SDK_ASSERT(vcn_id == info->spec.vcn.id);  //This is hw_id during read
        //SDK_ASSERT(sub_id == info->spec.subnet.id); //This is hw_id during read
        SDK_ASSERT(this->host_encap.type == info->spec.host_encap.type);
        SDK_ASSERT(this->fabric_encap.type == info->spec.fabric_encap.type);
        if (this->host_encap.type == PDS_ENCAP_TYPE_DOT1Q) {
            SDK_ASSERT(this->host_encap.val.vlan_tag ==
                       info->spec.host_encap.val.vlan_tag);
        } else if (this->host_encap.type == PDS_ENCAP_TYPE_QINQ) {
            SDK_ASSERT(this->host_encap.val.qinq_tag.c_tag ==
                       info->spec.host_encap.val.qinq_tag.c_tag);
            SDK_ASSERT(this->host_encap.val.qinq_tag.s_tag ==
                       info->spec.host_encap.val.qinq_tag.s_tag);
        }
        SDK_ASSERT(this->fabric_encap.val.mpls_tag == info->spec.fabric_encap.val.mpls_tag);
        if (mac_u64 != 0) {
            MAC_UINT64_TO_ADDR(mac, mac_u64);
        } else if (!vnic_mac.empty()) {
            mac_str_to_addr((char *)vnic_mac.c_str(), mac);
        }
        SDK_ASSERT(memcmp(mac, info->spec.mac_addr, sizeof(mac)) == 0);
        SDK_ASSERT(rsc_pool_id == info->spec.rsc_pool_id);
        SDK_ASSERT(src_dst_check == info->spec.src_dst_check);
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
vnic_util_object_stepper(vnic_stepper_seed_t *seed, uint32_t num_vnics,
                         utils_op_t op,
                         sdk_ret_t expected_result = sdk::SDK_RET_OK)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_vnic_info_t info = {};
    pds_encap_t host_encap = seed->host_encap;
    pds_encap_t fabric_encap = seed->fabric_encap;
    uint64_t mac_u64 = seed->mac_u64;

    if (seed->id == 0) seed->id = 1;
    for (uint32_t idx = seed->id; idx < seed->id + num_vnics; idx++) {
        vnic_util vnic_obj(idx, host_encap, fabric_encap, mac_u64);
        switch (op) {
        case OP_MANY_CREATE:
            rv = vnic_obj.create();
            break;
        case OP_MANY_DELETE:
            rv = vnic_obj.del();
            break;
        case OP_MANY_READ:
            rv = vnic_obj.read(&info, TRUE);
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return rv;
        }
        if (host_encap.type == PDS_ENCAP_TYPE_DOT1Q)
            host_encap.val.vlan_tag++;
        else if (host_encap.type == PDS_ENCAP_TYPE_QINQ) {
            host_encap.val.qinq_tag.c_tag++;
            host_encap.val.qinq_tag.s_tag++;
        }
        fabric_encap.val.mpls_tag++;
        mac_u64++;
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vnic_util::many_create(vnic_stepper_seed_t *seed, uint32_t num_vnics) {
    return (vnic_util_object_stepper(seed, num_vnics, OP_MANY_CREATE));
}

sdk::sdk_ret_t
vnic_util::many_read(vnic_stepper_seed_t *seed, uint32_t num_vnics,
                     sdk::sdk_ret_t exp_result) {
    return (vnic_util_object_stepper(seed, num_vnics, OP_MANY_READ,
                                     exp_result));
}

sdk::sdk_ret_t
vnic_util::many_delete(vnic_stepper_seed_t *seed, uint32_t num_vnics) {
    return (vnic_util_object_stepper(seed, num_vnics, OP_MANY_DELETE));
}

} // namespace api_test
