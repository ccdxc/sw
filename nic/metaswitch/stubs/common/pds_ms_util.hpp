//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Common Utilities used by all PDS-MS stub components
//---------------------------------------------------------------

#ifndef __PDS_MS_UTIL_HPP__
#define __PDS_MS_UTIL_HPP__

#include <nbase.h>
extern "C" {
#include <a0spec.h>
#include <o0mac.h>
#include <a0cust.h>
#include <a0mib.h>
#include "smsiincl.h"
#include "smsincl.h"
}
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include <string>

namespace pds_ms {

constexpr size_t VRF_PREF_LEN = 4;

static inline unsigned long
vrfname_2_vrfid (const NBB_BYTE* vrfname, NBB_ULONG len)
{
    auto vrfname_ = (const char*) vrfname;
    if (len == 0) {return 0;}
    auto vrf_id = strtol(vrfname_, nullptr, 10);
    if (vrf_id == 0) {
        throw Error(std::string("Invalid VRF Name from Metaswitch - ")
                    .append(vrfname_));
    }
    return vrf_id;
}

static inline void
ms_to_pds_ipaddr (const ATG_INET_ADDRESS& in_ip, ip_addr_t* out_ip)
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
    if (out_ip->af == IP_AF_IPV4) {
        // MS IP addresses are byte arrays
        // HAL uses SDK ipv4_addr_t address which is in host order
        out_ip->addr.v4_addr = ntohl(out_ip->addr.v4_addr);
    }
}

static inline void
pds_to_ms_ipaddr (ip_addr_t   in_ip, ATG_INET_ADDRESS*  out_ip)
{
    if (in_ip.af == IP_AF_IPV4) {
        // HAL uses SDK ipv4_addr_t address which is in host order
        // MS IP addresses are byte arrays
        in_ip.addr.v4_addr = htonl(in_ip.addr.v4_addr);
    }
    switch (in_ip.af)
    {
        case IP_AF_IPV4:
            out_ip->type = AMB_INETWK_ADDR_TYPE_IPV4;
            out_ip->length = AMB_MAX_IPV4_ADDR_LEN;
            break;

        case IP_AF_IPV6:
            out_ip->type = AMB_INETWK_ADDR_TYPE_IPV6;
            out_ip->length = AMB_MAX_IPV6_ADDR_LEN;
            break;

        default:
            out_ip->type = out_ip->length = 0;
            return;
    }

    NBB_MEMCPY (&(out_ip->address), &(in_ip.addr), out_ip->length);
    return;
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
        if (bctxt_ != 0) {pds_batch_destroy (bctxt_);}
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
        if (bctxt_ != 0) {pds_batch_destroy (bctxt_);}
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

class ip_hash {
public:
    std::size_t operator()(const ip_addr_t &ip_addr) const {
        if (ip_addr.af == IP_AF_IPV4) {
            // For v4 hash only the v4 part
            return hash_algo::fnv_hash((void *)&ip_addr.addr.v4_addr,
                                       sizeof(ip_addr.addr.v4_addr));
        }
        return hash_algo::fnv_hash((void *)&ip_addr, sizeof(ip_addr));
    }
};

class ip_prefix_hash {
public:
    std::size_t operator()(const ip_prefix_t &pfx) const {
        return hash_algo::fnv_hash((void *)&pfx, sizeof(pfx));
    }
};

// The following UUID conversion utilities are used by control-plane
// to generate UUIDs for TEP, ECMP and Next-hop tables from their u32 indexes.
// Currently these UUIDs are not unique across different tables
// within the same Naples as the u32 index can overlap for different tables. 
//
// This routine has logic to return different UUIDs for
// underlay and overlay ECMP table even if their u32 index overlaps.
// Because of this requirement we cannot use the generic
// api::uuid_from_objid() since that always returns the same sticky UUID
// for a given u32 index
static inline pds_obj_key_t
msidx2pdsobjkey (uint32_t id, bool underlay=false) {
    pds_obj_key_t key = { 0 };
    sprintf (key.id, "%x", id);

    // stash a signature in 10th byte
    if (!underlay) {
        key.id[9] = 0x40;
    } else {
        // Use signature to differentiate overlay and underlay indexes
        // since both indexes are allocated independently in MS HAL
        // but go into the same PDS HAL table
        key.id[9] = 0x41;
    }
    return key;
}

// extract integer id from given 'sticky' uuid
static inline uint32_t
pdsobjkey2msidx (const pds_obj_key_t& key) {
    char *buf;
    static thread_local uint8_t next_buf = 0;
    char id_buf[4][9];

    buf = id_buf[next_buf++ & 0x3];
    memcpy(buf, key.id, 8);
    buf[8] = '\0';
    return std::stoul((const char *)buf, 0, 16);
}

static inline bool
is_pds_obj_key_invalid (const pds_obj_key_t& key) {
    static pds_obj_key_t invalid_key = {0};
    return (key == invalid_key);
}

enum class rt_type_e {
    EVI = 1,
    VRF
};

static inline char *
rt2str (const uint8_t *rt_str) {
    static thread_local char buf[3*AMB_BGP_EXT_COMM_LEN];
    sprintf (buf, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
             rt_str[0], rt_str[1], rt_str[2], rt_str[3],
             rt_str[4], rt_str[5], rt_str[6], rt_str[7]);
    return buf;
}
static inline char *
rt2msstr (const uint8_t *rt_str) {
    static thread_local char buf[2*AMB_BGP_EXT_COMM_LEN];
    sprintf (buf, "%02X%02X%02X%02X%02X%02X%02X%02X",
             rt_str[0], rt_str[1], rt_str[2], rt_str[3],
             rt_str[4], rt_str[5], rt_str[6], rt_str[7]);
    return buf;
}

struct ms_rt_t {
    uint8_t rt_str[AMB_BGP_EXT_COMM_LEN];

    ms_rt_t (const unsigned char rt_str_[]) {
        memcpy(rt_str,rt_str_,AMB_BGP_EXT_COMM_LEN);
    }
    ms_rt_t() = default;

    bool operator==(const ms_rt_t& other) const {
        if (!memcmp(rt_str, other.rt_str, AMB_BGP_EXT_COMM_LEN)) {
            return true;
        }
        return false;
    }
    char *str (void) const {
        return rt2str(rt_str);
    }
    char *ms_str (void) const {
        return rt2msstr(rt_str);
    }
    bool equal (const uint8_t rt_str_[]) {
        if (!memcmp(rt_str, rt_str_, AMB_BGP_EXT_COMM_LEN)) {
            return true;
        }
        return false;
    }
};

struct pend_rt_t {
    using ms_id_t=uint32_t;
    uint8_t rt_str[AMB_BGP_EXT_COMM_LEN];
    rt_type_e type;
    ms_id_t src_id; //subnet/vpc ms_id

    pend_rt_t (const unsigned char rt_str_[], rt_type_e type_, ms_id_t id_) {
        memcpy(rt_str, rt_str_, AMB_BGP_EXT_COMM_LEN);
        type = type_;
        src_id = id_;
    }
    pend_rt_t() = default;

    bool operator==(const ms_rt_t& other) const {
        if (!memcmp(rt_str, other.rt_str, AMB_BGP_EXT_COMM_LEN)) {
            return true;
        }
        return false;
    }

    char *str (void) const {
        return rt2str(rt_str);
    }
};

static inline bool mgmt_state_locked(bool set, bool lock=false) {
    static thread_local bool mgmt_state_lock_ = false;
    if (set) {
        mgmt_state_lock_ = lock;
    }
    return mgmt_state_lock_;
}

} // End namespace
#endif
