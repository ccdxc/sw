/*****************************************************************************/
/* Session                                                                   */
/*****************************************************************************/
action session_info(iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,
                    iflow_tcp_win_sz, iflow_tcp_win_scale, rflow_tcp_state,
                    rflow_tcp_seq_num, rflow_tcp_ack_num, rflow_tcp_win_sz,
                    rflow_tcp_win_scale, tx_dst_ip, tx_dst_l4port, nexthop_idx,
                    tx_rewrite_flags, rx_rewrite_flags, tx_policer_idx,
                    rx_policer_idx, meter_idx, timestamp, drop) {
    if (p4e_i2e.session_index == 0) {
        egress_drop(P4E_DROP_INVALID_SESSION);
    }
    modify_field(scratch_metadata.session_stats_addr,
                 scratch_metadata.session_stats_addr +
                 (p4e_i2e.session_index * 8 * 4));
    modify_field(scratch_metadata.in_bytes, capri_p4_intrinsic.packet_len);

    modify_field(scratch_metadata.flag, drop);
    if (drop == TRUE) {
        egress_drop(P4E_DROP_SESSION_HIT);
    }

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

    modify_field(scratch_metadata.timestamp, timestamp);
    modify_field(rewrite_metadata.meter_idx, meter_idx);
    modify_field(rewrite_metadata.ip, tx_dst_ip);
    modify_field(rewrite_metadata.l4port, tx_dst_l4port);
    modify_field(rewrite_metadata.nexthop_idx, nexthop_idx);

    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(rewrite_metadata.flags, tx_rewrite_flags);
        modify_field(rewrite_metadata.policer_idx, tx_policer_idx);
        if (TX_REWRITE(tx_rewrite_flags, SRC_IP, FROM_SERVICE)) {
            modify_field(nat_metadata.xlate_idx, p4e_i2e.service_xlate_idx);
        } else {
            if (TX_REWRITE(tx_rewrite_flags, SRC_IP, FROM_PUBLIC)) {
                modify_field(nat_metadata.xlate_idx, p4e_i2e.public_xlate_idx);
            } else {
                modify_field(nat_metadata.xlate_idx, p4e_i2e.pa_or_ca_xlate_idx);
            }
        }
    } else {
        modify_field(rewrite_metadata.flags, rx_rewrite_flags);
        modify_field(rewrite_metadata.policer_idx, rx_policer_idx);
        if (RX_REWRITE(rx_rewrite_flags, DST_IP, FROM_SERVICE)) {
            modify_field(nat_metadata.xlate_idx, p4e_i2e.service_xlate_idx);
        } else {
            if (RX_REWRITE(rx_rewrite_flags, DST_IP, FROM_CA)) {
                modify_field(nat_metadata.xlate_idx, p4e_i2e.pa_or_ca_xlate_idx);
            }
        }
    }
}

@pragma stage 0
@pragma hbm_table
@pragma table_write
@pragma index_table
table session {
    reads {
        p4e_i2e.session_index : exact;
    }
    actions {
        session_info;
    }
    size : SESSION_TABLE_SIZE;
}

control session_lookup {
    apply(session);
}
