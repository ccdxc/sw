//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __FTE_FLOW_ARTEMIS_IMPL_H__
#define __FTE_FLOW_ARTEMIS_IMPL_H__

#include <gen/p4gen/artemis/include/p4pd.h>
#include <nic/apollo/p4/include/artemis_defines.h>
#include <stddef.h>

static char *
fte_flow4_key2str (void *key)
{
    static char str[256];
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET_ADDRSTRLEN + 1];
    char dststr[INET_ADDRSTRLEN + 1];

    inet_ntop(AF_INET, k->key_metadata_src, srcstr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, k->key_metadata_dst, dststr, INET_ADDRSTRLEN);
    sprintf(str, "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u VPC:%u",
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_vpc_id);
    return str;
}

static char *
fte_flow6_key2str (void *key)
{
    static char str[256];
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET6_ADDRSTRLEN + 1];
    char dststr[INET6_ADDRSTRLEN + 1];

    inet_ntop(AF_INET6, k->key_metadata_src, srcstr, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, k->key_metadata_dst, dststr, INET6_ADDRSTRLEN);
    sprintf(str, "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u VPC:%u",
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_vpc_id);
    return str;
}

static char *
fte_flow_appdata2str (void *appdata)
{
    static char str[512];
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "session_index:%d flow_role:%d",
            d->session_index, d->flow_role);
    return str;
}

static void
fte_ftlv4_set_session_index (ipv4_flow_hash_entry_t *entry, uint32_t session)
{
    entry->set_session_index(session);
}

#endif    // __FTE_FLOW_ARTEMIS_IMPL_H__
