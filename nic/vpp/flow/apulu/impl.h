//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_APULU_IMPL_H__
#define __VPP_FLOW_APULU_IMPL_H__

#include <gen/p4gen/apulu/include/p4pd.h>

always_inline int
pds_session_get_advance_offset (void)
{
    return 0;
}

always_inline void
pds_session_prog_x2 (vlib_buffer_t **b, u32 session_id0,
                     u32 session_id1, u16 *next, u32 *counter)
{
    return;
}

always_inline void
pds_session_prog_x1 (vlib_buffer_t *b, u32 session_id,
                     u16 *next, u32 *counter)
{
    return;
}

always_inline int
pds_flow_prog_get_next_offset (vlib_buffer_t *p0)
{
    return -((vnet_buffer(p0)->l3_hdr_offset - vnet_buffer(p0)->l2_hdr_offset));
}

always_inline int
pds_flow_prog_get_next_node (void)
{
    return FLOW_PROG_NEXT_SESSION_PROG;
}

always_inline void
pds_flow_add_tx_hdrs_x2 (vlib_buffer_t *b0, vlib_buffer_t *b1)
{
    return;
}

always_inline void
pds_flow_add_tx_hdrs_x1 (vlib_buffer_t *b0)
{
    return;
}
static char *
pds_flow4_key2str (void *key)
{
    static char str[256] = {0};
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET_ADDRSTRLEN + 1];
    char dststr[INET_ADDRSTRLEN + 1];

    inet_ntop(AF_INET, k->key_metadata_src, srcstr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, k->key_metadata_dst, dststr, INET_ADDRSTRLEN);
    sprintf(str, "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u VNIC:%u",
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_bd_id);
    return str;
}

static char *
pds_flow6_key2str (void *key)
{
    static char str[256] = {0};
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET6_ADDRSTRLEN + 1];
    char dststr[INET6_ADDRSTRLEN + 1];

    inet_ntop(AF_INET6, k->key_metadata_src, srcstr, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, k->key_metadata_dst, dststr, INET6_ADDRSTRLEN);
    sprintf(str, "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u VNIC:%u",
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_bd_id);
    return str;
}

static char *
pds_flow_appdata2str (void *appdata)
{
    static char str[512] = {0};
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "session_id:%d flow_role:%d",
            d->session_id, d->flow_role);
    return str;
}

#endif    // __VPP_FLOW_APULU_IMPL_H__

