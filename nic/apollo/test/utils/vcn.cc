//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vcn test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/vcn.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "iostream"

using namespace std;
namespace api_test {

vcn_util::vcn_util(pds_vcn_id_t id) {
    this->type = PDS_VCN_TYPE_TENANT;
    this->id = id;
    this->cidr_str = "";
}

vcn_util::vcn_util(pds_vcn_id_t id, std::string cidr_str) {
    this->type = PDS_VCN_TYPE_TENANT;
    this->id = id;
    this->cidr_str = cidr_str;
}

vcn_util::vcn_util(pds_vcn_type_t type, pds_vcn_id_t id, std::string cidr_str) {
    this->type = type;
    this->id = id;
    this->cidr_str = cidr_str;
}

vcn_util::~vcn_util() {}

sdk::sdk_ret_t
vcn_util::create() {
    pds_vcn_spec_t spec;
    ip_prefix_t ip_pfx;

    extract_ip_pfx(this->cidr_str.c_str(), &ip_pfx);

    SDK_ASSERT(TRUE);
    memset(&spec, 0, sizeof(spec));
    spec.type = this->type;
    spec.key.id = this->id;
    spec.pfx = ip_pfx;
    return (pds_vcn_create(&spec));
}

sdk::sdk_ret_t
vcn_util::read(pds_vcn_info_t *info, bool compare_spec)
{
    sdk_ret_t rv;
    pds_vcn_key_t key;

    memset(&key, 0, sizeof(pds_vcn_key_t));
    memset(info, 0, sizeof(pds_vcn_info_t));
    key.id = this->id;
    rv = pds_vcn_read(&key, info);
    cout << "vcn : key : " << key.id << ", id : " << info->spec.key.id << ", addr : "
         << ippfx2str(&info->spec.pfx) << "\n";
    if (rv != sdk::SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {

    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vcn_util::update(pds_vcn_spec_t *vcn_spec)
{
#if 0
    pds_vcn_key_t pds_vcn_key;
    pds_vcn_spec_t updated_vcn;

    memset(&pds_vcn_key, 0, sizeof(pds_vcn_key_t));
    pds_vcn_key.id = vcn_spec->key.id;

    pds_vcn_delete(&pds_vcn_key);

    updated_vcn.key.id = vcn_spec->key.id;
    updated_vcn.type = vcn_spec->type;
    updated_vcn.pfx = vcn_spec->pfx;
#endif
    return (pds_vcn_update(vcn_spec));
}

sdk::sdk_ret_t
vcn_util::del() {
    pds_vcn_key_t key = {};
    key.id = this->id;
    return (pds_vcn_delete(&key));
}

static inline sdk::sdk_ret_t
vcn_util_object_stepper(pds_vcn_key_t start_key, std::string start_pfxstr,
                        uint32_t num_vcns, utils_op_t op,
                        pds_vcn_type_t type, sdk_ret_t expected_result)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    ip_prefix_t ip_pfx;
    uint32_t addr;
    pds_vcn_info_t info = {};

    if (start_key.id == 0) start_key.id = 1;
    if (op == OP_MANY_CREATE) {
        SDK_ASSERT(str2ipv4pfx((char *)start_pfxstr.c_str(), &ip_pfx) == 0);
        addr = ip_pfx.addr.addr.v4_addr;
    }
    for (uint32_t idx = start_key.id; idx <= start_key.id + num_vcns; idx++) {
        ip_pfx.addr.addr.v4_addr = addr;
        vcn_util vcn_obj(type, idx, ippfx2str(&ip_pfx));
        switch (op) {
        case OP_MANY_CREATE:
            rv = vcn_obj.create();
            break;
        case OP_MANY_DELETE:
            rv = vcn_obj.del();
            break;
        case OP_MANY_READ:
            rv = vcn_obj.read(&info, TRUE);
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return rv;
        }
        addr = api_test::pds_get_next_addr16(addr);
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vcn_util::many_create(pds_vcn_key_t start_key, std::string start_pfxstr,
                      uint32_t num_vcns, pds_vcn_type_t type) {
    return (vcn_util_object_stepper(start_key, start_pfxstr, num_vcns,
                                    OP_MANY_CREATE, PDS_VCN_TYPE_TENANT,
                                    sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
vcn_util::many_read(pds_vcn_key_t start_key, uint32_t num_vcns,
                    sdk::sdk_ret_t expected_result) {
    return (vcn_util_object_stepper(start_key, "", num_vcns,
                                    OP_MANY_READ, PDS_VCN_TYPE_TENANT,
                                    expected_result));
}

sdk::sdk_ret_t
vcn_util::many_delete(pds_vcn_key_t start_key, uint32_t num_vcns) {
    return (vcn_util_object_stepper(start_key, "", num_vcns,
                                    OP_MANY_DELETE, PDS_VCN_TYPE_TENANT,
                                    sdk::SDK_RET_OK));
}

}    // namespace api_test
