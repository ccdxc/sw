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

pds_encap_t default_encap = {PDS_ENCAP_TYPE_VXLAN, 0};

tep_util::tep_util() {
    this->ip_str = "0.0.0.0";
    this->type = PDS_TEP_TYPE_NONE;
}

tep_util::tep_util(std::string ip_str) {
    this->ip_str = ip_str;
    this->type = PDS_TEP_TYPE_NONE;
}

tep_util::tep_util(std::string ip_str, pds_tep_type_t type, pds_encap_t encap) {
    this->ip_str = ip_str;
    this->type = type;
    this->encap = encap;
}

tep_util::~tep_util() {}

static inline sdk::sdk_ret_t
tep_util_object_stepper(std::string pfxstr, pds_tep_type_t type,
                        pds_encap_t encap, uint32_t num_tep, utils_op_t op,
                        sdk_ret_t expected_result = sdk::SDK_RET_OK) {
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)pfxstr.c_str(), &ip_pfx) == 0);

    for (uint32_t idx = 1; idx <= num_tep; ++idx) {
        tep_util tep_obj(ippfx2str(&ip_pfx), type, encap);
        switch (op) {
        case OP_MANY_CREATE:
            rv = tep_obj.create();
            break;
        case OP_MANY_READ:
            pds_tep_info_t info;
            rv = tep_obj.read(&info, FALSE);
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
    spec.type = this->type;
    spec.encap = this->encap;
    spec.key.ip_addr = ip_pfx.addr.addr.v4_addr;
    return (pds_tep_create(&spec));
}

sdk::sdk_ret_t
tep_util::many_create(uint32_t num_tep, std::string pfxstr,
                      pds_tep_type_t type, pds_encap_t encap) {
    return (tep_util_object_stepper(pfxstr, type, encap,
                                    num_tep, OP_MANY_CREATE));
}

static inline void
debug_dump_tep_spec (pds_tep_spec_t *spec)
{
    printf("Spec data : \n");
    printf("Key IPv4 address 0x%x ", spec->key.ip_addr);
    printf("TEP Type %d", spec->type);
    printf("Encap Type %d", spec->encap.type);
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

    if ((rv = pds_tep_read(&key, info)) != sdk::SDK_RET_OK)
        return rv;

    if (compare_spec) {
        // TODO: Temporary untill p4pd_entry_read() works for directmap
        debug_dump_tep_info(info);
        // validate tep ip
        if (strcmp(this->ip_str.c_str(),
                   ipv4addr2str(info->spec.key.ip_addr))) {
            // TODO: what error to return
            return sdk::SDK_RET_ERR;
        }
        // validate tep type
        if (this->type != info->spec.type) {
            return sdk::SDK_RET_ERR;
        }
        // validate tep encap
        if (this->encap.type != info->spec.encap.type) {
            return sdk::SDK_RET_ERR;
        }
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
tep_util::many_read(uint32_t num_tep, std::string pfxstr, pds_tep_type_t type,
                    pds_encap_t encap, sdk_ret_t expected_result) {
    return (tep_util_object_stepper(pfxstr, type, encap, num_tep, OP_MANY_READ,
                                    expected_result));
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
                      pds_tep_type_t type, pds_encap_t encap) {
    return (tep_util_object_stepper(pfxstr, type, encap,
                                    num_tep, OP_MANY_DELETE));
}

}    // namespace api_test
