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
#include "nic/apollo/test/utils/utils.hpp"

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

static inline sdk::sdk_ret_t
tep_util_object_stepper(sdk_ret_t expected_result, utils_op_t op,
                        uint32_t num_tep, std::string pfxstr,
                        pds_tep_encap_type_t type) {
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)pfxstr.c_str(), &ip_pfx) == 0);
    for (uint32_t idx = 1; idx <= num_tep; ++idx) {
        tep_util tep_obj(ippfx2str(&ip_pfx), type);
        switch (op) {
        case OP_MANY_CREATE:
            rv = tep_obj.create();
            break;
        case OP_MANY_READ:
            pds_tep_info_t info;
            rv = tep_obj.read(&info);
            break;
        case OP_MANY_DELETE:
            rv = tep_obj.del();
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return rv;
        }
        // Increment IPv4 address by 1 for next TEP
        ip_pfx.addr.addr.v4_addr += 1;
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
tep_util::create() {
    pds_tep_spec_t spec;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->ip_str.c_str(), &ip_pfx) == 0);
    memset(&spec, 0, sizeof(pds_tep_spec_t));
    spec.encap_type = this->type;
    spec.key.ip_addr = ip_pfx.addr.addr.v4_addr;
    return (pds_tep_create(&spec));
}

sdk::sdk_ret_t
tep_util::many_create(uint32_t num_tep, std::string pfxstr,
                      pds_tep_encap_type_t type) {
    return (tep_util_object_stepper(sdk::SDK_RET_OK, OP_MANY_CREATE,
                                    num_tep, pfxstr, type));
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
tep_util::read(pds_tep_info_t *info, bool compare_spec) {
    sdk_ret_t rv;
    pds_tep_key_t key;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->ip_str.c_str(), &ip_pfx) == 0);
    memset(&key, 0, sizeof(pds_tep_key_t));
    memset(info, 0, sizeof(pds_tep_info_t));
    key.ip_addr = ip_pfx.addr.addr.v4_addr;
    rv = pds_tep_read(&key, info);
    if (rv != sdk::SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {
        // TODO: Temporary untill p4pd_entry_read() works for directmap
        debug_dump_tep_info(info);
        // validate tep ip
        if (strcmp(this->ip_str.c_str(),
                   ipv4addr2str(info->spec.key.ip_addr))) {
            // TODO: what error to return
            return sdk::SDK_RET_ERR;
        }
        // validate tep encap type
        if (this->type != info->spec.encap_type) {
            return sdk::SDK_RET_ERR;
        }
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
tep_util::many_read(sdk_ret_t ret_code, uint32_t num_tep, std::string pfxstr,
                    pds_tep_encap_type_t type) {
    return (tep_util_object_stepper(ret_code, OP_MANY_READ,
                                    num_tep, pfxstr, type));
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

sdk::sdk_ret_t
tep_util::many_delete(uint32_t num_tep, std::string pfxstr,
                      pds_tep_encap_type_t type) {
    return (tep_util_object_stepper(sdk::SDK_RET_OK, OP_MANY_DELETE,
                                    num_tep, pfxstr, type));
}

}    // namespace api_test
