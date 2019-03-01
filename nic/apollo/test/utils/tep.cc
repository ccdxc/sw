//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the tep test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/tep.hpp"

namespace api_test {

tep_util::tep_util() {
    this->ip_str = "0.0.0.0";
    this->type = PDS_TEP_ENCAP_TYPE_NONE;
}

tep_util::tep_util(std::string ip_str) {
    this->ip_str = ip_str;
    this->type = PDS_TEP_ENCAP_TYPE_NONE;
}

tep_util::tep_util(std::string ip_str, pds_tep_encap_type_t type) {
    this->ip_str = ip_str;
    this->type = type;
}

tep_util::~tep_util() {}

sdk::sdk_ret_t
tep_util::create() {
    pds_tep_spec_t pds_tep;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->ip_str.c_str(), &ip_pfx) == 0);
    memset(&pds_tep, 0, sizeof(pds_tep));
    pds_tep.encap_type = this->type;
    pds_tep.key.ip_addr = ip_pfx.addr.addr.v4_addr;
    return (pds_tep_create(&pds_tep));
}

sdk::sdk_ret_t
tep_util::many_create(uint32_t num_tep, std::string pfxstr,
                      pds_tep_encap_type_t type) {
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    ip_prefix_t ip_pfx;
    SDK_ASSERT(num_tep <= PDS_MAX_TEP);

    SDK_ASSERT(str2ipv4pfx((char *)pfxstr.c_str(), &ip_pfx) == 0);
    for (uint32_t idx = 1; idx <= num_tep; ++idx) {
        tep_util tep_obj(ippfx2str(&ip_pfx), type);
        if ((rv = tep_obj.create()) != sdk::SDK_RET_OK)
            return rv;
        // Increment IPv4 address by 1 for next TEP
        ip_pfx.addr.addr.v4_addr += 1;
    }

    return rv;
}

static inline void
debug_dump_tep_spec (pds_tep_spec_t *spec)
{
    printf("Spec data : \n");
    printf("Key IPv4 address 0x%x ", spec->key.ip_addr);
    printf("Encap Type %d", spec->encap_type);
    printf("\n");
    return;
}

static inline void
debug_dump_tep_status (pds_tep_status_t *status)
{
    printf("Status data : \n");
    printf("NH id %d HW id %d ", status->nh_id, status->hw_id);
    printf("Outer Dest Mac 0x%x:%x:%x:%x:%x:%x", status->dmac[0],
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
tep_util::validate(pds_tep_info_t *info) {
    if (info == NULL)
        return sdk::SDK_RET_INVALID_ARG;

    // TODO: Temporary untill p4pd_entry_read() works
    debug_dump_tep_info(info);

    // TODO: validation code to compare test parameters against info
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
tep_util::read(pds_tep_info_t *info) {
    pds_tep_key_t key;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->ip_str.c_str(), &ip_pfx) == 0);
    memset(&key, 0, sizeof(pds_tep_key_t));
    memset(info, 0, sizeof(pds_tep_info_t));
    key.ip_addr = ip_pfx.addr.addr.v4_addr;
    return (pds_tep_read(&key, info));
}

sdk::sdk_ret_t
tep_util::del() {
    pds_tep_key_t pds_tep_key;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->ip_str.c_str(), &ip_pfx) == 0);
    memset(&pds_tep_key, 0, sizeof(pds_tep_key));
    pds_tep_key.ip_addr = ip_pfx.addr.addr.v4_addr;
    return (pds_tep_delete(&pds_tep_key));
}

}    // namespace api_test
