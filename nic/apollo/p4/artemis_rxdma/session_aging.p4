action session_aging(iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,
                     iflow_tcp_win_sz, iflow_tcp_win_scale, rflow_tcp_state,
                     rflow_tcp_seq_num, rflow_tcp_ack_num, rflow_tcp_win_sz,
                     rflow_tcp_win_scale, tx_dst_ip, tx_dst_l4port, nexthop_idx,
                     tx_rewrite_flags, rx_rewrite_flags, tx_policer_idx,
                     rx_policer_idx, meter_idx, timestamp, drop, entry_valid) {

    modify_field(p4_to_rxdma.aging_enable, FALSE);

    
    modify_field(scratch_metadata.flag, drop);

    modify_field(scratch_metadata.field4, iflow_tcp_state);
    modify_field(scratch_metadata.field32, iflow_tcp_seq_num);
    modify_field(scratch_metadata.field32, iflow_tcp_ack_num);
    modify_field(scratch_metadata.field16, iflow_tcp_win_sz);
    modify_field(scratch_metadata.field4, iflow_tcp_win_scale);

    modify_field(scratch_metadata.field4, rflow_tcp_state);
    modify_field(scratch_metadata.field32, rflow_tcp_seq_num);
    modify_field(scratch_metadata.field32, rflow_tcp_ack_num);
    modify_field(scratch_metadata.field16, rflow_tcp_win_sz);
    modify_field(scratch_metadata.field4, rflow_tcp_win_scale);

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.timestamp, timestamp);
    modify_field(scratch_metadata.field16, meter_idx);
    modify_field(scratch_metadata.field128, tx_dst_ip);
    modify_field(scratch_metadata.field16, tx_dst_l4port);
    modify_field(scratch_metadata.field20, nexthop_idx);

    modify_field(scratch_metadata.field8, tx_rewrite_flags);
    modify_field(scratch_metadata.field12, tx_policer_idx);

    modify_field(scratch_metadata.field8, rx_rewrite_flags);
    modify_field(scratch_metadata.field12, rx_policer_idx);
}

@pragma stage 0
@pragma hbm_table
@pragma index_table
table session_aging {
    reads {
        capri_rxdma_intr.qid   : exact;
    }
    actions {
        session_aging;
    }
    size : SESSION_TABLE_SIZE;
}

control session_aging {
    apply(session_aging);
}
