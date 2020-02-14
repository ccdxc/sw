/*****************************************************************************/
/* Ingress drops per LIF                                                     */
/*****************************************************************************/
action ingress_tx_stats(ucast_bytes, ucast_pkts, mcast_bytes, mcast_pkts,
                        bcast_bytes, bcast_pkts, pad1, pad2) {
    if (recirc_header.valid == TRUE) {
        modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
        // return
    }
    if (capri_intrinsic.drop == TRUE) {
        // drop stats update implemented using memwr in ASM
        modify_field(scratch_metadata.lif, control_metadata.src_lif);
        if (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
            add_to_field(scratch_metadata.flow_packets, 1);
            add_to_field(scratch_metadata.flow_bytes,
                         capri_p4_intrinsic.packet_len);
        }
        if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
            add_to_field(scratch_metadata.flow_packets, 1);
            add_to_field(scratch_metadata.flow_bytes,
                         capri_p4_intrinsic.packet_len);
        }
        if (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST) {
            add_to_field(scratch_metadata.flow_packets, 1);
            add_to_field(scratch_metadata.flow_bytes,
                         capri_p4_intrinsic.packet_len);
        }
    } else {
        // stats update implemented using tbladd in ASM
        if (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
            add(scratch_metadata.flow_bytes, ucast_bytes,
                capri_p4_intrinsic.packet_len);
            add(scratch_metadata.flow_packets, ucast_pkts, 1);
        }
        if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
            add(scratch_metadata.flow_bytes, mcast_bytes,
                capri_p4_intrinsic.packet_len);
            add(scratch_metadata.flow_packets, mcast_pkts, 1);
        }
        if (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST) {
            add(scratch_metadata.flow_bytes, bcast_bytes,
                capri_p4_intrinsic.packet_len);
            add(scratch_metadata.flow_packets, bcast_pkts, 1);
        }
        modify_field(scratch_metadata.flow_packets, pad1);
        modify_field(scratch_metadata.flow_bytes, pad2);
    }

    tcp_options_fixup();
    add_header(capri_p4_intrinsic);

    // Since we couldn't get the clear_promiscuous_repl as I field into
    // registered_mac table, adding this check here.
    // This will make sure that we don't go through replication
    // in PB if all we have to send is one copy for this packet
    // which could be
    // 1. Uplink port for packets coming from Host side OR
    // 2. Host Port (when there is one PF to Uplink mapping) and
    //    the PF is always in promiscuous mode.
    if (control_metadata.clear_promiscuous_repl == TRUE and
        flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
        modify_field(capri_intrinsic.tm_replicate_en, FALSE);
    }

    modify_field(control_metadata.i2e_flags,
		 control_metadata.uplink << P4_I2E_FLAGS_UPLINK,
                 (1 << P4_I2E_FLAGS_UPLINK));
    // modify_field(control_metadata.i2e_flags,
    //              control_metadata.nic_mode << P4_I2E_FLAGS_NIC_MODE,
    //              (1 << P4_I2E_FLAGS_NIC_MODE));
    modify_field(control_metadata.i2e_flags,
        l3_metadata.ip_frag << P4_I2E_FLAGS_IP_FRAGMENT,
                (1 << P4_I2E_FLAGS_IP_FRAGMENT));

    if ((control_metadata.uplink == TRUE) and
        ((flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) or
         (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST))) {
        modify_field(control_metadata.dst_lport, 0);
    }

    // if ((control_metadata.uplink == TRUE) and
    //     (control_metadata.nic_mode == NIC_MODE_CLASSIC) and
    //     ((flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) or
    //      (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST))) {
    //     modify_field(control_metadata.dst_lport, 0);
    // }
}

@pragma stage 5
@pragma hbm_table
@pragma table_write
@pragma index_table
table ingress_tx_stats {
    reads {
        control_metadata.lif_tx_stats_idx   : exact;
    }
    actions {
        ingress_tx_stats;
    }
    size : TX_STATS_TABLE_SIZE;
}

