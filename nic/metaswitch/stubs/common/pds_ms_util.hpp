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

// generate a sticky uuid from an interger id
// same id always returs the same uuid
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

#define PDS_UUID_MAGIC_BYTE           0x42
#define PDS_UUID_MAGIC_BYTE_OFFSET    8
#define PDS_UUID_SYSTEM_MAC_OFFSET    10
static inline pds_obj_key_t
uuid_from_msid (uint32_t id)
{
    pds_obj_key_t key = { 0 };
    mac_addr_t    system_mac;

    MAC_UINT64_TO_ADDR(system_mac, PENSANDO_NIC_MAC);
    memcpy(&key.id[0], &id, sizeof(id));
    memset(&key.id[8], 0x42, 2);
    memcpy(&key.id[10], system_mac, ETH_ADDR_LEN);
    return key;
}

// extract integer id from given 'sticky' uuid
static inline uint32_t
msid_from_uuid (const pds_obj_key_t& key) {
    char *buf;
    char id_buf[4][9];
    static thread_local uint8_t next_buf = 0;

    buf = id_buf[next_buf++ & 0x3];
    memcpy(buf, key.id, 8);
    buf[8] = '\0';
    return *(uint32_t *)buf;
}

static inline bool
is_pds_obj_key_invalid (const pds_obj_key_t& key) {
    static pds_obj_key_t invalid_key = {0};
    return (key == invalid_key);
}
} // End namespace
#endif
