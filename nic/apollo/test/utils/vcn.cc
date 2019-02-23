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

namespace api_test {

vcn_util::vcn_util(oci_vcn_type_t type, oci_vcn_id_t id, std::string cidr_str) {
    this->type = type;
    this->id = id;
    this->cidr_str = cidr_str;
}

vcn_util::~vcn_util() {}

sdk::sdk_ret_t
vcn_util::create() {
    oci_vcn_spec_t spec;
    ip_prefix_t ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->cidr_str.c_str(), &ip_pfx) == 0);

    memset(&spec, 0, sizeof(spec));
    spec.type = this->type;
    spec.key.id = this->id;
    spec.pfx = ip_pfx;
    printf("vcn:%d %s %x\n", spec.key.id, (char *)cidr_str.c_str(),
           spec.pfx.addr.addr.v4_addr);
    return (oci_vcn_create(&spec));
}

#if 0
sdk::sdk_ret_t
vcn_util::read(oci_vcn_spec_t *vcn)
{
    return sdk::SDK_RET_OK;
    return (oci_vcn_create(&spec));
}

sdk::sdk_ret_t
vcn_util::update(oci_vcn_spec_t *vcn_spec)
{

    oci_vcn_key_t oci_vcn_key;
    oci_vcn_spec_t updated_vcn;

    memset(&oci_vcn_key, 0, sizeof(oci_vcn_key_t));
    oci_vcn_key.id = vcn_spec->key.id;

    oci_vcn_delete(&oci_vcn_key);

    updated_vcn.key.id = vcn_spec->key.id;
    updated_vcn.type = vcn_spec->type;
    updated_vcn.pfx = vcn_spec->pfx;

    return (oci_vcn_update(vcn_spec));
}
#endif

sdk::sdk_ret_t
vcn_util::del() {
    oci_vcn_key_t oci_vcn_key;

    memset(&oci_vcn_key, 0, sizeof(oci_vcn_key_t));
    oci_vcn_key.id = this->id;
    return (oci_vcn_delete(&oci_vcn_key));
}

#if 0
sdk::sdk_ret_t
vcn_util::read(oci_vcn_spec_t *vcn)
{
    return sdk::SDK_RET_OK;
}
#endif

sdk::sdk_ret_t
vcn_util::update(oci_vcn_spec_t *vcn_spec) {

#if 0
    oci_vcn_key_t oci_vcn_key;
    oci_vcn_spec_t updated_vcn;

    memset(&oci_vcn_key, 0, sizeof(oci_vcn_key_t));
    oci_vcn_key.id = vcn_spec->key.id;

    oci_vcn_delete(&oci_vcn_key);

    updated_vcn.key.id = vcn_spec->key.id;
    updated_vcn.type = vcn_spec->type;
    updated_vcn.pfx = vcn_spec->pfx;
#endif

    return (oci_vcn_update(vcn_spec));
}

sdk::sdk_ret_t
vcn_util::many_create(uint32_t num_vcn, std::string pfxstr,
                      oci_vcn_type_t vcn_type) {
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
