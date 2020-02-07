/*****************************************************************************/
/* Session tracking                                                          */
/*****************************************************************************/
action session_track_info(iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,
                          iflow_tcp_win_size, iflow_tcp_win_scale,
                          iflow_tcp_mss, iflow_tcp_exceptions,
                          rflow_tcp_state, rflow_tcp_seq_num, rflow_tcp_ack_num,
                          rflow_tcp_win_size, rflow_tcp_win_scale,
                          rflow_tcp_mss, rflow_tcp_exceptions) {
    if (tcp.valid == TRUE) {
        modify_field(scratch_metadata.tcp_flags, tcp.flags);
        modify_field(scratch_metadata.tcp_flags, tcp_option_ws.value);
        modify_field(scratch_metadata.tcp_flags, tcp_option_mss.value);
        if (p4e_i2e.flow_role == TCP_FLOW_INITIATOR) {
            modify_field(scratch_metadata.tcp_state, iflow_tcp_state);
            modify_field(scratch_metadata.tcp_seq_num, iflow_tcp_seq_num);
            modify_field(scratch_metadata.tcp_ack_num, iflow_tcp_ack_num);
            modify_field(scratch_metadata.tcp_win_size, iflow_tcp_win_size);
            modify_field(scratch_metadata.tcp_win_scale, iflow_tcp_win_scale);
            modify_field(scratch_metadata.tcp_mss, iflow_tcp_mss);
            modify_field(scratch_metadata.tcp_exceptions, iflow_tcp_exceptions);
        } else {
            modify_field(scratch_metadata.tcp_state, rflow_tcp_state);
            modify_field(scratch_metadata.tcp_seq_num, rflow_tcp_seq_num);
            modify_field(scratch_metadata.tcp_ack_num, rflow_tcp_ack_num);
            modify_field(scratch_metadata.tcp_win_size, rflow_tcp_win_size);
            modify_field(scratch_metadata.tcp_win_scale, rflow_tcp_win_scale);
            modify_field(scratch_metadata.tcp_mss, rflow_tcp_mss);
            modify_field(scratch_metadata.tcp_exceptions, rflow_tcp_exceptions);
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