/*****************************************************************************/
/* Drop accounting per reason code                                           */
/*****************************************************************************/
action drop_stats(mirror_en, mirror_session_id, pad, drop_pkts) {
    /* mirror on drop */
    if (control_metadata.mirror_on_drop_en == TRUE) {
            modify_field(capri_intrinsic.tm_span_session,
                         control_metadata.mirror_on_drop_session_id);
    } else {
        if (mirror_en == TRUE) {
            modify_field(capri_intrinsic.tm_span_session, mirror_session_id);
        }
    }

    // force tm_oport to EGRESS when packet is being dropped
    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.drop_stats_packets, drop_pkts);
    modify_field(scratch_metadata.drop_stats_pad, pad);
    modify_field(scratch_metadata.ingress_mirror_en, mirror_en);

    // fields for logging
    modify_field(capri_intrinsic.tm_iport, capri_intrinsic.tm_iport);
    modify_field(capri_intrinsic.tm_oport, capri_intrinsic.tm_oport);
    modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_iq);
    modify_field(capri_intrinsic.lif, capri_intrinsic.lif);
    modify_field(capri_intrinsic.tm_replicate_ptr, capri_intrinsic.tm_replicate_ptr);
    modify_field(capri_intrinsic.tm_replicate_en, capri_intrinsic.tm_replicate_en);
    modify_field(capri_intrinsic.tm_q_depth, capri_intrinsic.tm_q_depth);
    modify_field(capri_intrinsic.bypass, capri_intrinsic.bypass);
    modify_field(capri_intrinsic.hw_error, capri_intrinsic.hw_error);
    modify_field(capri_intrinsic.tm_oq, capri_intrinsic.tm_oq);
    modify_field(capri_intrinsic.csum_err, capri_intrinsic.csum_err);
    modify_field(capri_intrinsic.error_bits, capri_intrinsic.error_bits);
    modify_field(capri_intrinsic.tm_instance_type, capri_intrinsic.tm_instance_type);

    modify_field(capri_p4_intrinsic.crc_err, capri_p4_intrinsic.crc_err);
    modify_field(capri_p4_intrinsic.len_err, capri_p4_intrinsic.len_err);
    modify_field(capri_p4_intrinsic.recirc_count, capri_p4_intrinsic.recirc_count);
    modify_field(capri_p4_intrinsic.parser_err, capri_p4_intrinsic.parser_err);
    modify_field(capri_p4_intrinsic.frame_size, capri_p4_intrinsic.frame_size);
    modify_field(capri_p4_intrinsic.no_data, capri_p4_intrinsic.no_data);
    modify_field(capri_p4_intrinsic.recirc, capri_p4_intrinsic.recirc);
    modify_field(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len);

    modify_field(scratch_metadata.flag, icmp_echo.valid);
    modify_field(scratch_metadata.flag, icmp.valid);
    modify_field(scratch_metadata.flag, tcp_option_eol.valid);
    modify_field(scratch_metadata.flag, tcp_option_nop_1.valid);
    modify_field(scratch_metadata.flag, tcp_option_nop.valid);
    modify_field(scratch_metadata.flag, tcp_option_unknown.valid);
    modify_field(scratch_metadata.flag, tcp_option_timestamp.valid);
    modify_field(scratch_metadata.flag, tcp_option_four_sack.valid);
    modify_field(scratch_metadata.flag, tcp_option_three_sack.valid);
    modify_field(scratch_metadata.flag, tcp_option_two_sack.valid);
    modify_field(scratch_metadata.flag, tcp_option_one_sack.valid);
    modify_field(scratch_metadata.flag, tcp_option_sack_perm.valid);
    modify_field(scratch_metadata.flag, tcp_option_ws.valid);
    modify_field(scratch_metadata.flag, tcp_option_mss.valid);
    modify_field(scratch_metadata.flag, tcp_options_blob.valid);
    modify_field(scratch_metadata.flag, roce_deth.valid);
    modify_field(scratch_metadata.flag, roce_deth_immdt.valid);
    modify_field(scratch_metadata.flag, roce_bth.valid);
    modify_field(scratch_metadata.flag, esp.valid);
    modify_field(scratch_metadata.flag, ah.valid);
    modify_field(scratch_metadata.flag, tcp.valid);
    modify_field(scratch_metadata.flag, inner_udp.valid);
    modify_field(scratch_metadata.flag, inner_ipv4_options_blob.valid);
    modify_field(scratch_metadata.flag, inner_ipv4.valid);
    modify_field(scratch_metadata.flag, inner_v6_generic.valid);
    modify_field(scratch_metadata.flag, inner_ipv6_options_blob.valid);
    modify_field(scratch_metadata.flag, inner_ipv6.valid);
    modify_field(scratch_metadata.flag, inner_ethernet.valid);
    modify_field(scratch_metadata.flag, vxlan.valid);
    modify_field(scratch_metadata.flag, genv.valid);
    modify_field(scratch_metadata.flag, vxlan_gpe.valid);
    modify_field(scratch_metadata.flag, udp.valid);
    modify_field(scratch_metadata.flag, nvgre.valid);
    modify_field(scratch_metadata.flag, erspan_t3.valid);
    modify_field(scratch_metadata.flag, gre.valid);
    modify_field(scratch_metadata.flag, ipv4_options_blob.valid);
    modify_field(scratch_metadata.flag, ipv4.valid);
    modify_field(scratch_metadata.flag, v6_generic.valid);
    modify_field(scratch_metadata.flag, ipv6_options_blob.valid);
    modify_field(scratch_metadata.flag, ipv6.valid);
    modify_field(scratch_metadata.flag, vlan_tag.valid);
    modify_field(scratch_metadata.flag, snap_header.valid);
    modify_field(scratch_metadata.flag, llc_header.valid);
    modify_field(scratch_metadata.flag, ethernet.valid);
    modify_field(scratch_metadata.flag, p4plus_to_p4_vlan.valid);
    modify_field(scratch_metadata.flag, p4plus_to_p4.valid);
    modify_field(scratch_metadata.flag, capri_txdma_intrinsic.valid);
    modify_field(scratch_metadata.flag, recirc_header.valid);
    modify_field(scratch_metadata.flag, capri_i2e_metadata.valid);
    modify_field(scratch_metadata.flag, capri_p4_intrinsic.valid);
    modify_field(scratch_metadata.flag, capri_intrinsic.valid);

    modify_field(control_metadata.i2e_pad0, 0);
    modify_field(control_metadata.i2e_pad1, 0);
}

