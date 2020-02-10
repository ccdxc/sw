//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_FLOW_H__
#define __VPP_IMPL_APULU_FLOW_H__

#include <gen/p4gen/apulu/include/p4pd.h>
#include <api.h>
#include <impl_db.h>
#include <nic/apollo/api/impl/apulu/nacl_data.h>
#include <nic/vpp/impl/nat.h>
#include "p4_cpu_hdr_utils.h"

always_inline u32
pds_session_get_max (void)
{
    // session table is 2M - 1 as 0 is reserved.
    // so index is 0 to (2 *1024 * 1024) - 2
    return 2097150;
}

extern pds_flow_main_t pds_flow_main;

always_inline int
pds_session_get_advance_offset (void)
{
    return (APULU_P4_TO_ARM_HDR_SZ - APULU_ARM_TO_P4_HDR_SZ);
}

always_inline void
pds_session_prog_x2 (vlib_buffer_t *b0, vlib_buffer_t *b1,
                     u32 session_id0, u32 session_id1,
                     u16 *next0, u16 *next1, u32 *counter)
{
    p4_rx_cpu_hdr_t *ses_info0 = vlib_buffer_get_current(b0);
    p4_rx_cpu_hdr_t *ses_info1 = vlib_buffer_get_current(b1);
    session_actiondata_t actiondata = {0};
    pds_flow_main_t *fm = &pds_flow_main;
    u16 tx_rewrite_flags0 = 0, tx_rewrite_flags1 = 0;

    actiondata.action_id = SESSION_SESSION_INFO_ID;
    if (vnet_buffer(b0)->pds_flow_data.flags & VPP_CPU_FLAGS_NAPT_VALID) {
        // NAPT - rewrite both ip and port
        tx_rewrite_flags0 =
            ((TX_REWRITE_SIP_FROM_NAT << TX_REWRITE_SIP_START) |
             (TX_REWRITE_SPORT_FROM_NAT << TX_REWRITE_SPORT_START));
        // rewrite dmaci from nexthop, since there will be no mapping
        // lookup in rx direction
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DIP_FROM_NAT << RX_REWRITE_DIP_START) |
             (RX_REWRITE_DPORT_FROM_NAT << RX_REWRITE_DPORT_START) |
             (RX_REWRITE_DMAC_FROM_NEXTHOP << RX_REWRITE_DMAC_START) |
             (RX_REWRITE_SMAC_FROM_VRMAC << RX_REWRITE_SMAC_START));
        actiondata.action_u.session_session_info.tx_xlate_id =
            vnet_buffer2(b0)->pds_nat_data.xlate_idx;
        actiondata.action_u.session_session_info.rx_xlate_id =
            vnet_buffer2(b0)->pds_nat_data.xlate_idx_rflow;
    } else if (vnet_buffer2(b0)->pds_nat_data.xlate_idx) {
        // static NAT - rewrite only ip
        // TODO: service mapping, flow miss from uplink
        tx_rewrite_flags0 =
            (TX_REWRITE_SIP_FROM_NAT << TX_REWRITE_SIP_START);
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DIP_FROM_NAT << RX_REWRITE_DIP_START) |
             (RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START));
        actiondata.action_u.session_session_info.tx_xlate_id =
            vnet_buffer2(b0)->pds_nat_data.xlate_idx;
        actiondata.action_u.session_session_info.rx_xlate_id =
            vnet_buffer2(b0)->pds_nat_data.xlate_idx + 1;
    } else {
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            (RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START);
    }
    actiondata.action_u.session_session_info.tx_rewrite_flags =
        tx_rewrite_flags0 |
        vec_elt(fm->nh_flags, (vnet_buffer(b0)->pds_flow_data.nexthop) >> 16);

    if (PREDICT_FALSE(session_program(session_id0, (void *)&actiondata))) {
        *next0 = SESSION_PROG_NEXT_DROP;
    } else {
        *next0 = SESSION_PROG_NEXT_FWD_FLOW;
        vnet_buffer(b0)->pds_flow_data.lif = ses_info0->lif;
    }
    clib_memset(&actiondata, 0, sizeof(actiondata));
    if (vnet_buffer(b1)->pds_flow_data.flags & VPP_CPU_FLAGS_NAPT_VALID) {
        // NAPT - rewrite both ip and port
        tx_rewrite_flags1 =
            ((TX_REWRITE_SIP_FROM_NAT << TX_REWRITE_SIP_START) |
             (TX_REWRITE_SPORT_FROM_NAT << TX_REWRITE_SPORT_START));
        // rewrite dmaci from nexthop, since there will be no mapping
        // lookup in rx direction
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DIP_FROM_NAT << RX_REWRITE_DIP_START) |
             (RX_REWRITE_DPORT_FROM_NAT << RX_REWRITE_DPORT_START) |
             (RX_REWRITE_DMAC_FROM_NEXTHOP << RX_REWRITE_DMAC_START) |
             (RX_REWRITE_SMAC_FROM_VRMAC << RX_REWRITE_SMAC_START));
        actiondata.action_u.session_session_info.tx_xlate_id =
            vnet_buffer2(b1)->pds_nat_data.xlate_idx;
        actiondata.action_u.session_session_info.rx_xlate_id =
            vnet_buffer2(b1)->pds_nat_data.xlate_idx_rflow;
    } else if (vnet_buffer2(b1)->pds_nat_data.xlate_idx) {
        // static NAT - rewrite only ip
        // TODO: service mapping, flow miss from uplink
        tx_rewrite_flags1 =
            (TX_REWRITE_SIP_FROM_NAT << TX_REWRITE_SIP_START);
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DIP_FROM_NAT << RX_REWRITE_DIP_START) |
             (RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START));
        actiondata.action_u.session_session_info.tx_xlate_id =
            vnet_buffer2(b1)->pds_nat_data.xlate_idx;
        actiondata.action_u.session_session_info.rx_xlate_id =
            vnet_buffer2(b1)->pds_nat_data.xlate_idx + 1;
    } else {
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            (RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START);
    }
    actiondata.action_u.session_session_info.tx_rewrite_flags =
        tx_rewrite_flags1 |
        vec_elt(fm->nh_flags, (vnet_buffer(b1)->pds_flow_data.nexthop) >> 16);
    if (PREDICT_FALSE(session_program(session_id1, (void *)&actiondata))) {
        *next1 = SESSION_PROG_NEXT_DROP;
    } else {
        *next1 = SESSION_PROG_NEXT_FWD_FLOW;
        vnet_buffer(b1)->pds_flow_data.lif = ses_info1->lif;
    }

    vlib_buffer_advance(b0, pds_session_get_advance_offset());
    vlib_buffer_advance(b1, pds_session_get_advance_offset());
}

