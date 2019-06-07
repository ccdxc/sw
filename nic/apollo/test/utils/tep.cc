//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the tep test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

constexpr pds_encap_t k_default_tep_encap = {PDS_ENCAP_TYPE_MPLSoUDP, 100};
constexpr pds_encap_t k_zero_encap = {PDS_ENCAP_TYPE_NONE, 0};
constexpr ip_addr_t k_zero_ip = {IP_AF_IPV4, {0}};
constexpr pds_tep_type_t k_default_tep_type = PDS_TEP_TYPE_WORKLOAD;

tep_util::tep_util(std::string ip_str, pds_tep_type_t type,
                   pds_encap_t encap, bool nat) {
    extract_ip_addr(ip_str.c_str(), &this->ip_addr);
    this->type = type;
    this->encap = encap;
    this->nat = nat;
}

tep_util::tep_util(ip_addr_t ip_addr, pds_tep_type_t type,
                   pds_encap_t encap, bool nat, ip_addr_t dipi,
                   uint64_t dmac) {
    this->ip_addr = ip_addr;
    this->type = type;
    this->encap = encap;
    this->nat = nat;
    this->dipi = dipi;
    this->dmac = dmac;
}

tep_util::~tep_util() {}

static inline void
debug_dump_tep_spec (pds_tep_spec_t *spec)
{
    printf("Spec data : \n");
    printf("Key IPv4 address %s", ipaddr2str(&spec->key.ip_addr));
    printf(", DIPi %s", ipaddr2str(&spec->ip_addr));
    printf(", dMAC %s", macaddr2str(spec->mac));
    printf(", TEP Type %d", spec->type);
    printf(", Encap %s", pdsencap2str(spec->encap));
    printf(", NAT %d", spec->nat);
    printf("\n");
    return;
}

static inline void
debug_dump_tep_status (pds_tep_status_t *status)
{
    printf("Status data : \n");
    printf("NH id %d, HW id %d", status->nh_id, status->hw_id);
    printf(", Outer Dest Mac 0x%x:%x:%x:%x:%x:%x", status->dmac[0],
           status->dmac[1], status->dmac[2], status->dmac[3], status->dmac[4],
           status->dmac[5]);
    printf("\n");
    return;
}

static inline void
debug_dump_tep_stats (pds_tep_stats_t *stats)
{
    printf("Stats data : \n");
    printf("\n");
    return;
}

static inline void
debug_dump_tep_info (pds_tep_info_t *info)
{
    debug_dump_tep_spec(&info->spec);
    debug_dump_tep_status(&info->status);
    debug_dump_tep_stats(&info->stats);
    return;
}

sdk::sdk_ret_t
tep_util::create(void) const {
    pds_tep_spec_t spec;

    memset(&spec, 0, sizeof(pds_tep_spec_t));
    spec.type = this->type;
    spec.encap = this->encap;
    spec.nat = this->nat;
    spec.key.ip_addr = this->ip_addr;
    spec.ip_addr = this->dipi;
    MAC_UINT64_TO_ADDR(spec.mac, this->dmac);
    return (pds_tep_create(&spec));
}