@pragma stage 5
@pragma table_write
table drop_stats {
    reads {
        entry_inactive.drop_stats    : ternary;
        control_metadata.drop_reason : ternary;
    }
    actions {
        drop_stats;
    }
    default_action : drop_stats;
    size : DROP_STATS_TABLE_SIZE;
}

/*****************************************************************************/
/* Egress Drop accounting per reason code                                    */
/*****************************************************************************/
action egress_drop_stats(mirror_en, mirror_session_id, pad, drop_pkts) {
    /* mirror on drop */
    if (control_metadata.mirror_on_drop_en == TRUE) {
            modify_field(capri_intrinsic.tm_span_session,
                         control_metadata.mirror_on_drop_session_id);
    } else {
        if (mirror_en == TRUE) {
            modify_field(capri_intrinsic.tm_span_session, mirror_session_id);
        }
    }

    // force tm_oport to DMA when packet is being dropped
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);

    // dummy ops to keep compiler happy
    modify_field(capri_intrinsic.debug_trace, capri_intrinsic.debug_trace);
    modify_field(capri_intrinsic.csum_err, capri_intrinsic.csum_err);
    modify_field(capri_intrinsic.error_bits, capri_intrinsic.error_bits);
    modify_field(capri_intrinsic.tm_instance_type, capri_intrinsic.tm_instance_type);

    modify_field(scratch_metadata.drop_stats_packets, drop_pkts);
    modify_field(scratch_metadata.drop_stats_pad, pad);
    modify_field(scratch_metadata.ingress_mirror_en, mirror_en);

    modify_field(control_metadata.i2e_pad0, 0);
    modify_field(control_metadata.i2e_pad1, 0);
}

@pragma stage 5
@pragma table_write
table egress_drop_stats {
    reads {
        entry_inactive.egress_drop_stats    : ternary;
        control_metadata.egress_drop_reason : ternary;
    }
    actions {
        egress_drop_stats;
    }
    default_action : egress_drop_stats;
    size : DROP_STATS_TABLE_SIZE;
}

