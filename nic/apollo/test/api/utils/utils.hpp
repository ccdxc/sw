//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the utils routines for tests.
///
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_UTILS_HPP__
#define __TEST_API_UTILS_UTILS_HPP__

namespace test {
namespace api {

static inline void
pds_str2encap (const char *buf, pds_encap_t *encap)
{
    char encap_type[16];
    uint32_t encap_val = 0;

    sscanf(buf, "%s %u", encap_type, &encap_val);
    if (strcmp(encap_type, "Dot1q") == 0)
        encap->type = PDS_ENCAP_TYPE_DOT1Q;
    if (strcmp(encap_type, "QinQ") == 0)
        encap->type = PDS_ENCAP_TYPE_QINQ;
    if (strcmp(encap_type, "MPLSoUDP") == 0)
        encap->type = PDS_ENCAP_TYPE_MPLSoUDP;
    else if (strcmp(encap_type, "VxLAN") == 0)
        encap->type = PDS_ENCAP_TYPE_VXLAN;
    encap->val.value = encap_val;
}

inline void
utils_encap_type_update (pds_encap_t *encap)
{
    if (encap->type == PDS_ENCAP_TYPE_MPLSoUDP) {
        encap->type = PDS_ENCAP_TYPE_VXLAN;
        encap->val.vnid = 1;
    } else if (encap->type == PDS_ENCAP_TYPE_VXLAN) {
        encap->type = PDS_ENCAP_TYPE_MPLSoUDP;
        encap->val.mpls_tag = 1;
    }
}

inline void
utils_encap_val_update (pds_encap_t *encap, uint32_t width=1)
{
    switch (encap->type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        encap->val.vlan_tag += width;
        break;
    case PDS_ENCAP_TYPE_QINQ:
        encap->val.qinq_tag.c_tag += width;
        encap->val.qinq_tag.s_tag += width;
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        encap->val.mpls_tag += width;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        encap->val.vnid += width;
        break;
    default:
        encap->val.value += width;
        break;
    }
}

/// @}

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_UTILS_HPP__
