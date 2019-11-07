//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_APULU_IMPL_H__
#define __VPP_FLOW_APULU_IMPL_H__

#include <gen/p4gen/apulu/include/p4pd.h>

always_inline int
pds_session_get_advance_offset (void)
{
    return (APULU_P4_TO_ARM_HDR_SZ - APULU_ARM_TO_P4_HDR_SZ);
}

always_inline void
pds_session_prog_x2 (vlib_buffer_t **b, u32 session_id0,
                     u32 session_id1, u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *ses_info0 = vlib_buffer_get_current(b[0]);
    p4_rx_cpu_hdr_t *ses_info1 = vlib_buffer_get_current(b[1]);
    static session_actiondata_t actiondata = {0};

    actiondata.action_id = SESSION_SESSION_INFO_ID;
    actiondata.action_u.session_session_info.tx_rewrite_flags =
            ((TX_REWRITE_DMAC_FROM_MAPPING << TX_REWRITE_DMAC_START) |
                    (TX_REWRITE_ENCAP_VXLAN << TX_REWRITE_ENCAP_START));
    actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START) |
                    (RX_REWRITE_ENCAP_VLAN << RX_REWRITE_ENCAP_START));

    if (PREDICT_FALSE(session_program(session_id0, (void *)&actiondata))) {
        next[0] = SESSION_PROG_NEXT_DROP;
    } else {
        next[0] = SESSION_PROG_NEXT_FWD_FLOW;
        /* Reuse flowhash field to store lif id for packet tx
         * as we have limited packet metadata fields
         */
        vnet_buffer(b[0])->pds_data.flow_hash =
                clib_net_to_host_u16(ses_info0->lif);
    }
    if (PREDICT_FALSE(session_program(session_id1, (void *)&actiondata))) {
        next[1] = SESSION_PROG_NEXT_DROP;
    } else {
        next[1] = SESSION_PROG_NEXT_FWD_FLOW;
        /* Reuse flowhash field to store lif id for packet tx
         * as we have limited packet metadata fields
         */
        vnet_buffer(b[1])->pds_data.flow_hash =
                clib_net_to_host_u16(ses_info1->lif);
    }

    vlib_buffer_advance(b[0], pds_session_get_advance_offset());
    vlib_buffer_advance(b[1], pds_session_get_advance_offset());
}

always_inline void
pds_session_prog_x1 (vlib_buffer_t *b, u32 session_id,
                     u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *ses_info0 = vlib_buffer_get_current(b);
    static session_actiondata_t actiondata = {0};

    actiondata.action_id = SESSION_SESSION_INFO_ID;
    actiondata.action_u.session_session_info.tx_rewrite_flags =
            ((TX_REWRITE_DMAC_FROM_MAPPING << TX_REWRITE_DMAC_START) |
                    (TX_REWRITE_ENCAP_VXLAN << TX_REWRITE_ENCAP_START));
    actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START) |
                    (RX_REWRITE_ENCAP_VLAN << RX_REWRITE_ENCAP_START));

    if (PREDICT_FALSE(session_program(session_id, (void *)&actiondata))) {
        next[0] = SESSION_PROG_NEXT_DROP;
    } else {
        next[0] = SESSION_PROG_NEXT_FWD_FLOW;
        /* Reuse flowhash field to store lif id for packet tx
         * as we have limited packet metadata fields
         */
        vnet_buffer(b)->pds_data.flow_hash =
                clib_net_to_host_u16(ses_info0->lif);
    }
    vlib_buffer_advance(b, pds_session_get_advance_offset());
}

always_inline int
pds_flow_prog_get_next_offset (vlib_buffer_t *p0)
{
    return -(APULU_P4_TO_ARM_HDR_SZ +
            (vnet_buffer(p0)->l3_hdr_offset - vnet_buffer(p0)->l2_hdr_offset));
}

always_inline int
pds_flow_prog_get_next_node (void)
{
    return FLOW_PROG_NEXT_SESSION_PROG;
}

always_inline void
pds_flow_add_tx_hdrs_x2 (vlib_buffer_t *b0, vlib_buffer_t *b1)
{
    p4_tx_cpu_hdr_t *tx0, *tx1;

    tx0 = vlib_buffer_get_current(b0);
    tx1 = vlib_buffer_get_current(b1);
    tx0->lif = vnet_buffer(b0)->pds_data.flow_hash & 0x7ff;
    tx1->lif = vnet_buffer(b1)->pds_data.flow_hash & 0x7ff;
    tx0->pad = 0;
    tx1->pad = 0;
    tx0->lif_pad = clib_net_to_host_u16(tx0->lif_pad);
    tx1->lif_pad = clib_net_to_host_u16(tx1->lif_pad);
}

always_inline void
pds_flow_add_tx_hdrs_x1 (vlib_buffer_t *b0)
{
    p4_tx_cpu_hdr_t *tx0;

    tx0 = vlib_buffer_get_current(b0);
    tx0->lif = vnet_buffer(b0)->pds_data.flow_hash & 0x7ff;
    tx0->pad = 0;
    tx0->lif_pad = clib_net_to_host_u16(tx0->lif_pad);
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

always_inline void
pds_flow_extract_nexthop_info(void * local_entry,
                              void * remote_entry,
                              vlib_buffer_t *p0,
                              u8 is_ip4)
{
    u32 nexthop = vnet_buffer(p0)->pds_data.nexthop;

    //TODO: Rflow nexthop
    if (is_ip4) {
        ftlv4_entry_t *local = (ftlv4_entry_t *)local_entry;
        if (nexthop) {
            local->nexthop_id = nexthop & 0xff;
            local->nexthop_type = (nexthop >> 16) & 0x3;
            local->nexthop_valid = 1;
        } else {
            local->nexthop_id = local->nexthop_type = local->nexthop_valid = 0;
        }
    } else {
        ftlv6_entry_t *local = (ftlv6_entry_t *)local_entry;
        if (nexthop) {
            local->nexthop_id = nexthop & 0xff;
            local->nexthop_type = (nexthop >> 16) & 0x3;
            local->nexthop_valid = 1;
        } else {
            local->nexthop_id = local->nexthop_type = local->nexthop_valid = 0;
        }
    }

}

#endif    // __VPP_FLOW_APULU_IMPL_H__