always_inline void
pds_session_prog_x1 (vlib_buffer_t *b, u32 session_id,
                     u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *ses_info0 = vlib_buffer_get_current(b);
    session_actiondata_t actiondata = {0};
    pds_flow_main_t *fm = &pds_flow_main;
    u16 tx_rewrite_flags = 0;

    actiondata.action_id = SESSION_SESSION_INFO_ID;
    if (vnet_buffer(b)->pds_flow_data.flags & VPP_CPU_FLAGS_NAPT_VALID) {
        // NAPT - rewrite both ip and port
        tx_rewrite_flags =
            ((TX_REWRITE_SIP_FROM_NAT << TX_REWRITE_SIP_START) |
             (TX_REWRITE_SPORT_FROM_NAT << TX_REWRITE_SPORT_START));
        // rewrite dmaci from nexthop, since there will be no mapping
        // lookup in rx direction
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DIP_FROM_NAT << RX_REWRITE_DIP_START) |
             (RX_REWRITE_DPORT_FROM_NAT << RX_REWRITE_DPORT_START) |
             (RX_REWRITE_DMAC_FROM_NEXTHOP << RX_REWRITE_DMAC_START) |
             (RX_REWRITE_SMAC_FROM_VRMAC << RX_REWRITE_SMAC_START));
        actiondata.action_u.session_session_info.tx_xlate_id =
            vnet_buffer2(b)->pds_nat_data.xlate_idx;
        actiondata.action_u.session_session_info.rx_xlate_id =
            vnet_buffer2(b)->pds_nat_data.xlate_idx_rflow;
    } else if (vnet_buffer2(b)->pds_nat_data.xlate_idx) {
        // static NAT - rewrite only ip
        // TODO: service mapping, flow miss from uplink
        tx_rewrite_flags =
            (TX_REWRITE_SIP_FROM_NAT << TX_REWRITE_SIP_START);
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DIP_FROM_NAT << RX_REWRITE_DIP_START) |
             (RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START));
        actiondata.action_u.session_session_info.tx_xlate_id =
            vnet_buffer2(b)->pds_nat_data.xlate_idx;
        actiondata.action_u.session_session_info.rx_xlate_id =
            vnet_buffer2(b)->pds_nat_data.xlate_idx + 1;
    } else {
        actiondata.action_u.session_session_info.rx_rewrite_flags =
            (RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START);
    }
    actiondata.action_u.session_session_info.tx_rewrite_flags =
        tx_rewrite_flags |
        vec_elt(fm->nh_flags, (vnet_buffer(b)->pds_flow_data.nexthop) >> 16);

    if (PREDICT_FALSE(session_program(session_id, (void *)&actiondata))) {
        next[0] = SESSION_PROG_NEXT_DROP;
    } else {
        next[0] = SESSION_PROG_NEXT_FWD_FLOW;
        vnet_buffer(b)->pds_flow_data.lif = ses_info0->lif;
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

    tx0->pad = 0;
    tx1->pad = 0;
    tx0->nexthop_valid = 0;
    tx1->nexthop_valid = 0;
    tx0->lif_sbit0_ebit7 = vnet_buffer(b0)->pds_flow_data.lif & 0xff;
    tx1->lif_sbit0_ebit7 = vnet_buffer(b1)->pds_flow_data.lif & 0xff;
    tx0->lif_sbit8_ebit10 = vnet_buffer(b0)->pds_flow_data.lif >> 0x8;
    tx1->lif_sbit8_ebit10 = vnet_buffer(b1)->pds_flow_data.lif >> 0x8;

    tx0->lif_flags = clib_host_to_net_u16(tx0->lif_flags);
    tx1->lif_flags = clib_host_to_net_u16(tx1->lif_flags);
    //Dont care about nexthoptype/id as we don't set nexthop_valid.
}

