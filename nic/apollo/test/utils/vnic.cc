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
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vnic.hpp"

namespace api_test {

vnic_util::vnic_util(uint32_t vnic_id, pds_encap_t vnic_encap,
                     pds_encap_t fabric_encap, uint64_t mac_u64,
                     bool src_dst_check) {
    mac_addr_t mac;
    this->vpc_id = 1;
    this->vnic_id = vnic_id;
    this->sub_id = 1;
    this->vnic_encap = vnic_encap;
    this->fabric_encap = fabric_encap;
    this->mac_u64 = mac_u64;
    this->rsc_pool_id = 0;
    this->src_dst_check = src_dst_check;
    MAC_UINT64_TO_ADDR(mac, this->mac_u64);
    this->vnic_mac = macaddr2str(mac);
}

vnic_util::vnic_util() {
    this->vpc_id = PDS_MAX_VPC + 1;
    this->sub_id = PDS_MAX_SUBNET + 1;
    this->vnic_id = PDS_MAX_VNIC + 1;
    this->vnic_encap.type = PDS_ENCAP_TYPE_DOT1Q;
    this->vnic_encap.val.value = -1;
    this->fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
    this->fabric_encap.val.value = -1;
    this->rsc_pool_id = 0;
    this->src_dst_check = true;
}

vnic_util::vnic_util(pds_vpc_id_t vpc_id, pds_vnic_id_t vnic_id,
                     pds_subnet_id_t sub_id, std::string vnic_mac,
                     bool src_dst_check) {
    this->vpc_id = vpc_id;
    this->sub_id = sub_id;
    this->vnic_id = vnic_id;
    this->vnic_mac = vnic_mac;
    this->src_dst_check = src_dst_check;
}

vnic_util::~vnic_util() {}

sdk::sdk_ret_t
vnic_util::create(void) const {
    pds_vnic_spec_t spec = {0};

    spec.vpc.id = vpc_id;
    spec.subnet.id = sub_id;
    spec.key.id = vnic_id;
    spec.vnic_encap = vnic_encap;
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
vnic_util::read(pds_vnic_info_t *info) const {
    sdk_ret_t rv;
    pds_vnic_key_t key;

    key.id = vnic_id;
    memset(info, 0, sizeof(*info));
    if ((rv = pds_vnic_read(&key, info)) != SDK_RET_OK)
        return rv;

    mac_addr_t mac = {0};
    if (vnic_id != info->spec.key.id)
        return sdk::SDK_RET_ERR;

    //SDK_ASSERT(vpc_id == info->spec.vpc.id);  //This is hw_id during read
    //SDK_ASSERT(sub_id == info->spec.subnet.id); //This is hw_id during read
    if (!api::pdsencap_isequal(&this->vnic_encap, &info->spec.vnic_encap))
        return sdk::SDK_RET_ERR;

    if (!api::pdsencap_isequal(&this->fabric_encap, &info->spec.fabric_encap))
        return sdk::SDK_RET_ERR;

    if (rsc_pool_id != info->spec.rsc_pool_id)
        return sdk::SDK_RET_ERR;

    if (this->src_dst_check != info->spec.src_dst_check)
        return sdk::SDK_RET_ERR;

    if (mac_u64 != 0) {
        MAC_UINT64_TO_ADDR(mac, mac_u64);
    } else if (!vnic_mac.empty()) {
        mac_str_to_addr((char *)vnic_mac.c_str(), mac);
    }

    if (memcmp(mac, info->spec.mac_addr, sizeof(mac)))
        return sdk::SDK_RET_ERR;

    return rv;
}

sdk::sdk_ret_t
vnic_util::update(void) const {
    pds_vnic_spec_t spec = {0};

    spec.vpc.id = vpc_id;
    spec.subnet.id = sub_id;
    spec.key.id = vnic_id;
    spec.vnic_encap = vnic_encap;
    spec.fabric_encap = fabric_encap;
    if (mac_u64 != 0) {
        MAC_UINT64_TO_ADDR(spec.mac_addr, mac_u64);
    } else if (!vnic_mac.empty()) {
        mac_str_to_addr((char *)vnic_mac.c_str(), spec.mac_addr);
    }
    spec.rsc_pool_id = rsc_pool_id;
    spec.src_dst_check = src_dst_check;
    return pds_vnic_update(&spec);
}

sdk::sdk_ret_t
vnic_util::del(void) const {
    pds_vnic_key_t key;

    key.id = vnic_id;
    return pds_vnic_delete(&key);
}

static inline void
vnic_util_encap_value_stepper(pds_encap_t *encap)
{
    switch (encap->type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        encap->val.vlan_tag++;
        break;
    case PDS_ENCAP_TYPE_QINQ:
        encap->val.qinq_tag.c_tag++;
        encap->val.qinq_tag.s_tag++;
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        encap->val.mpls_tag++;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        encap->val.vnid++;
        break;
    default:
        encap->val.value++;
        break;
    }
}

static inline sdk::sdk_ret_t
vnic_util_object_stepper(vnic_stepper_seed_t *seed, utils_op_t op,
                         sdk_ret_t expected_result = sdk::SDK_RET_OK)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_vnic_info_t info = {};
    pds_encap_t vnic_encap = seed->vnic_encap;
    pds_encap_t fabric_encap = seed->fabric_encap;
    uint64_t mac_u64 = seed->mac_u64;
    bool src_dst_check = seed->src_dst_check;

    if (seed->id == 0) seed->id = 1;
    for (uint32_t idx = seed->id; idx < seed->id + seed->num_vnics; idx++) {
        vnic_util vnic_obj(idx, vnic_encap, fabric_encap,
                           mac_u64, src_dst_check);
        switch (op) {
        case OP_MANY_CREATE:
            rv = vnic_obj.create();
            break;
        case OP_MANY_READ:
            rv = vnic_obj.read(&info);
            break;
        case OP_MANY_UPDATE:
            rv = vnic_obj.update();
            break;
        case OP_MANY_DELETE:
            rv = vnic_obj.del();
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return sdk::SDK_RET_ERR;
        }
        vnic_util_encap_value_stepper(&vnic_encap);
        vnic_util_encap_value_stepper(&fabric_encap);
        mac_u64++;
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vnic_util::many_create(vnic_stepper_seed_t *seed) {
    return (vnic_util_object_stepper(seed, OP_MANY_CREATE));
}

sdk::sdk_ret_t
vnic_util::many_read(vnic_stepper_seed_t *seed, sdk::sdk_ret_t exp_result) {
    return (vnic_util_object_stepper(seed, OP_MANY_READ, exp_result));
}

sdk::sdk_ret_t
vnic_util::many_update(vnic_stepper_seed_t *seed) {
    return (vnic_util_object_stepper(seed, OP_MANY_UPDATE));
}

sdk::sdk_ret_t
vnic_util::many_delete(vnic_stepper_seed_t *seed) {
    return (vnic_util_object_stepper(seed, OP_MANY_DELETE));
}

static inline void
vnic_stepper_seed_populate_encap (uint32_t seed_base,
                                  pds_encap_type_t encap_type,
                                  pds_encap_t *encap)
{
    encap->type = encap_type;
    // update encap value to seed base
    switch (encap_type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        encap->val.vlan_tag = seed_base;
        break;
    case PDS_ENCAP_TYPE_QINQ:
        encap->val.qinq_tag.c_tag = seed_base;
        encap->val.qinq_tag.s_tag = seed_base + 4096;
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        encap->val.mpls_tag = seed_base;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        encap->val.vnid = seed_base;
        break;
    default:
        encap->val.value = seed_base;
        break;
    }
}

void
vnic_util::vnic_stepper_seed_init(uint32_t seed_base, uint32_t num_vnics,
                                  uint64_t seed_mac,
                                  pds_encap_type_t vnic_encap_type,
                                  pds_encap_type_t fabric_encap_type,
                                  bool src_dst_check,
                                  vnic_stepper_seed_t *seed) {
    seed->id = seed_base;
    seed->num_vnics = num_vnics;
    seed->mac_u64 = seed_mac;
    vnic_stepper_seed_populate_encap(seed_base, vnic_encap_type,
                                     &seed->vnic_encap);
    vnic_stepper_seed_populate_encap(seed_base, fabric_encap_type,
                                     &seed->fabric_encap);
    seed->src_dst_check = src_dst_check;
}

} // namespace api_test