action flow_stats(last_seen_timestamp, permit_packets, permit_bytes,
                  drop_packets, drop_bytes, drop_reason, drop_count_map,
                  drop_count1, drop_count2, drop_count3, drop_count4,
                  drop_count5, drop_count6, drop_count7, drop_count8,
                  flow_agg_index1, flow_agg_index2) {
    if (control_metadata.skip_flow_update == TRUE) {
        // return;
    }
    if (capri_intrinsic.drop == TRUE) {
        // update timestamp only if drop reason is flow_hit drop
        if (control_metadata.drop_reason == (1 << DROP_FLOW_HIT)) {
            modify_field(scratch_metadata.flow_last_seen_timestamp,
                         last_seen_timestamp);
        }
        bit_or(scratch_metadata.drop_reason, drop_reason,
               control_metadata.drop_reason);
        add(scratch_metadata.flow_packets, drop_packets, 1);
        add(scratch_metadata.flow_bytes, drop_bytes,
            capri_p4_intrinsic.packet_len);
    } else {
        // update timestamp
        modify_field(scratch_metadata.flow_last_seen_timestamp,
                     last_seen_timestamp);
        add(scratch_metadata.flow_packets, permit_packets, 1);
        add(scratch_metadata.flow_bytes, permit_bytes,
            capri_p4_intrinsic.packet_len);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.drop_count, drop_count_map);
    modify_field(scratch_metadata.drop_count, drop_count1);
    modify_field(scratch_metadata.drop_count, drop_count2);
    modify_field(scratch_metadata.drop_count, drop_count3);
    modify_field(scratch_metadata.drop_count, drop_count4);
    modify_field(scratch_metadata.drop_count, drop_count5);
    modify_field(scratch_metadata.drop_count, drop_count6);
    modify_field(scratch_metadata.drop_count, drop_count7);
    modify_field(scratch_metadata.drop_count, drop_count8);
    modify_field(scratch_metadata.flow_agg_index, flow_agg_index1);
    modify_field(scratch_metadata.flow_agg_index, flow_agg_index2);
}

@pragma stage 5
@pragma hbm_table
@pragma table_write
table flow_stats {
    reads {
        flow_info_metadata.flow_index : exact;
    }
    actions {
        flow_stats;
    }
    default_action : flow_stats;
    size : FLOW_TABLE_SIZE;
}

/*****************************************************************************/
/* Stats for NACL                                                            */
/*****************************************************************************/
action nacl_stats(stats_packets) {
    add(scratch_metadata.nacl_stats_packets, stats_packets, 1);
}

@pragma stage 5
@pragma table_write
table nacl_stats {
    reads {
        control_metadata.nacl_stats_idx : exact;
    }
    actions {
        nop;
        nacl_stats;
    }
    default_action : nop;
    size : NACL_TABLE_SIZE;
}

control process_stats {
    if (capri_intrinsic.drop == TRUE) {
        apply(drop_stats);
    }
    if (control_metadata.flow_miss_ingress == FALSE) {
        apply(flow_stats);
    }
    apply(nacl_stats);
}

/*****************************************************************************/
/* Stats per LIF - accounted in the egress pipeline                          */
/*****************************************************************************/
action tx_stats() {
    modify_field(scratch_metadata.lif, capri_intrinsic.lif);
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
        add_to_field(scratch_metadata.stats_packets, 1);
        modify_field(scratch_metadata.stats_bytes,
                     capri_p4_intrinsic.packet_len);
    }
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
        add_to_field(scratch_metadata.stats_packets, 1);
        modify_field(scratch_metadata.stats_bytes,
                     capri_p4_intrinsic.packet_len);
    }
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST) {
        add_to_field(scratch_metadata.stats_packets, 1);
        modify_field(scratch_metadata.stats_bytes,
                     capri_p4_intrinsic.packet_len);
    }
}

@pragma stage 5
table tx_stats {
    actions {
        tx_stats;
    }
    size : TX_STATS_TABLE_SIZE;
}

control process_tx_stats {
    if (capri_intrinsic.drop == TRUE) {
        apply(egress_drop_stats);
        apply(tx_stats);
    }
}
