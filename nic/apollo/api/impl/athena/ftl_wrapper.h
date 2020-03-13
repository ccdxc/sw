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

static inline void
ftlv6_set_index (flow_hash_entry_t *entry, uint32_t index)
{
    return entry->set_index(index);
}

static inline void
ftlv6_set_index_type (flow_hash_entry_t *entry, uint8_t index_type)
{
    return entry->set_index_type(index_type);
}

static inline void
ftlv6_set_key_vnic_id (flow_hash_entry_t *entry, uint16_t vnic_id)
{
    /* FIXME: FTL generated code has the bit positions flipped
     * using hand-coded implementation for now
     */
    //return entry->set_key_metadata_vnic_id(vnic_id);
    entry->key_metadata_vnic_id_sbit8_ebit8 = (vnic_id & 0x01);
    entry->key_metadata_vnic_id_sbit0_ebit7 = (vnic_id >> 1) & 0xFF;
    return;
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
    return entry->get_index();
}

static inline uint8_t
ftlv6_get_index_type (flow_hash_entry_t *entry)
{
    return entry->get_index_type();
}

static inline uint16_t
ftlv6_get_key_vnic_id (flow_hash_entry_t *entry)
{
    uint16_t vnic_id = 0;
    /* FIXME: FTL generated code has the bit positions flipped
     * using hand-coded implementation for now
     */
    //return entry->get_key_metadata_vnic_id();
    vnic_id |= (entry->key_metadata_vnic_id_sbit0_ebit7 << 1);
    vnic_id |= entry->key_metadata_vnic_id_sbit8_ebit8;
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

#ifdef __cplusplus
}
#endif

#endif // __FTL_WRAPPER_H__
