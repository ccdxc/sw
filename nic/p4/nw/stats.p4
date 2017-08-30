/*****************************************************************************/
/* Ingress drops per LIF                                                     */
/*****************************************************************************/
action ingress_tx_stats(tx_ingress_drops) {
    if (capri_intrinsic.drop == TRUE) {
        modify_field(scratch_metadata.tx_drop_count, tx_ingress_drops);
    }
}

@pragma stage 5
@pragma table_write
table ingress_tx_stats {
    reads {
        control_metadata.src_lif : exact;
    }
    actions {
        ingress_tx_stats;
    }
    size : TX_STATS_TABLE_SIZE;
}

/*****************************************************************************/
/* Drop accounting per reason code                                           */
/*****************************************************************************/
action drop_stats(stats_idx, drop_pkts, mirror_session_id) {
    modify_field(capri_intrinsic.tm_span_session, mirror_session_id);

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.stats_packets, drop_pkts);
    modify_field(scratch_metadata.stats_idx, stats_idx);
}

@pragma stage 5
@pragma table_write
table drop_stats {
    reads {
        entry_status.inactive        : ternary;
        control_metadata.drop_reason : ternary;
    }
    actions {
        nop;
        drop_stats;
    }
    default_action : nop;
    size : DROP_STATS_TABLE_SIZE;
}

action flow_stats(last_seen_timestamp, permit_packets, permit_bytes,
                  drop_packets, drop_bytes, drop_reason, drop_count_map,
                  drop_count1, drop_count2, drop_count3, drop_count4,
                  drop_count5, drop_count6, drop_count7, drop_count8,
                  flow_agg_index1, flow_agg_index2,
                  // micro-burst detection
                  burst_start_timestamp, burst_max_timestamp,
                  micro_burst_cycles, allowed_bytes,
                  max_allowed_bytes, burst_exceed_bytes,
                  burst_exceed_count) {
    modify_field(scratch_metadata.flow_last_seen_timestamp,
                 last_seen_timestamp);
    if (capri_intrinsic.drop == TRUE) {
        bit_or(scratch_metadata.drop_reason, drop_reason,
               control_metadata.drop_reason);
        add(scratch_metadata.flow_packets, drop_packets, 1);
        add(scratch_metadata.flow_bytes, drop_bytes,
            control_metadata.packet_len);
        // 1. on overflow, update to atomic add region indexed by flow_index
        // 2. based on drop_count_map, update drop_count[1-8]
    } else {
        add(scratch_metadata.flow_packets, permit_packets, 1);
        add(scratch_metadata.flow_bytes, permit_bytes,
            control_metadata.packet_len);
        // 1. on overflow, update to atomic add region indexed by flow index
        // 2. if flow_agg_index1/flow_agg_index2 are non-zero, then update
        // to those indicies as well
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
    modify_field(scratch_metadata.burst_start_timestamp, burst_start_timestamp);
    modify_field(scratch_metadata.burst_max_timestamp, burst_max_timestamp);
    modify_field(scratch_metadata.allowed_bytes, allowed_bytes);
    modify_field(scratch_metadata.max_allowed_bytes, max_allowed_bytes);
    modify_field(scratch_metadata.burst_exceed_bytes, burst_exceed_bytes);
    modify_field(scratch_metadata.burst_exceed_count, burst_exceed_count);
    modify_field(scratch_metadata.micro_burst_cycles, micro_burst_cycles);

#if 0
    // take care of wrap around case.
    if ((burst_start_timestamp <= capri_intrinsic.timestamp) &&
        (capri_intrinsic.timestamp <= burst_max_timestamp)) {
        if (allowed_bytes + control_metadata.packet_len <= max_allowed_bytes) {
            allowed_bytes += control_metadata.packet_len;
        } else {
            burst_exceed_bytes += control_metadata.packet_len;
            burst_exceed_count++;
        }
    } else if (capri_intrinsic.timestamp > burst_max_timestamp) {
        allowed_bytes = control_metadata.packet_len;
        burst_max_timestamp = capri_intrinsic.timestamp+micro_burst_cycles;
        burst_start_timestamp = capri_intrinsic.timestamp;
        burst_exceed_count = 0;
        burst_exceed_bytes = 0;
    }
#endif
}

@pragma stage 5
@pragma hbm_table
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

control process_stats {
    if (capri_intrinsic.drop == TRUE) {
        apply(drop_stats);
    }
    apply(flow_stats);
    apply(ingress_tx_stats);
}

/*****************************************************************************/
/* Stats per LIF - accounted in the egress pipeline                          */
/*****************************************************************************/
action tx_stats(tx_ucast_pkts, tx_mcast_pkts, tx_bcast_pkts,
                tx_ucast_bytes, tx_mcast_bytes, tx_bcast_bytes,
                tx_egress_drops) {
    if (capri_intrinsic.drop == TRUE) {
        modify_field(scratch_metadata.tx_drop_count, tx_egress_drops);
    }
    modify_field(scratch_metadata.stats_bytes, control_metadata.packet_len);
    if ((capri_intrinsic.drop == FALSE) and
        (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST)) {
        modify_field(scratch_metadata.stats_packets, tx_ucast_pkts);
        modify_field(scratch_metadata.stats_bytes, tx_ucast_bytes);
    }
    if ((capri_intrinsic.drop == FALSE) and
        (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST)) {
        modify_field(scratch_metadata.stats_packets, tx_mcast_pkts);
        modify_field(scratch_metadata.stats_bytes, tx_mcast_bytes);
    }
    if ((capri_intrinsic.drop == FALSE) and
        (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST)) {
        modify_field(scratch_metadata.stats_packets, tx_bcast_pkts);
        modify_field(scratch_metadata.stats_bytes, tx_bcast_bytes);
    }
}

@pragma stage 5
@pragma table_write
table tx_stats {
    reads {
        control_metadata.src_lif : exact;
    }
    actions {
        tx_stats;
    }
    size : TX_STATS_TABLE_SIZE;
}

control process_tx_stats {
    apply(tx_stats);
}