always_inline void
pds_flow_add_tx_hdrs_x1 (vlib_buffer_t *b0)
{
    p4_tx_cpu_hdr_t *tx0;

    tx0 = vlib_buffer_get_current(b0);
    tx0->pad = 0;
    tx0->nexthop_valid = 0;
    tx0->lif_sbit0_ebit7 = vnet_buffer(b0)->pds_flow_data.lif & 0xff;
    tx0->lif_sbit8_ebit10 = vnet_buffer(b0)->pds_flow_data.lif >> 0x8;

    tx0->lif_flags = clib_host_to_net_u16(tx0->lif_flags);
    //Dont care about nexthoptype/id as we don't set nexthop_valid.
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
    sprintf(str, "session_index:%d flow_role:%d",
            d->session_index, d->flow_role);
    return str;
}

always_inline void
pds_flow_extract_nexthop_info(vlib_buffer_t *p0,
                              u8 is_ip4, u8 iflow)
{
    u32 nexthop = 0;
    pds_impl_db_vnic_entry_t *vnic0;

    u8 rx_pak = (vnet_buffer(p0)->pds_flow_data.flags &
                VPP_CPU_FLAGS_RX_PKT_VALID) ? 1 : 0;

    if (iflow) {
        // nexthop is for iflow if tx packet
        if (!rx_pak) {
            nexthop = vnet_buffer(p0)->pds_flow_data.nexthop;
        }
    } else {
        if (rx_pak) {
            // nexthop is for rflow if rx packet
            nexthop = vnet_buffer(p0)->pds_flow_data.nexthop;
        } else {
            // use vnic nexthop for rflow as there may be cases (for example
            // NAT) where mapping is not hit for reverse flow.
            if (vnet_buffer(p0)->pds_flow_data.flags & VPP_CPU_FLAGS_NAPT_VALID) {
                vnic0 = pds_impl_db_vnic_get(vnet_buffer2(p0)->pds_nat_data.vnic_id);
                if (vnic0) {
                    nexthop = vnic0->nh_hw_id | NEXTHOP_TYPE_NEXTHOP << 16;
                }
            }
        }
    }

    if (is_ip4) {
        if (nexthop & 0xffff) {
            ftlv4_cache_set_nexthop(nexthop & 0xffff, ((nexthop >> 16) & 0x3), 1);
        } else {
            ftlv4_cache_set_nexthop(0, 0, 0);
        }
    } else {
        if (nexthop & 0xffff) {
            ftlv6_cache_set_nexthop(nexthop & 0xffff, ((nexthop >> 16) & 0x3), 1);
        } else {
            ftlv6_cache_set_nexthop(0, 0, 0);
        }
    }
}

