//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the nh test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/test/utils/nh.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

nh_util::nh_util(pds_nexthop_id_t id, pds_nh_type_t type, ip_addr_t ip,
                 pds_vpc_key_t vpc, uint16_t vlan, uint64_t mac) {
    this->id = id;
    this->ip = ip;
    this->type = type;
    this->vpc = vpc;
    this->vlan = vlan;
    this->mac = mac;
}

nh_util::~nh_util() {}

static inline void
debug_dump_nh_spec (pds_nexthop_spec_t *spec)
{
    printf("Spec data : \n");
    printf("Key ID %d", spec->key);
    printf(", NH Type %d", spec->type);
    printf(", VPC %d", spec->vpc.id);
    printf(", IP %s", ipaddr2str(&spec->ip));
    printf(", Vlan %d", spec->vlan);
    printf(", MAC %s", macaddr2str(spec->mac));
    printf("\n");
}

static inline void
debug_dump_nh_status (pds_nexthop_status_t *status)
{
    printf("Status data : \n");
    printf("NH HW id %d", status->hw_id);
    printf("\n");
}

static inline void
debug_dump_nh_stats (pds_nexthop_stats_t *stats)
{
    printf("Stats data : \n");
    printf("\n");
}

static inline void
debug_dump_nh_info (pds_nexthop_info_t *info)
{
    debug_dump_nh_spec(&info->spec);
    debug_dump_nh_status(&info->status);
    debug_dump_nh_stats(&info->stats);
}

sdk::sdk_ret_t
nh_util::create(void) const {
    pds_nexthop_spec_t spec;

    memset(&spec, 0, sizeof(pds_nexthop_spec_t));
    spec.key = this->id;
    spec.type = this->type;
    spec.vpc = this->vpc;
    spec.ip = this->ip;
    spec.vlan = this->vlan;
    MAC_UINT64_TO_ADDR(spec.mac, this->mac);
    return (pds_nexthop_create(&spec));
}

sdk::sdk_ret_t
nh_util::read(pds_nexthop_info_t *info) const {
    sdk_ret_t rv;
    pds_nexthop_key_t key = this->id;
    ip_addr_t nh_ip = this->ip;

    memset(info, 0, sizeof(pds_nexthop_info_t));

    if ((rv = pds_nexthop_read(&key, info)) != sdk::SDK_RET_OK)
        return rv;

    // dump for debug
    // debug_dump_nh_info(info);

    // validate NH type
    if (this->type != info->spec.type)
        return sdk::SDK_RET_ERR;

    // nothing much to check for blackhole nh
    if (this->type == PDS_NH_TYPE_BLACKHOLE)
        return sdk::SDK_RET_OK;

    // validate NH vlan
    if (this->vlan != info->spec.vlan)
        return sdk::SDK_RET_ERR;

    // validate NH MAC
    if (this->mac != MAC_TO_UINT64(info->spec.mac))
        return sdk::SDK_RET_ERR;

    if (this->type != PDS_NH_TYPE_IP) {
        // validate NH vpc
        if (this->vpc.id != info->spec.vpc.id)
            return sdk::SDK_RET_ERR;

        // validate NH ip
        if (!IPADDR_EQ(&nh_ip, &info->spec.ip))
            return sdk::SDK_RET_ERR;
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
nh_util::update(void) const {
    pds_nexthop_spec_t spec;

    memset(&spec, 0, sizeof(pds_nexthop_spec_t));
    spec.key = this->id;
    spec.type = this->type;
    spec.vpc = this->vpc;
    spec.ip = this->ip;
    spec.vlan = this->vlan;
    MAC_UINT64_TO_ADDR(spec.mac, this->mac);
    return (pds_nexthop_update(&spec));
}

sdk::sdk_ret_t
nh_util::del(void) const {
    pds_nexthop_key_t key = this->id;

    return (pds_nexthop_delete(&key));
}

static inline sdk::sdk_ret_t
nh_util_object_stepper (nh_stepper_seed_t *seed, utils_op_t op,
                        sdk_ret_t expected_result = sdk::SDK_RET_OK)
{
    sdk::sdk_ret_t rv;
    ip_addr_t ip_addr = seed->ip;
    uint16_t vlan = seed->vlan;
    pds_vpc_key_t vpc = {.id=seed->vpc_id};
    uint64_t mac = seed->mac;

    for (uint32_t idx = seed->id; idx < seed->id + seed->num_nh; idx++) {
        if (vlan >= 4096) vlan %= 4096;
        nh_util nh_obj(idx, seed->type, ip_addr, vpc, vlan, mac);

        switch (op) {
        case OP_MANY_CREATE:
            rv = nh_obj.create();
            break;
        case OP_MANY_READ:
            pds_nexthop_info_t info;
            rv = nh_obj.read(&info);
            break;
        case OP_MANY_UPDATE:
            rv = nh_obj.update();
            break;
        case OP_MANY_DELETE:
            rv = nh_obj.del();
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return sdk::SDK_RET_ERR;
        }
        // Increment IPv4 address by 1 for next NH
        ip_addr.addr.v4_addr += 1;
        // Increment MAC
        mac += 1;
        // Increment vlan
        vlan += 1;
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
nh_util::many_create(nh_stepper_seed_t *seed) {
    return (nh_util_object_stepper(seed, OP_MANY_CREATE));
}

sdk::sdk_ret_t
nh_util::many_read(nh_stepper_seed_t *seed, sdk_ret_t expected_result) {
    return (nh_util_object_stepper(seed, OP_MANY_READ, expected_result));
}

sdk::sdk_ret_t
nh_util::many_update(nh_stepper_seed_t *seed) {
    return (nh_util_object_stepper(seed, OP_MANY_UPDATE));
}

sdk::sdk_ret_t
nh_util::many_delete(nh_stepper_seed_t *seed) {
    return (nh_util_object_stepper(seed, OP_MANY_DELETE));
}

void
nh_util::stepper_seed_init(nh_stepper_seed_t *seed, std::string ip_str,
                           uint64_t mac, uint32_t num_nh,
                           pds_nexthop_id_t seed_base, pds_nh_type_t type,
                           uint16_t vlan, pds_vpc_id_t vpc_id) {
    seed->id = seed_base;
    seed->type = type;
    extract_ip_addr(ip_str.c_str(), &seed->ip);
    seed->mac = mac;
    seed->num_nh = num_nh;
    seed->vlan = vlan;
    seed->vpc_id = vpc_id;
}

}    // namespace api_test