sdk::sdk_ret_t
tep_util::read(pds_tep_info_t *info) const {
    sdk_ret_t rv;
    pds_tep_key_t key;
    ip_addr_t tep_ip = this->ip_addr, tep_dipi = this->dipi;
#ifdef APOLLO
    pds_encap_t tep_encap = this->encap;
#else
    pds_encap_t tep_encap = k_zero_encap;
#endif

    memset(info, 0, sizeof(pds_tep_info_t));
    memset(&key, 0, sizeof(pds_tep_key_t));
    key.ip_addr = tep_ip;

    if ((rv = pds_tep_read(&key, info)) != sdk::SDK_RET_OK)
        return rv;

    // dump for debug
    debug_dump_tep_info(info);

    // validate TEP ip
    if (!IPADDR_EQ(&tep_ip, &info->spec.key.ip_addr))
        return sdk::SDK_RET_ERR;

    // validate TEP DIPi
    if (!IPADDR_EQ(&tep_dipi, &info->spec.ip_addr))
        return sdk::SDK_RET_ERR;

    // validate TEP type
    if (this->type != info->spec.type)
        return sdk::SDK_RET_ERR;

    // validate TEP encap
    if (!api::pdsencap_isequal(&tep_encap, &info->spec.encap))
        return sdk::SDK_RET_ERR;

    // validate NAT
    if (this->nat != info->spec.nat)
        return sdk::SDK_RET_ERR;

    // validate destination MAC
    if (this->dmac != MAC_TO_UINT64(info->spec.mac))
        return sdk::SDK_RET_ERR;

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
tep_util::update(void) const {
    pds_tep_spec_t spec;

    memset(&spec, 0, sizeof(pds_tep_spec_t));
    spec.type = this->type;
    spec.encap = this->encap;
    spec.nat = this->nat;
    spec.key.ip_addr = this->ip_addr;
    spec.ip_addr = this->dipi;
    MAC_UINT64_TO_ADDR(spec.mac, this->dmac);
    return (pds_tep_update(&spec));
}

sdk::sdk_ret_t
tep_util::del(void) const {
    pds_tep_key_t pds_tep_key;

    memset(&pds_tep_key, 0, sizeof(pds_tep_key));
    pds_tep_key.ip_addr = this->ip_addr;
    return (pds_tep_delete(&pds_tep_key));
}

static inline sdk::sdk_ret_t
tep_util_object_stepper (tep_stepper_seed_t *seed, utils_op_t op,
                         sdk_ret_t expected_result = sdk::SDK_RET_OK)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    ip_addr_t ip_addr = seed->ip_addr;
    ip_addr_t dipi = seed->dipi;
    uint64_t dmac = seed->dmac;

    for (uint32_t idx = 1; idx <= seed->num_tep; ++idx) {
        tep_util tep_obj(ip_addr, seed->type, seed->encap,
                         seed->nat, dipi, dmac);
        switch (op) {
        case OP_MANY_CREATE:
            rv = tep_obj.create();
            break;
        case OP_MANY_READ:
            pds_tep_info_t info;
            rv = tep_obj.read(&info);
            break;
        case OP_MANY_UPDATE:
            rv = tep_obj.update();
            break;
        case OP_MANY_DELETE:
            rv = tep_obj.del();
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return sdk::SDK_RET_ERR;
        }
        // Increment IPv4 address by 1 for next TEP
        ip_addr.addr.v4_addr += 1;
        // Increment DMAC if set
        if (dmac)
            dmac += 1;
        // Increment DIPi if set
        if (!ip_addr_is_zero(&dipi))
            dipi.addr.v6_addr.addr64[1] += 1;
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
tep_util::many_create(tep_stepper_seed_t *seed) {
    return (tep_util_object_stepper(seed, OP_MANY_CREATE));
}

sdk::sdk_ret_t
tep_util::many_read(tep_stepper_seed_t *seed, sdk_ret_t expected_result) {
    return (tep_util_object_stepper(seed, OP_MANY_READ, expected_result));
}

sdk::sdk_ret_t
tep_util::many_update(tep_stepper_seed_t *seed) {
    return (tep_util_object_stepper(seed, OP_MANY_UPDATE));
}

sdk::sdk_ret_t
tep_util::many_delete(tep_stepper_seed_t *seed) {
    return (tep_util_object_stepper(seed, OP_MANY_DELETE));
}

void
tep_util::stepper_seed_init(tep_stepper_seed_t *seed, std::string ip_str,
                            uint32_t num_tep, pds_encap_t encap, bool nat,
                            pds_tep_type_t type, std::string dipi_str,
                            uint64_t dmac) {
    extract_ip_addr(ip_str.c_str(), &seed->ip_addr);
    extract_ip_addr(dipi_str.c_str(), &seed->dipi);
    seed->dmac = dmac;
    seed->num_tep = num_tep;
    seed->type = type;
    seed->encap = encap;
    seed->nat = nat;
}

}    // namespace api_test
