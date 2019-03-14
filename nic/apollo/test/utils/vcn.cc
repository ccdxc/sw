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

namespace api_test {

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

    memset(&spec, 0, sizeof(spec));
    spec.type = this->type;
    spec.key.id = this->id;
    spec.pfx = ip_pfx;
    return (pds_vcn_create(&spec));
}

#if 0
sdk::sdk_ret_t
vcn_util::read(pds_vcn_spec_t *vcn)
{
    return sdk::SDK_RET_OK;
    return (pds_vcn_create(&spec));
}

sdk::sdk_ret_t
vcn_util::update(pds_vcn_spec_t *vcn_spec)
{

    pds_vcn_key_t pds_vcn_key;
    pds_vcn_spec_t updated_vcn;

    memset(&pds_vcn_key, 0, sizeof(pds_vcn_key_t));
    pds_vcn_key.id = vcn_spec->key.id;

    pds_vcn_delete(&pds_vcn_key);

    updated_vcn.key.id = vcn_spec->key.id;
    updated_vcn.type = vcn_spec->type;
    updated_vcn.pfx = vcn_spec->pfx;

    return (pds_vcn_update(vcn_spec));
}
#endif

sdk::sdk_ret_t
vcn_util::del() {
    pds_vcn_key_t pds_vcn_key;

    memset(&pds_vcn_key, 0, sizeof(pds_vcn_key_t));
    pds_vcn_key.id = this->id;
    return (pds_vcn_delete(&pds_vcn_key));
}

#if 0
sdk::sdk_ret_t
vcn_util::read(pds_vcn_spec_t *vcn)
{
    return sdk::SDK_RET_OK;
}
#endif

sdk::sdk_ret_t
vcn_util::update(pds_vcn_spec_t *vcn_spec) {

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
vcn_util::many_create(uint32_t num_vcn, std::string pfxstr,
                      pds_vcn_type_t vcn_type) {
#if 0
    //sdk::sdk_ret_t rv;
    SDK_ASSERT(num_vcn <= 1024);

    for (uint32_t idx = 1; idx <= num_vcn; idx++)
        //if ((rv = create()) != sdk::SDK_RET_OK)
        //    return rv;

#endif
    return sdk::SDK_RET_OK;
}

}    // namespace api_test
