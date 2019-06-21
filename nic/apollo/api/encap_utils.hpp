//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains encap utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_ENCAP_UTILS_HPP__
#define __API_ENCAP_UTILS_HPP__

#include <iostream>
#include "nic/apollo/api/include/pds.hpp"

static inline char *
pds_encap2str (pds_encap_t encap)
{
    static thread_local char buf_pool[4][32];
    static thread_local int buf_next = 0;
    char *buf;

    buf = buf_pool[buf_next++ % 4];
    switch (encap.type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        snprintf(buf, 32, "Dot1q:%u", encap.val.vlan_tag);
        break;
    case PDS_ENCAP_TYPE_QINQ:
        snprintf(buf, 32, "QinQ:%u/%u",
                 encap.val.qinq_tag.c_tag, encap.val.qinq_tag.s_tag);
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        snprintf(buf, 32, "MPLSoUDP:%u", encap.val.mpls_tag);
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        snprintf(buf, 32, "VXLAN:%u", encap.val.vnid);
        break;
    case PDS_ENCAP_TYPE_NONE:
    default:
        snprintf(buf, 32, "None");
        break;
    }
    return buf;
}

#endif    // __API_ENCAP_UTILS_HPP__