always_inline void
pds_flow_nh_flags_add (void)
{
    pds_flow_main_t *fm = &pds_flow_main;
    u16 encap, mapping, nexthop, tunnel;

    encap = (TX_REWRITE_ENCAP_VXLAN << TX_REWRITE_ENCAP_START);
    nexthop = 0;
    mapping = (TX_REWRITE_DMAC_FROM_MAPPING << TX_REWRITE_DMAC_START);
    tunnel = (TX_REWRITE_DMAC_FROM_TUNNEL << TX_REWRITE_DMAC_START);
    fm->nh_flags = vec_new(u16, NEXTHOP_TYPE_MAX);
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_VPC) = mapping | encap;
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_ECMP) = nexthop | encap;
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_TUNNEL) = tunnel | encap;
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_NEXTHOP) = nexthop | encap;
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_NAT) = mapping | encap;
}

always_inline int
pds_flow_classify_get_advance_offset (vlib_buffer_t *b)
{
    return (VPP_P4_TO_ARM_HDR_SZ +
            (vnet_buffer(b)->l3_hdr_offset - vnet_buffer (b)->l2_hdr_offset));
}

always_inline void
pds_flow_classify_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u16 *next0, u16 *next1, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr0 = vlib_buffer_get_current(p0);
    p4_rx_cpu_hdr_t *hdr1 = vlib_buffer_get_current(p1);
    u8 flag_orig0, flag_orig1;
    u32 nexthop;
    u16 xlate_id0, xlate_id1;
    u8 next_determined = 0;
    pds_impl_db_vnic_entry_t *vnic0, *vnic1;

    flag_orig0 = hdr0->flags;
    flag_orig1 = hdr1->flags;

    u8 flags0 = flag_orig0 &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);
    u8 flags1 = flag_orig1 &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);

    vnic0 = pds_impl_db_vnic_get(hdr0->vnic_id);
    if (!vnic0) {
        *next0 = FLOW_CLASSIFY_NEXT_DROP;
        counter[FLOW_CLASSIFY_COUNTER_VNIC_NOT_FOUND] += 1;
        next_determined |= 0x1;
    } else {
        vnet_buffer(p0)->pds_flow_data.flow_hash = hdr0->flow_hash;
        vnet_buffer(p0)->pds_flow_data.flags = flag_orig0 |
            (hdr0->rx_packet << VPP_CPU_FLAGS_RX_PKT_POS) |
            (vnic0->flow_log_en << VPP_CPU_FLAGS_FLOW_LOG_POS);

        nexthop = hdr0->nexthop_id;
        if ((!hdr0->mapping_hit || hdr0->rx_packet) && !hdr0->drop) {
            vnet_buffer(p0)->pds_flow_data.nexthop = nexthop |
                                                     (hdr0->nexthop_type << 16);
        } else {
            vnet_buffer(p0)->pds_flow_data.nexthop = hdr0->drop << 18;
        }
        vnet_buffer(p0)->l2_hdr_offset = hdr0->l2_offset;
        vnet_buffer(p0)->l3_hdr_offset =
                hdr0->l3_inner_offset ? hdr0->l3_inner_offset : hdr0->l3_offset;
        vnet_buffer(p0)->l4_hdr_offset =
                hdr0->l4_inner_offset ? hdr0->l4_inner_offset : hdr0->l4_offset;
        vnet_buffer(p0)->sw_if_index[VLIB_TX] = hdr0->ingress_bd_id;
        if (!hdr0->rx_packet) {
            if (hdr0->mapping_hit) {
                vnet_buffer(p0)->pds_flow_data.egress_lkp_id = hdr0->egress_bd_id;
            } else {
                // dest is a route hit but p4 doesn't fill in egress_bd_id
                vnet_buffer(p0)->pds_flow_data.egress_lkp_id = hdr0->vpc_id;
            }
        } else {
            vnet_buffer(p0)->pds_flow_data.egress_lkp_id = hdr0->egress_bd_id;
        }
        vnet_buffer2(p0)->pds_nat_data.vpc_id = hdr0->vpc_id;
        vnet_buffer2(p0)->pds_nat_data.vnic_id = hdr0->vnic_id;
        if (PREDICT_FALSE(vnic0->max_sessions &&
                (vnic0->active_session_count >= vnic0->max_sessions))) {
            *next0 = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_MAX_EXCEEDED] += 1;
            next_determined |= 0x1;
        } else {
            vnic0->active_session_count++;
        }
    }

    vnic1 = pds_impl_db_vnic_get(hdr1->vnic_id);
    if (!vnic1) {
        *next1 = FLOW_CLASSIFY_NEXT_DROP;
        counter[FLOW_CLASSIFY_COUNTER_VNIC_NOT_FOUND] += 1;
        next_determined |= 0x2;
    } else {
        vnet_buffer(p1)->pds_flow_data.flow_hash = hdr1->flow_hash;
        vnet_buffer(p1)->pds_flow_data.flags = flag_orig1 |
            (hdr1->rx_packet << VPP_CPU_FLAGS_RX_PKT_POS) |
            (vnic1->flow_log_en << VPP_CPU_FLAGS_FLOW_LOG_POS);
        nexthop = hdr1->nexthop_id;
        if ((!hdr1->mapping_hit || hdr1->rx_packet) && !hdr1->drop) {
            vnet_buffer(p1)->pds_flow_data.nexthop = nexthop |
                                                (hdr1->nexthop_type << 16);
        } else {
            vnet_buffer(p1)->pds_flow_data.nexthop = hdr1->drop << 18;
        }
        vnet_buffer(p1)->l2_hdr_offset = hdr1->l2_offset;
        vnet_buffer(p1)->l3_hdr_offset =
                hdr1->l3_inner_offset ? hdr1->l3_inner_offset : hdr1->l3_offset;
        vnet_buffer(p1)->l4_hdr_offset =
                hdr1->l4_inner_offset ? hdr1->l4_inner_offset : hdr1->l4_offset;
        vnet_buffer(p1)->sw_if_index[VLIB_TX] = hdr1->ingress_bd_id;
        if (!hdr1->rx_packet) {
            if (hdr1->mapping_hit) {
                vnet_buffer(p1)->pds_flow_data.egress_lkp_id = hdr1->egress_bd_id;
            } else {
                // dest is a route hit but p4 doesn't fill in egress_bd_id
                vnet_buffer(p1)->pds_flow_data.egress_lkp_id = hdr1->vpc_id;
            }
        } else {
            vnet_buffer(p1)->pds_flow_data.egress_lkp_id = hdr1->egress_bd_id;
        }
        vnet_buffer2(p1)->pds_nat_data.vpc_id = hdr1->vpc_id;
        vnet_buffer2(p1)->pds_nat_data.vnic_id = hdr1->vnic_id;
        if (PREDICT_FALSE(vnic1->max_sessions &&
            (vnic1->active_session_count >= vnic1->max_sessions))) {
            *next1 = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_MAX_EXCEEDED] += 1;
            next_determined |= 0x2;
        } else {
            vnic1->active_session_count++;
        }
    }

    vlib_buffer_advance(p0, pds_flow_classify_get_advance_offset(p0));
    vlib_buffer_advance(p1, pds_flow_classify_get_advance_offset(p1));

    if (((next_determined & 0x1) == 0) &&
        (hdr0->snat_type != ROUTE_RESULT_SNAT_TYPE_NONE)) {
        if (!hdr0->rx_packet) {
            /* only from host supported for now */
            /* Try static NAT first */
            if (hdr0->mapping_xlate_id != 0) {
                u32 ip;
                u16 port;
                xlate_id0 = hdr0->mapping_xlate_id;
                pds_snat_tbl_read_ip4(xlate_id0, &ip, &port);
                vnet_buffer2(p0)->pds_nat_data.xlate_idx = xlate_id0;
                vnet_buffer2(p0)->pds_nat_data.xlate_addr = ip;
            } else {
                /* nat44 */
                if (flags0 & VPP_CPU_FLAGS_IPV4_1_VALID) {
                    vnet_buffer(p0)->pds_flow_data.flags |= VPP_CPU_FLAGS_NAPT_VALID;
                    if (hdr0->snat_type == ROUTE_RESULT_SNAT_TYPE_NAPT_SVC) {
                        vnet_buffer(p0)->pds_flow_data.flags |=
                            VPP_CPU_FLAGS_NAPT_SVC_VALID;
                    }
                    *next0 = FLOW_CLASSIFY_NEXT_IP4_NAT;
                    counter[FLOW_CLASSIFY_COUNTER_IP4_NAT] += 1;
                } else {
                    *next0 = FLOW_CLASSIFY_NEXT_DROP;
                    counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
                    vnic0->active_session_count--;
                }
                next_determined |= 0x1;
            }
        } else {
            /* Only static nat should be valid here */
            /* TODO : From network pkt */
            *next0 = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
            next_determined |= 0x1;
            vnic0->active_session_count--;
        }
    }
    if (((next_determined & 0x2) == 0) &&
        (hdr1->snat_type != ROUTE_RESULT_SNAT_TYPE_NONE)) {
        if (!hdr1->rx_packet) {
            /* only from host supported for now */
            /* Try static NAT first */
            if (hdr1->mapping_xlate_id != 0) {
                u32 ip;
                u16 port;
                xlate_id1 = hdr1->mapping_xlate_id;
                pds_snat_tbl_read_ip4(xlate_id1, &ip, &port);
                vnet_buffer2(p1)->pds_nat_data.xlate_idx = xlate_id1;
                vnet_buffer2(p1)->pds_nat_data.xlate_addr = ip;
            } else {
                /* nat44 */
                if (flags1 & VPP_CPU_FLAGS_IPV4_1_VALID) {
                    vnet_buffer(p1)->pds_flow_data.flags |= VPP_CPU_FLAGS_NAPT_VALID;
                    if (hdr1->snat_type == ROUTE_RESULT_SNAT_TYPE_NAPT_SVC) {
                        vnet_buffer(p1)->pds_flow_data.flags |=
                            VPP_CPU_FLAGS_NAPT_SVC_VALID;
                    }
                    *next1 = FLOW_CLASSIFY_NEXT_IP4_NAT;
                    counter[FLOW_CLASSIFY_COUNTER_IP4_NAT] += 1;
                } else {
                    *next1 = FLOW_CLASSIFY_NEXT_DROP;
                    counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
                    vnic1->active_session_count--;
                }
                next_determined |= 0x2;
            }
        } else {
            /* Only static nat should be valid here */
            /* TODO : From network pkt */
            *next1 = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
            next_determined |= 0x2;
            vnic1->active_session_count--;
        }
    }

    if ((flags0 == flags1) && !next_determined) {
        if ((flags0 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_IP4_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP4_FLOW] += 2;
        } else if (flags0 & VPP_CPU_FLAGS_IPV4_2_VALID) {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_IP4_TUN_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP4_TUN_FLOW] += 2;
        } else if (flags0 == VPP_CPU_FLAGS_IPV6_1_VALID) {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_IP6_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP6_FLOW] += 2;
        } else if (flags0 & VPP_CPU_FLAGS_IPV6_2_VALID) {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_IP6_TUN_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP6_TUN_FLOW] += 2;
        } else {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 2;
            vnic0->active_session_count--;
            vnic1->active_session_count--;
        }
        return;
    }

    if ((next_determined & 0x1) == 0) {
        if ((flags0 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
            *next0 = FLOW_CLASSIFY_NEXT_IP4_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP4_FLOW] += 1;
        } else if (flags0 & VPP_CPU_FLAGS_IPV4_2_VALID) {
            *next0 = FLOW_CLASSIFY_NEXT_IP4_TUN_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP4_TUN_FLOW] += 1;
        } else if (flags0 == VPP_CPU_FLAGS_IPV6_1_VALID) {
            *next0 = FLOW_CLASSIFY_NEXT_IP6_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP6_FLOW] += 1;
        } else if (flags0 & VPP_CPU_FLAGS_IPV6_2_VALID) {
            *next0 = FLOW_CLASSIFY_NEXT_IP6_TUN_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP6_TUN_FLOW] += 1;
        } else {
            *next0 = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
            vnic0->active_session_count--;
        }
    }

    if ((next_determined & 0x2) == 0) {
        if ((flags1 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
            *next1 = FLOW_CLASSIFY_NEXT_IP4_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP4_FLOW] += 1;
        } else if (flags1 & VPP_CPU_FLAGS_IPV4_2_VALID) {
            *next1 = FLOW_CLASSIFY_NEXT_IP4_TUN_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP4_TUN_FLOW] += 1;
        } else if (flags1 == VPP_CPU_FLAGS_IPV6_1_VALID) {
            *next1 = FLOW_CLASSIFY_NEXT_IP6_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP6_FLOW] += 1;
        } else if (flags1 & VPP_CPU_FLAGS_IPV6_2_VALID) {
            *next1 = FLOW_CLASSIFY_NEXT_IP6_TUN_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP6_TUN_FLOW] += 1;
        } else {
            *next1 = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
            vnic1->active_session_count--;
        }
    }
}

