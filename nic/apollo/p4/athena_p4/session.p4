action session_info_common(valid_flag, conntrack_id, timestamp,
                    h2s_throttle_pps_id, h2s_throttle_bw_id,
                    h2s_vnic_statistics_id, h2s_vnic_statistics_mask, h2s_vnic_histogram_id,
                    s2h_throttle_pps_id, s2h_throttle_bw_id,
                    s2h_vnic_statistics_id, s2h_vnic_statistics_mask, s2h_vnic_histogram_id) {
    if (valid_flag == TRUE) {
        if (conntrack_id != 0) {
            modify_field(control_metadata.conntrack_index, conntrack_id);
            modify_field(control_metadata.conntrack_index_valid, TRUE);
        }

        if (control_metadata.direction == RX_FROM_SWITCH) {
            if (h2s_throttle_pps_id != 0) {
                modify_field(control_metadata.throttle_pps, h2s_throttle_pps_id);
                modify_field(control_metadata.throttle_pps_valid, TRUE);
            }
            if (h2s_throttle_bw_id != 0) {
                modify_field(control_metadata.throttle_bw, h2s_throttle_bw_id);
                modify_field(control_metadata.throttle_bw_valid, TRUE);
            }
            if (h2s_vnic_statistics_id != 0) {
                modify_field(control_metadata.statistics_id, h2s_vnic_statistics_id);
                modify_field(control_metadata.statistics_id_valid, TRUE);
                modify_field(control_metadata.statistics_mask, h2s_vnic_statistics_mask);
            }
            if (h2s_vnic_histogram_id != 0) {
                modify_field(control_metadata.histogram_id, h2s_vnic_histogram_id);
                modify_field(control_metadata.histogram_id_valid, TRUE);
            }

        }
        else {
            if (s2h_throttle_pps_id != 0) {
                modify_field(control_metadata.throttle_pps, s2h_throttle_pps_id);
                modify_field(control_metadata.throttle_pps_valid, TRUE);
            }
            if (s2h_throttle_bw_id != 0) {
                modify_field(control_metadata.throttle_bw, s2h_throttle_bw_id);
                modify_field(control_metadata.throttle_bw_valid, TRUE);
            }
            if (s2h_vnic_statistics_id != 0) {
                modify_field(control_metadata.statistics_id, s2h_vnic_statistics_id);
                modify_field(control_metadata.statistics_id_valid, TRUE);
                modify_field(control_metadata.statistics_mask, s2h_vnic_statistics_mask);
            }
            if (s2h_vnic_histogram_id != 0) {
                modify_field(control_metadata.histogram_id, s2h_vnic_histogram_id);
                modify_field(control_metadata.histogram_id_valid, TRUE);
            }
        }

        modify_field(scratch_metadata.timestamp, timestamp);
        modify_field(scratch_metadata.flag, valid_flag);
    }
    else {
        modify_field(control_metadata.flow_miss, TRUE);
    }
}

action session_info_per_direction(valid_flag,
                    epoch1_value, epoch1_id, epoch2_value, epoch2_id, allowed_flow_state_bitmask,
                    strip_outer_encap_flag, strip_l2_header_flag, strip_vlan_tag_flag,
                    nat_type, nat_address,
                    encap_type, add_vlan_tag_flag, dmac, smac, vlan, ipv4_sa, ipv4_da,
                    udp_sport, udp_dport, mpls_label1, mpls_label2, mpls_label3,
                    egress_action) {
    if (valid_flag == TRUE) {
        if (epoch1_id != 0) {
            modify_field(control_metadata.epoch1_id, epoch1_id);
            modify_field(control_metadata.epoch1_value, epoch1_value);
            modify_field(control_metadata.epoch1_id_valid, TRUE);
        }
        if (epoch2_id != 0) {
            modify_field(control_metadata.epoch2_id, epoch2_id);
            modify_field(control_metadata.epoch2_value, epoch2_value);
            modify_field(control_metadata.epoch2_id_valid, TRUE);
        }
        modify_field(control_metadata.allowed_flow_state_bitmask, allowed_flow_state_bitmask);
        modify_field(control_metadata.strip_outer_encap_flag, strip_outer_encap_flag);
        modify_field(control_metadata.strip_l2_header_flag, strip_l2_header_flag);
        modify_field(control_metadata.strip_vlan_tag_flag, strip_vlan_tag_flag);

        modify_field(control_metadata.nat_type, nat_type);
        modify_field(control_metadata.nat_address, nat_address);

        modify_field(control_metadata.encap_type, encap_type);
        modify_field(control_metadata.add_vlan_tag_flag, add_vlan_tag_flag);
        modify_field(control_metadata.dmac, dmac);
        modify_field(control_metadata.smac, smac);
        modify_field(control_metadata.vlan, vlan);
        modify_field(control_metadata.ipv4_sa, ipv4_sa);
        modify_field(control_metadata.ipv4_da, ipv4_da);
        modify_field(control_metadata.udp_sport, udp_sport);
        modify_field(control_metadata.udp_dport, udp_dport);
        modify_field(control_metadata.mpls_label1, mpls_label1);
        modify_field(control_metadata.mpls_label2, mpls_label2);
        modify_field(control_metadata.mpls_label3, mpls_label3);

        modify_field(control_metadata.egress_action, egress_action);
    }
    else {
        modify_field(control_metadata.flow_miss, TRUE);
    }
    modify_field(scratch_metadata.flag, valid_flag);
}


@pragma stage 1
@pragma hbm_table
@pragma table_write
@pragma index_table
table session_info_common {
    reads {
        control_metadata.session_index   : exact;
    }
    actions {
        session_info_common;
    }
    size : SESSION_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
@pragma table_write
@pragma index_table
table session_info_h2s {
    reads {
        control_metadata.session_index   : exact;
    }
    actions {
        session_info_per_direction;
    }
    size : SESSION_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
@pragma table_write
@pragma index_table
table session_info_s2h {
    reads {
        control_metadata.session_index   : exact;
    }
    actions {
        session_info_per_direction;
    }
    size : SESSION_TABLE_SIZE;
}

control session_info_lookup {
    if (control_metadata.flow_miss == FALSE) {
        apply(session_info_common);
        if (control_metadata.direction == RX_FROM_SWITCH) {
            apply(session_info_s2h);
        }
        else {
            apply(session_info_h2s);
        }
    }
}
