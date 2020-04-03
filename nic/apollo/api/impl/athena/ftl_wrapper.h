//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// wrapper functions to ftl
///
//----------------------------------------------------------------------------

#ifndef __FTL_WRAPPER_H__
#define __FTL_WRAPPER_H__

#include "gen/p4gen/p4/include/ftl_table.hpp"

#ifdef __cplusplus
extern "C" {
#endif

// Flow hash table set/get API wrappers
static inline void
ftlv6_set_index (flow_hash_entry_t *entry, uint32_t index)
{
    return entry->set_idx(index);
}

static inline void
ftlv6_set_index_type (flow_hash_entry_t *entry, uint8_t index_type)
{
    return entry->set_idx_type(index_type);
}

static inline void
ftlv6_set_key_vnic_id (flow_hash_entry_t *entry, uint16_t vnic_id)
{
    return entry->set_key_metadata_vnic_id(vnic_id);
}

static inline void
ftlv6_set_key_sport (flow_hash_entry_t *entry, uint16_t sport)
{
    return entry->set_key_metadata_sport(sport);
}

static inline void
ftlv6_set_key_dport (flow_hash_entry_t *entry, uint16_t dport)
{
    return entry->set_key_metadata_dport(dport);
}

static inline void
ftlv6_set_key_ktype (flow_hash_entry_t *entry, uint8_t ktype)
{
    return entry->set_key_metadata_ktype(ktype);
}

static inline void
ftlv6_set_key_proto (flow_hash_entry_t *entry, uint16_t proto)
{
    return entry->set_key_metadata_proto(proto);
}

static inline void
ftlv6_set_key_src_ip (flow_hash_entry_t *entry, uint8_t *src)
{
    return entry->set_key_metadata_src(src);
}

static inline void
ftlv6_set_key_dst_ip (flow_hash_entry_t *entry, uint8_t *dst)
{
    return entry->set_key_metadata_dst(dst);
}

static inline uint32_t
ftlv6_get_index (flow_hash_entry_t *entry)
{
    return entry->get_idx();
}

static inline uint8_t
ftlv6_get_index_type (flow_hash_entry_t *entry)
{
    return entry->get_idx_type();
}

static inline uint16_t
ftlv6_get_key_vnic_id (flow_hash_entry_t *entry)
{
    uint16_t vnic_id = 0;

    // ftl generated code misses out on msb
    vnic_id |= (entry->key_metadata_vnic_id_sbit0_ebit7 << 1) & 0x1ff;
    vnic_id |= (entry->key_metadata_vnic_id_sbit8_ebit8 << 0) & 0x1;
    return vnic_id;
}

static inline uint16_t
ftlv6_get_key_sport (flow_hash_entry_t *entry)
{
    return entry->get_key_metadata_sport();
}

static inline uint16_t
ftlv6_get_key_dport (flow_hash_entry_t *entry)
{
    return entry->get_key_metadata_dport();
}

static inline uint8_t
ftlv6_get_key_ktype (flow_hash_entry_t *entry)
{
    return entry->get_key_metadata_ktype();
}

static inline uint16_t
ftlv6_get_key_proto (flow_hash_entry_t *entry)
{
    return entry->get_key_metadata_proto();
}

static inline void
ftlv6_get_key_src_ip (flow_hash_entry_t *entry, uint8_t *src)
{
    return entry->get_key_metadata_src(src);
}

static inline void
ftlv6_get_key_dst_ip (flow_hash_entry_t *entry, uint8_t *dst)
{
    return entry->get_key_metadata_dst(dst);
}

// DNAT hash table set/get API wrappers
static inline void
dnat_set_map_ip (dnat_entry_t *entry, uint8_t *ipaddr)
{
    return entry->set_addr(ipaddr);
}

static inline void
dnat_set_map_addr_type (dnat_entry_t *entry, uint8_t addr_type)
{
    return entry->set_addr_type(addr_type);
}

static inline void
dnat_set_map_epoch (dnat_entry_t *entry, uint16_t epoch)
{
    return entry->set_epoch(epoch);
}

static inline void
dnat_set_key_ip (dnat_entry_t *entry, uint8_t *ipaddr)
{
    return entry->set_key_metadata_src(ipaddr);
}

static inline void
dnat_set_key_vnic_id (dnat_entry_t *entry, uint16_t vnic_id)
{
    return entry->set_key_metadata_vnic_id(vnic_id);
}

static inline void
dnat_set_key_ktype (dnat_entry_t *entry, uint8_t ktype)
{
    return entry->set_key_metadata_ktype(ktype);
}

static inline void
dnat_get_map_ip (dnat_entry_t *entry, uint8_t *ipaddr)
{
    return entry->get_addr(ipaddr);
}

static inline uint8_t
dnat_get_map_addr_type (dnat_entry_t *entry)
{
    return entry->get_addr_type();
}

static inline uint8_t
dnat_get_map_epoch(dnat_entry_t *entry)
{
    return entry->get_epoch();
}

static inline void
dnat_get_key_ip (dnat_entry_t *entry, uint8_t *ipaddr)
{
    return entry->get_key_metadata_src(ipaddr);
}

static inline uint16_t
dnat_get_key_vnic_id (dnat_entry_t *entry)
{
    return entry->get_key_metadata_vnic_id();
}

static inline uint8_t
dnat_get_key_ktype (dnat_entry_t *entry)
{
    return entry->get_key_metadata_ktype();
}

#ifdef __cplusplus
}
#endif

#endif // __FTL_WRAPPER_H__
