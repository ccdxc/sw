//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Common Utilities used by all PDSA stub components
//---------------------------------------------------------------

#ifndef __PDSA_UTIL_HPP__
#define __PDSA_UTIL_HPP__

#include <nbase.h>
#include "sdk/include/sdk/eth.hpp"

namespace pdsa_stub {

constexpr size_t VRF_PREF_LEN = 4;

static inline unsigned long vrfname_2_vrfid(const NBB_BYTE* vrfname, 
                                            NBB_ULONG len)
{
    assert(len > VRF_PREF_LEN);
    return strtol(((const char*)vrfname)+VRF_PREF_LEN, nullptr, 10);
}

struct mac_addr_wr_t {
    mac_addr_t m_mac;
    mac_addr_wr_t(void) {
        memset(m_mac, 0, ETH_ADDR_LEN); 
    }
    mac_addr_wr_t(const mac_addr_t& mac) {
        memcpy(m_mac, mac, ETH_ADDR_LEN); 
    }
    bool operator<(const mac_addr_wr_t& mac) const {
        int i = 0;
        while (i<ETH_ADDR_LEN) {
            if (m_mac[i] < mac.m_mac[i]) {return 1;}
            if (m_mac[i] > mac.m_mac[i]) {return 0;}
            ++i;
        }
        return 0;
    }
    bool operator==(const mac_addr_wr_t& m2) const {
        return(memcmp(m_mac, m2.m_mac, ETH_ADDR_LEN) == 0);
    }
};

}

#endif
