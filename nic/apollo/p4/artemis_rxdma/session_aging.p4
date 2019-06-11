action session_aging(entry_valid, iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,
                     iflow_tcp_win_sz, iflow_tcp_win_scale, rflow_tcp_state,
                     rflow_tcp_seq_num, rflow_tcp_ack_num, rflow_tcp_win_sz,
                     rflow_tcp_win_scale, tx_dst_ip, tx_dst_l4port, nexthop_idx,
                     tx_rewrite_flags, rx_rewrite_flags, tx_policer_idx,
                     rx_policer_idx, meter_idx, timestamp, drop) {
    modify_field(p4_to_rxdma.aging_enable, FALSE);
    modify_field(scratch_metadata.entry_valid, entry_valid);
    modify_field(scratch_metadata.iflow_tcp_state, iflow_tcp_state);
    modify_field(scratch_metadata.rflow_tcp_state, rflow_tcp_state);
    modify_field(scratch_metadata.timestamp, timestamp);
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