always_inline void
pds_flow_classify_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr = vlib_buffer_get_current(p);
    u8 flag_orig;
    u32 nexthop;
    u16 xlate_id;
    pds_impl_db_vnic_entry_t *vnic;

    flag_orig = hdr->flags;
    u8 flags = flag_orig &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);

    vnic = pds_impl_db_vnic_get(hdr->vnic_id);
    if (!vnic) {
        *next = FLOW_CLASSIFY_NEXT_DROP;
        counter[FLOW_CLASSIFY_COUNTER_VNIC_NOT_FOUND] += 1;
        return;
    }
    vnet_buffer(p)->pds_flow_data.flow_hash = hdr->flow_hash;
    vnet_buffer(p)->pds_flow_data.flags = flag_orig |
        (hdr->rx_packet << VPP_CPU_FLAGS_RX_PKT_POS) | 
        (vnic->flow_log_en << VPP_CPU_FLAGS_FLOW_LOG_POS);
    nexthop = hdr->nexthop_id;
    if ((!hdr->mapping_hit || hdr->rx_packet) && !hdr->drop) {
        vnet_buffer(p)->pds_flow_data.nexthop = nexthop |
                                                (hdr->nexthop_type << 16);
    } else {
        vnet_buffer(p)->pds_flow_data.nexthop = hdr->drop << 18;
    }
    vnet_buffer(p)->l2_hdr_offset = hdr->l2_offset;
    vnet_buffer(p)->l3_hdr_offset =
            hdr->l3_inner_offset ? hdr->l3_inner_offset : hdr->l3_offset;
    vnet_buffer(p)->l4_hdr_offset =
            hdr->l4_inner_offset ? hdr->l4_inner_offset : hdr->l4_offset;

    vnet_buffer(p)->sw_if_index[VLIB_TX] = hdr->ingress_bd_id;
    if (!hdr->rx_packet) {
        if (hdr->mapping_hit) {
            vnet_buffer(p)->pds_flow_data.egress_lkp_id = hdr->egress_bd_id;
        } else {
            // dest is a route hit but p4 doesn't fill in egress_bd_id
            vnet_buffer(p)->pds_flow_data.egress_lkp_id = hdr->vpc_id;
        }
    } else {
        vnet_buffer(p)->pds_flow_data.egress_lkp_id = hdr->egress_bd_id;
    }
    vnet_buffer2(p)->pds_nat_data.vpc_id = hdr->vpc_id;
    vnet_buffer2(p)->pds_nat_data.vnic_id = hdr->vnic_id;

    vlib_buffer_advance(p, pds_flow_classify_get_advance_offset(p));
    if (PREDICT_FALSE(vnic->max_sessions &&
        (vnic->active_session_count >= vnic->max_sessions))) {
        *next = FLOW_CLASSIFY_NEXT_DROP;
        counter[FLOW_CLASSIFY_COUNTER_MAX_EXCEEDED] += 1;
        return;
    }
    vnic->active_session_count++;

    if (hdr->snat_type != ROUTE_RESULT_SNAT_TYPE_NONE) {
        if (!hdr->rx_packet) {
            /* only from host supported for now */
            /* Try static NAT first */
            if (hdr->mapping_xlate_id != 0) {
                u32 ip;
                u16 port;
                xlate_id = hdr->mapping_xlate_id;
                pds_snat_tbl_read_ip4(xlate_id, &ip, &port);
                vnet_buffer2(p)->pds_nat_data.xlate_idx = xlate_id;
                vnet_buffer2(p)->pds_nat_data.xlate_addr = ip;
            } else {
                /* nat44 */
                if (flags & VPP_CPU_FLAGS_IPV4_1_VALID) {
                    vnet_buffer(p)->pds_flow_data.flags |= VPP_CPU_FLAGS_NAPT_VALID;
                    if (hdr->snat_type == ROUTE_RESULT_SNAT_TYPE_NAPT_SVC) {
                        vnet_buffer(p)->pds_flow_data.flags |=
                            VPP_CPU_FLAGS_NAPT_SVC_VALID;
                    }
                    *next = FLOW_CLASSIFY_NEXT_IP4_NAT;
                    counter[FLOW_CLASSIFY_COUNTER_IP4_NAT] += 1;
                    return;
                } else {
                    *next = FLOW_CLASSIFY_NEXT_DROP;
                    counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
                    goto err;
                }
            }
        } else {
            /* Only static nat should be valid here */
            /* TODO : From network pkt */
            *next = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
            goto err;
        }
    }

    if ((flags == VPP_CPU_FLAGS_IPV4_1_VALID)) {
        *next = FLOW_CLASSIFY_NEXT_IP4_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP4_FLOW] += 1;
    } else if (flags & VPP_CPU_FLAGS_IPV4_2_VALID) {
        *next = FLOW_CLASSIFY_NEXT_IP4_TUN_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP4_TUN_FLOW] += 1;
    } else if (flags == VPP_CPU_FLAGS_IPV6_1_VALID) {
        *next = FLOW_CLASSIFY_NEXT_IP6_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP6_FLOW] += 1;
    } else if (flags & VPP_CPU_FLAGS_IPV6_2_VALID) {
        *next = FLOW_CLASSIFY_NEXT_IP6_TUN_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP6_TUN_FLOW] += 1;
    } else {
        *next = FLOW_CLASSIFY_NEXT_DROP;
        counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
        goto err;
    }

    return;
err:
    vnic->active_session_count--;
    return;
}

always_inline void
pds_flow_pipeline_init (void)
{
    pds_infra_api_reg_t params = {0}; 
    //vlib_node_t *flow;

    params.nacl_id = NACL_DATA_ID_FLOW_MISS_IP4_IP6;
    params.node = format(0, "pds-flow-classify");
    //flow = vlib_get_node_by_name(vlib_get_main(), (u8 *) "pds-flow-classify");
    //params.frame_queue_index = vlib_frame_queue_main_init (flow->index, 0);
    //params.handoff_thread = 0;
    params.frame_queue_index = ~0;
    params.handoff_thread = ~0;
    params.offset = 0;
    params.unreg = 0;
    
    if (0 != pds_register_nacl_id_to_node(&params)) {
        ASSERT(0);
    }
    return;
}

#endif    // __VPP_IMPL_APULU_FLOW_H__
