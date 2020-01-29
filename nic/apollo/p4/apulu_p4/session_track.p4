/*****************************************************************************/
/* Session tracking                                                          */
/*****************************************************************************/
action session_track_info(iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,
                          iflow_tcp_win_sz, iflow_tcp_win_scale,
                          rflow_tcp_state, rflow_tcp_seq_num, rflow_tcp_ack_num,
                          rflow_tcp_win_sz, rflow_tcp_win_scale) {
    if (tcp.valid == TRUE) {
        modify_field(scratch_metadata.tcp_flags, tcp.flags);
        if (p4e_i2e.flow_role == TCP_FLOW_INITIATOR) {
            modify_field(scratch_metadata.tcp_state, iflow_tcp_state);
            modify_field(scratch_metadata.tcp_seq_num, iflow_tcp_seq_num);
            modify_field(scratch_metadata.tcp_ack_num, iflow_tcp_ack_num);
            modify_field(scratch_metadata.tcp_win_sz, iflow_tcp_win_sz);
            modify_field(scratch_metadata.tcp_win_scale, iflow_tcp_win_scale);
        } else {
            modify_field(scratch_metadata.tcp_state, rflow_tcp_state);
            modify_field(scratch_metadata.tcp_seq_num, rflow_tcp_seq_num);
            modify_field(scratch_metadata.tcp_ack_num, rflow_tcp_ack_num);
            modify_field(scratch_metadata.tcp_win_sz, rflow_tcp_win_sz);
            modify_field(scratch_metadata.tcp_win_scale, rflow_tcp_win_scale);
        }
    }
}

@pragma stage 2
@pragma hbm_table
@pragma table_write
@pragma index_table
table session_track {
    reads {
        p4e_i2e.session_id  : exact;
    }
    actions {
        session_track_info;
    }
    size : SESSION_TABLE_SIZE;
}
