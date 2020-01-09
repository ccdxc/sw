/*****************************************************************************/
/* Session                                                                   */
/*****************************************************************************/
action session_info(iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,
                    iflow_tcp_win_sz, iflow_tcp_win_scale, rflow_tcp_state,
                    rflow_tcp_seq_num, rflow_tcp_ack_num, rflow_tcp_win_sz,
                    rflow_tcp_win_scale, tx_policer_id, tx_rewrite_flags,
                    tx_xlate_id, tx_xlate_id2, rx_policer_id, rx_rewrite_flags,
                    rx_xlate_id, rx_xlate_id2, meter_id, timestamp, drop) {
    subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.frame_size,
             offset_metadata.l2_1);
    modify_field(control_metadata.rx_packet, p4e_i2e.rx_packet);
    modify_field(control_metadata.update_checksum, p4e_i2e.update_checksum);
    if (p4e_i2e.copp_policer_id != 0) {
        modify_field(control_metadata.copp_policer_valid, TRUE);
    }

    if (p4e_i2e.session_id == 0) {
        egress_drop(P4E_DROP_SESSION_INVALID);
    }

    modify_field(scratch_metadata.flag, drop);
    if (drop == TRUE) {
        egress_drop(P4E_DROP_SESSION_HIT);
    }

    // update stats and state only on first pass through egress pipeline
    if (egress_recirc.valid == FALSE) {
        modify_field(scratch_metadata.session_stats_addr,
                     scratch_metadata.session_stats_addr +
                     (p4e_i2e.session_id * 8 * 4));
        modify_field(scratch_metadata.in_bytes, capri_p4_intrinsic.packet_len);

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

        if ((meter_id != 0) and (p4e_i2e.meter_enabled == TRUE)) {
            modify_field(meter_metadata.meter_enabled, TRUE);
            modify_field(scratch_metadata.meter_id, meter_id);
            if (p4e_i2e.rx_packet == TRUE) {
                modify_field(meter_metadata.meter_id,
                             scratch_metadata.meter_id + (METER_TABLE_SIZE/2));
            } else {
                modify_field(meter_metadata.meter_id,
                             scratch_metadata.meter_id);
            }
            modify_field(meter_metadata.meter_len,
                         capri_p4_intrinsic.packet_len);
        }

        modify_field(scratch_metadata.timestamp, timestamp);
    }

    if (p4e_i2e.rx_packet == FALSE) {
        modify_field(rewrite_metadata.policer_id, tx_policer_id);
        modify_field(rewrite_metadata.flags, tx_rewrite_flags);
        if (tx_xlate_id != 0) {
            modify_field(rewrite_metadata.xlate_id, tx_xlate_id);
            modify_field(control_metadata.apply_nat, TRUE);
        } else {
            if (p4e_i2e.xlate_id != 0) {
                modify_field(rewrite_metadata.xlate_id, p4e_i2e.xlate_id);
                modify_field(control_metadata.apply_nat, TRUE);
            }
        }
        if (tx_xlate_id2 != 0) {
            modify_field(rewrite_metadata.xlate_id2, tx_xlate_id2);
            modify_field(control_metadata.apply_nat2, TRUE);
        }
    } else {
        modify_field(rewrite_metadata.policer_id, rx_policer_id);
        modify_field(rewrite_metadata.flags, rx_rewrite_flags);
        if (rx_xlate_id != 0) {
            modify_field(rewrite_metadata.xlate_id, rx_xlate_id);
            modify_field(control_metadata.apply_nat, TRUE);
        } else {
            if (p4e_i2e.xlate_id != 0) {
                modify_field(rewrite_metadata.xlate_id, p4e_i2e.xlate_id);
                modify_field(control_metadata.apply_nat, TRUE);
            }
        }
        if (rx_xlate_id2 != 0) {
            modify_field(rewrite_metadata.xlate_id2, rx_xlate_id2);
            modify_field(control_metadata.apply_nat2, TRUE);
        }
    }
}

@pragma stage 0
@pragma hbm_table
@pragma table_write
@pragma index_table
table session {
    reads {
        p4e_i2e.session_id  : exact;
    }
    actions {
        session_info;
    }
    size : SESSION_TABLE_SIZE;
}

control session_lookup {
    apply(session);
}
