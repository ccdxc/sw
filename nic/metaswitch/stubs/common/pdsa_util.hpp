//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Common Utilities used by all PDSA stub components
//---------------------------------------------------------------

#ifndef __PDSA_UTIL_HPP__
#define __PDSA_UTIL_HPP__

#include <nbase.h>
#include <a0spec.h>
#include <o0mac.h>
#include <a0cust.h>
#include <a0mib.h>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds.hpp"

namespace pdsa_stub {

constexpr size_t VRF_PREF_LEN = 4;

static inline unsigned long 
vrfname_2_vrfid (const NBB_BYTE* vrfname, NBB_ULONG len)
{
    assert(len > VRF_PREF_LEN);
    return strtol(((const char*)vrfname)+VRF_PREF_LEN, nullptr, 10);
}

static inline void 
convert_ipaddr_ms_to_pdsa (const ATG_INET_ADDRESS& in_ip, ip_addr_t* out_ip)
{
    switch (in_ip.type) {
    case AMB_INETWK_ADDR_TYPE_IPV4:
        out_ip->af = IP_AF_IPV4;
        SDK_ASSERT (in_ip.length == IP4_ADDR8_LEN);
        break;
    case AMB_INETWK_ADDR_TYPE_IPV6:
        out_ip->af = IP_AF_IPV6;
        SDK_ASSERT (in_ip.length == IP6_ADDR8_LEN);
        break;
    default:
        SDK_ASSERT (0);
    }
    memcpy (&(out_ip->addr), &(in_ip.address), in_ip.length);
}

// Wrapper struct to use MAC as key in STL
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

class pds_batch_ctxt_guard_t {
public:
    pds_batch_ctxt_guard_t() {};
    pds_batch_ctxt_guard_t(pds_batch_ctxt_t bctxt) : bctxt_ (bctxt) {};
    ~pds_batch_ctxt_guard_t (void) {
#if 0 /* TODO: SDK Library linkage */
        if (bctxt_ != 0) {pds_batch_destroy (bctxt_);}
#endif
    }
    // Allow move
    pds_batch_ctxt_guard_t(pds_batch_ctxt_guard_t&& bg) {
        bctxt_ = bg.bctxt_;
        bg.bctxt_ = 0;
    }
    pds_batch_ctxt_guard_t& operator=(pds_batch_ctxt_guard_t&& bg) {
        bctxt_ = bg.bctxt_;
        bg.bctxt_ = 0;
        return *this;
    }
    // Prohibit copy
    pds_batch_ctxt_guard_t(const pds_batch_ctxt_guard_t& bg)=delete; 
    pds_batch_ctxt_guard_t& operator=(const pds_batch_ctxt_guard_t& bg)=delete;
    void set (pds_batch_ctxt_t bctxt) {
#if 0 /* TODO: SDK Library linkage */
        if (bctxt_ != 0) {pds_batch_destroy (bctxt_);}
#endif
        bctxt_ = bctxt;
    }
    pds_batch_ctxt_t get(void) {return bctxt_;}
    pds_batch_ctxt_t release(void) {
        pds_batch_ctxt_t bctxt = bctxt_;
        bctxt_ = 0; return bctxt;
    }
private:
    pds_batch_ctxt_t bctxt_ = 0;
};

}
#endif
