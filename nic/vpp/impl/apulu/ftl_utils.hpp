//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_FTL_UTILS_H__
#define __VPP_IMPL_APULU_FTL_UTILS_H__

void
ftlv4_set_nexthop(ipv4_flow_hash_entry_t *entry,
                  uint32_t nexthop,
                  uint32_t nexthop_type,
                  uint8_t nexthop_valid)
{
    entry->set_nexthop_id(nexthop);
    entry->set_nexthop_type(nexthop_type);
    entry->set_nexthop_valid(nexthop_valid);
}

void
ftlv6_set_nexthop(flow_hash_entry_t *entry,
                  uint32_t nexthop,
                  uint32_t nexthop_type,
                  uint8_t nexthop_valid)
{
    entry->set_nexthop_id(nexthop);
    entry->set_nexthop_type(nexthop_type);
    entry->set_nexthop_valid(nexthop_valid);
}

void
ftlv4_set_lookup_id(ipv4_flow_hash_entry_t *entry, 
                    uint16_t lookup_id)
{
    entry->set_vnic_metadata_bd_id(lookup_id);
}

void
ftlv6_set_lookup_id(flow_hash_entry_t *entry, 
                    uint16_t lookup_id)
{
    entry->set_vnic_metadata_bd_id(lookup_id);
}

#endif  // __VPP_IMPL_APULU_FTL_UTILS_H__
