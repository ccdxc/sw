/*****************************************************************************/
/* Session                                                                   */
/*****************************************************************************/
action session_info(tx_rewrite_flags, tx_xlate_id, tx_xlate_id2,
                    rx_rewrite_flags, rx_xlate_id, rx_xlate_id2,
                    meter_id, timestamp, session_tracking_en, drop) {
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
        modify_field(control_metadata.session_tracking_en, session_tracking_en);
        modify_field(scratch_metadata.timestamp, timestamp);
        if (p4e_i2e.skip_stats_update == FALSE) {
            modify_field(scratch_metadata.session_stats_addr,
                         scratch_metadata.session_stats_addr +
                         (p4e_i2e.session_id * 8 * 4));
            modify_field(scratch_metadata.in_bytes,
                         capri_p4_intrinsic.packet_len);

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
        }
    }

    if (p4e_i2e.rx_packet == FALSE) {
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
    if ((control_metadata.session_tracking_en == 1) and
        (tcp.valid == TRUE)) {
        apply(session_track);
    }
}
