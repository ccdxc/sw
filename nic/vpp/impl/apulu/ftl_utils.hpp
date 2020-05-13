//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_FTL_UTILS_H__
#define __VPP_IMPL_APULU_FTL_UTILS_H__

void
ftlv4_set_nexthop(ipv4_flow_hash_entry_t *entry,
                  uint32_t nexthop,
                  uint32_t nexthop_type,
                  uint8_t nexthop_valid,
                  uint8_t priority)
{
    entry->set_nexthop_id(nexthop);
    entry->set_nexthop_type(nexthop_type);
    entry->set_nexthop_valid(nexthop_valid);
    entry->set_priority(priority);
}

void
ftl_set_nexthop(flow_hash_entry_t *entry,
                uint32_t nexthop,
                uint32_t nexthop_type,
                uint8_t nexthop_valid,
                uint8_t priority)
{
    entry->set_nexthop_id(nexthop);
    entry->set_nexthop_type(nexthop_type);
    entry->set_nexthop_valid(nexthop_valid);
    entry->set_priority(priority);
}

void
ftlv4_set_lookup_id(ipv4_flow_hash_entry_t *entry, 
                    uint16_t lookup_id)
{
    entry->set_key_metadata_flow_lkp_id(lookup_id);
}

void
ftl_set_lookup_id(flow_hash_entry_t *entry, 
                  uint16_t lookup_id)
{
    entry->set_key_metadata_flow_lkp_id(lookup_id);
}

void
ftlv4_set_flow_miss_hit(ipv4_flow_hash_entry_t *entry, uint8_t val)
{
    entry->set_force_flow_miss(val);
}

void
ftl_set_flow_miss_hit(flow_hash_entry_t *entry, uint8_t val)
{
    entry->set_force_flow_miss(val);
}

void
ftlv4_set_l2l(ipv4_flow_hash_entry_t *entry, uint8_t val)
{
    entry->set_is_local_to_local(val);
}

void
ftl_set_l2l(flow_hash_entry_t *entry, uint8_t val)
{
    entry->set_is_local_to_local(val);
}

uint16_t
ftlv4_get_lookup_id(ipv4_flow_hash_entry_t *entry)
{
    return entry->get_key_metadata_flow_lkp_id();
}

uint16_t
ftl_get_lookup_id(flow_hash_entry_t *entry)
{
    return entry->get_key_metadata_flow_lkp_id();
}


uint8_t
ftlv4_get_epoch(ipv4_flow_hash_entry_t *entry)
{
    return entry->get_epoch();
}

uint8_t
ftl_get_epoch(flow_hash_entry_t *entry)
{
    return entry->get_epoch();
}

uint8_t
ftlv4_get_nexthop_valid(ipv4_flow_hash_entry_t *entry)
{
    return entry->get_nexthop_valid();
}

uint8_t
ftl_get_nexthop_valid(flow_hash_entry_t *entry)
{
    return entry->get_nexthop_valid();
}

uint8_t
ftlv4_get_nexthop_type(ipv4_flow_hash_entry_t *entry)
{
    return entry->get_nexthop_type();
}

uint8_t
ftl_get_nexthop_type(flow_hash_entry_t *entry)
{
    return entry->get_nexthop_type();
}

uint16_t
ftlv4_get_nexthop_id(ipv4_flow_hash_entry_t *entry)
{
    return entry->get_nexthop_id();
}

uint16_t
ftl_get_nexthop_id(flow_hash_entry_t *entry)
{
    return entry->get_nexthop_id();
}

uint8_t
ftlv4_get_nexthop_priority(ipv4_flow_hash_entry_t *entry)
{
    return entry->get_priority();
}

uint8_t
ftl_get_nexthop_priority(flow_hash_entry_t *entry)
{
    return entry->get_priority();
}

uint8_t
ftlv4_get_l2l(ipv4_flow_hash_entry_t *entry)
{
    return entry->get_is_local_to_local();
}

uint8_t
ftl_get_l2l(flow_hash_entry_t *entry)
{
    return entry->get_is_local_to_local();
}

#endif  // __VPP_IMPL_APULU_FTL_UTILS_H__
