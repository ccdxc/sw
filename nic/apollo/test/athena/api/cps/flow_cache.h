/*
 * {C} Copyright 2020 Pensando Systems Inc. All rights reserved
 *
 * athena vnic test implementation
 *
 */

#include "pds_flow_cache.h"

#define IDX_TO_KEY_TYPE(id) (pds_key_type_t)(idx%KEY_TYPE_L2)+1


static inline void fill_key_ip_tcp(pds_flow_key_t *key , uint32_t idx)
{
        uint32_t sip_addr = idx+1;
        uint32_t dip_addr = idx+2;

        key->key_type = KEY_TYPE_IPV4;
        memcpy(key->ip_saddr, &sip_addr, sizeof(uint32_t));
        memcpy(key->ip_daddr, &dip_addr, sizeof(uint32_t));
        key->ip_proto = 6;
        key->l4.tcp_udp.sport = (idx%65534)+1;
        key->l4.tcp_udp.dport = (idx%65534)+2; 
}
/*
void fill_key_ip_udp(pds_flow_key_t *key , uint32_t idx)
{
        uint32_t sip_addr = idx+1;
        uint32_t dip_addr = idx+2;

        key->key_type = KEY_TYPE_IPV4;
        memcpy(key->ip_saddr, &sip_addr, sizeof(uint32_t));
        memcpy(key->ip_daddr, &dip_addr, sizeof(uint32_t));
        key->ip_proto = 6;
        key->l4.tcp_udp.sport = (idx%65534)+1;
        key->l4.tcp_udp.dport = (idx%65534)+2; 
}

void fill_key_ip_icmp(pds_flow_key_t *key , uint32_t idx)
{
        uint32_t sip_addr = idx+1;
        uint32_t dip_addr = idx+2;

        key->key_type = KEY_TYPE_IPV4;
        memcpy(key->ip_saddr, &sip_addr, sizeof(uint32_t));
        memcpy(key->ip_daddr, &dip_addr, sizeof(uint32_t));
        key->ip_proto = 6;
        key->l4.tcp_udp.sport = (idx%65534)+1;
        key->l4.tcp_udp.dport = (idx%65534)+2; 
}

void fill_key_ipv6_tcp(pds_flow_key_t *key , uint32_t idx)
{
        uint32_t sip_addr = idx+1;
        uint32_t dip_addr = idx+2;

        key->key_type = KEY_TYPE_IPV4;
        memcpy(key->ip_saddr, &sip_addr, sizeof(uint32_t));
        memcpy(key->ip_daddr, &dip_addr, sizeof(uint32_t));
        key->ip_proto = 6;
        key->l4.tcp_udp.sport = (idx%65534)+1;
        key->l4.tcp_udp.dport = (idx%65534)+2; 
}

void fill_key_ipv6_udp(pds_flow_key_t *key , uint32_t idx)
{
        uint32_t sip_addr = idx+1;
        uint32_t dip_addr = idx+2;

        key->key_type = KEY_TYPE_IPV4;
        memcpy(key->ip_saddr, &sip_addr, sizeof(uint32_t));
        memcpy(key->ip_daddr, &dip_addr, sizeof(uint32_t));
        key->ip_proto = 6;
        key->l4.tcp_udp.sport = (idx%65534)+1;
        key->l4.tcp_udp.dport = (idx%65534)+2; 
}

void fill_key_ipv6_icmp(pds_flow_key_t *key , uint32_t idx)
{
        uint32_t sip_addr = idx+1;
        uint32_t dip_addr = idx+2;

        key->key_type = KEY_TYPE_IPV4;
        memcpy(key->ip_saddr, &sip_addr, sizeof(uint32_t));
        memcpy(key->ip_daddr, &dip_addr, sizeof(uint32_t));
        key->ip_proto = 6;
        key->l4.tcp_udp.sport = (idx%65534)+1;
        key->l4.tcp_udp.dport = (idx%65534)+2; 
}

typedef void (*FILL_FUNC) (pds_flow_key_t *, uint32_t);

FILL_FUNC fill_key[] = 
    {
        fill_key_ip_tcp,
        fill_key_ip_udp,
        fill_key_ip_icmp,
        fill_key_ipv6_tcp,
        fill_key_ipv6_udp,
        fill_key_ipv6_icmp
    };
#define FILL_KEY_MAX sizeof(fill_key)/sizeof(void *)
*/

static inline void
test_pds_flow_cache_entry_create(uint32_t idx) {

    pds_flow_spec_t  spec = {0};
    pds_ret_t        ret = PDS_RET_OK;

    //fill_key[idx%FILL_KEY_MAX](&spec.key, idx);
    fill_key_ip_tcp(&spec.key, idx);
    spec.data.index_type = (pds_flow_spec_index_type_t)(idx%2);
    spec.data.index = idx;

    ret = pds_flow_cache_entry_create(&spec);

    assert(ret == PDS_RET_OK);
}

static inline void
test_pds_flow_cache_entry_update(uint32_t idx) {

    pds_flow_spec_t  spec = {0};
    pds_ret_t        ret = PDS_RET_OK;

    //fill_key[idx%FILL_KEY_MAX](&spec.key, idx);
    fill_key_ip_tcp(&spec.key, idx);

    spec.data.index_type = (pds_flow_spec_index_type_t)((idx+1)%2);
    spec.data.index = idx;

    ret = pds_flow_cache_entry_update(&spec);
    assert(ret == PDS_RET_OK);
}

static inline void
test_pds_flow_cache_entry_read(uint32_t idx) {

    pds_flow_key_t   key = {0};
    pds_flow_info_t  info = {0};
    pds_ret_t        ret = PDS_RET_OK;

    //fill_key[idx%FILL_KEY_MAX](&key, idx);
    fill_key_ip_tcp(&key, idx);

    ret = pds_flow_cache_entry_read(&key, &info);
    assert((ret == PDS_RET_OK || 
            ret == PDS_RET_ENTRY_NOT_FOUND));
}

static inline void
test_pds_flow_cache_entry_delete(uint32_t idx) {

    pds_flow_key_t   key = {0};
    pds_ret_t        ret = PDS_RET_OK;

    //fill_key[idx%FILL_KEY_MAX](&key, idx);
    fill_key_ip_tcp(&key, idx);

    ret = pds_flow_cache_entry_delete(&key);
    assert(ret == PDS_RET_OK);
}
