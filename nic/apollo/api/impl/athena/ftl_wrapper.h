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
ftlv6_set_session_index (flow_hash_entry_t *entry, uint32_t session)
{
    return entry->set_session_index(session);
}

static inline void
ftlv6_set_vnic_id (flow_hash_entry_t *entry, uint16_t vnic_id)
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
ftlv6_set_key_proto (flow_hash_entry_t *entry, uint64_t proto)
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
ftlv6_get_session_id (flow_hash_entry_t *entry)
{
    return entry->get_session_index();
}

#ifdef __cplusplus
}
#endif

#endif // __FTL_WRAPPER_H__
