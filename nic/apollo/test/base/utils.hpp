//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_BASE_UTILS_HPP__
#define __TEST_BASE_UTILS_HPP__

#include <vector>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/platform/capri/capri_p4.hpp" //UPLINK_0/1
#include "nic/apollo/api/include/pds.hpp"
#ifndef SIM
#include "nic/sdk/platform/fru/fru.hpp"
#endif

namespace test {

// todo: @kalyanbade delete this once all references are gone
typedef enum op_e {
    OP_NONE,            ///< None
    OP_MANY_CREATE,     ///< Create
    OP_MANY_READ,       ///< Read
    OP_MANY_UPDATE,     ///< Update
    OP_MANY_DELETE,     ///< Delete
    OP_INVALID,         ///< Invalid
} utils_op_t;

//----------------------------------------------------------------------------
// ip address related helper routines
//----------------------------------------------------------------------------

static inline int
ip_version (const char *ip)
{
    char buf[16];

    if (inet_pton(AF_INET, ip, buf)) {
        return IP_AF_IPV4;
    } else if (inet_pton(AF_INET6, ip, buf)) {
        return IP_AF_IPV6;
    }
    return -1;
}

static inline void
extract_ipv4_addr (const char *ip, ipv4_addr_t *ip_addr)
{
    int af;
    ip_prefix_t pfx;

    *ip_addr = 0;
    af = ip_version(ip);
    if (af == IP_AF_IPV4) {
        SDK_ASSERT(str2ipv4pfx((char *)ip, &pfx) == 0);
        *ip_addr = pfx.addr.addr.v4_addr;
    } else {
        SDK_ASSERT(0);
    }
}

static inline void
extract_ip_addr (const char *ip, ip_addr_t *ip_addr)
{
    int af;
    ip_prefix_t pfx;

    memset(&pfx, 0x0, sizeof(ip_prefix_t));
    af = ip_version(ip);
    if (af == IP_AF_IPV4) {
        SDK_ASSERT(str2ipv4pfx((char *)ip, &pfx) == 0);
        *ip_addr = pfx.addr;
    } else if (af == IP_AF_IPV6) {
        SDK_ASSERT(str2ipv6pfx((char *)ip, &pfx) == 0);
        *ip_addr = pfx.addr;
    } else {
        SDK_ASSERT(0);
    }
}

static inline void
extract_ip_pfx (const char *str, ip_prefix_t *ip_pfx)
{
    char ip[64];
    char *slash;
    int af;

    memset(ip_pfx, 0, sizeof(*ip_pfx));
    // Input may get modified. Copying hence
    strncpy(ip, str, sizeof(ip) - 1);
    slash = strchr(ip, '/');
    if (slash != NULL) {
        *slash = '\0';
    }
    af = ip_version(ip);
    if (slash != NULL) {
        *slash = '/';
    }
    if (af == IP_AF_IPV4) {
        SDK_ASSERT(str2ipv4pfx((char *)ip, ip_pfx) == 0);
    } else if (af == IP_AF_IPV6) {
        SDK_ASSERT(str2ipv6pfx((char *)ip, ip_pfx) == 0);
    } else {
        SDK_ASSERT(0);
    }
}

static inline void
increment_ip_addr (ip_addr_t *ipaddr, int width = 1)
{
    switch (ipaddr->af) {
    case IP_AF_IPV4:
        ipaddr->addr.v4_addr += width;
        break;
    case IP_AF_IPV6:
        if (likely(!((ipaddr->addr.v6_addr.addr64[0] == ((uint64_t)-1)) &&
                     (ipaddr->addr.v6_addr.addr64[1] == ((uint64_t)-1))))) {
            for (uint8_t i = IP6_ADDR8_LEN - 1; i >= 0 ; i--) {
                // keep adding one until there is no rollover
                if ((++(ipaddr->addr.v6_addr.addr8[i]))) {
                    break;
                }
            }
        }
        break;
    default:
        SDK_ASSERT(0);
    }
}

static inline uint32_t
pds_get_next_addr16 (uint32_t addr)
{
    uint16_t base;

    base = addr >> 16;
    base++;
    return base << 16;
}

//----------------------------------------------------------------------------
// mac address related helper routines
//----------------------------------------------------------------------------

static inline void
increment_mac_addr (mac_addr_t macaddr, int width = 1)
{
    uint64_t mac = MAC_TO_UINT64(macaddr) + width;
    MAC_UINT64_TO_ADDR(macaddr, mac);
}

//----------------------------------------------------------------------------
// encap related helper routines
//----------------------------------------------------------------------------

// check if given two encaps are same. Returns true if equal, otherwise false
static inline bool
pdsencap_isequal (const pds_encap_t *encap1, const pds_encap_t *encap2)
{
    // compare encap type
    if (encap1->type != encap2->type) {
        return FALSE;
    }

    // compare encap value
    switch (encap1->type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        if (encap1->val.vlan_tag != encap2->val.vlan_tag) {
            return FALSE;
        }
        break;
    case PDS_ENCAP_TYPE_QINQ:
        if ((encap1->val.qinq_tag.c_tag != encap2->val.qinq_tag.c_tag) ||
            (encap1->val.qinq_tag.s_tag != encap2->val.qinq_tag.s_tag)) {
            return FALSE;
        }
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        if (encap1->val.mpls_tag != encap2->val.mpls_tag) {
            return FALSE;
        }
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        if (encap1->val.vnid != encap2->val.vnid) {
            return FALSE;
        }
        break;
    default:
        if (encap1->val.value != encap2->val.value) {
            return FALSE;
        }
        break;
    }
    return TRUE;
}

static inline void
increment_encap (pds_encap_t *encap, int width)
{
    switch (encap->type) {
    case PDS_ENCAP_TYPE_MPLSoUDP:
        encap->val.mpls_tag += width;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        encap->val.vnid += width;
        break;
    case PDS_ENCAP_TYPE_DOT1Q:
        encap->val.vlan_tag += width;
        break;
    default:
        encap->val.value += width;
    }
}

static inline pds_obj_key_t
int2pdsobjkey (uint32_t id) {
    pds_obj_key_t key = { 0 };
    std::string id_str = std::to_string(id);

    memcpy(key.id, id_str.data(), id_str.length());
    return key;
}

static inline uint32_t
pdsobjkey2int (pds_obj_key_t& key) {
    return (uint32_t)atoi((const char *)key.id);
}

// function prototypes
void send_packet(const uint8_t *tx_pkt, uint32_t tx_pkt_len, uint32_t tx_port,
                 const uint8_t *exp_rx_pkt, uint32_t exp_rx_pkt_len,
                 uint32_t exp_rx_port);
void dump_packet(std::vector<uint8_t> data);

inline std::ostream&
operator<<(std::ostream& os, const pds_obj_key_t *key) {
    os << "id: " << std::string(key->str());
    return os;
}

// construct a 'sticky' uuid given an integer so that same uuid is generated
// even across reboots i.e., same input gives same uuid everytime
#define PDS_UUID_MAGIC_BYTE           0x42
#define PDS_UUID_MAGIC_BYTE_OFFSET    8
#define PDS_UUID_SYSTEM_MAC_OFFSET    10
static inline pds_obj_key_t
uuid_from_objid (uint32_t id)
{
    pds_obj_key_t key = { 0 };
    mac_addr_t    system_mac;

    memcpy(&key.id[0], &id, sizeof(id));
    memset(&key.id[8], 0x42, 2);
#ifdef SIM
    MAC_UINT64_TO_ADDR(system_mac, PENSANDO_NIC_MAC);
#else
    std::string   mac_str;
    sdk::platform::readFruKey(MACADDRESS_KEY, mac_str);
    mac_str_to_addr((char *)mac_str.c_str(), system_mac);
#endif
    memcpy(&key.id[10], system_mac, ETH_ADDR_LEN);

    return key;
}

// extract integer id from given 'sticky' uuid
static inline uint32_t
objid_from_uuid (const pds_obj_key_t& key) {
    char *buf;
    char id_buf[4][9];
    static thread_local uint8_t next_buf = 0;

    buf = id_buf[next_buf++ & 0x3];
    memcpy(buf, key.id, 8);
    buf[8] = '\0';
    return *(uint32_t *)buf;
}

}    // namespace test

#endif    // __TEST_BASE_UTILS_HPP__
