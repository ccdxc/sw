/*****************************************************************************/
/* Ingress policer                                                           */
/*****************************************************************************/
header_type policer_metadata_t {
    fields {
        rx_policer_color  : 1;
    }
}

metadata policer_metadata_t policer_metadata;

/*****************************************************************************/
/* Egress policer                                                            */
/*****************************************************************************/
action execute_rx_policer(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                              color_aware, rsvd, axi_wr_pend,
                              burst, rate, tbkt) {
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        modify_field(policer_metadata.rx_policer_color, POLICER_COLOR_RED);
        drop_packet();
    }

    modify_field(scratch_metadata.policer_valid, entry_valid);
    modify_field(scratch_metadata.policer_pkt_rate, pkt_rate);
    modify_field(scratch_metadata.policer_rlimit_en, rlimit_en);
    modify_field(scratch_metadata.policer_rlimit_prof, rlimit_prof);
    modify_field(scratch_metadata.policer_color_aware, color_aware);
    modify_field(scratch_metadata.policer_rsvd, rsvd);
    modify_field(scratch_metadata.policer_axi_wr_pend, axi_wr_pend);
    modify_field(scratch_metadata.policer_burst, burst);
    modify_field(scratch_metadata.policer_rate, rate);
    modify_field(scratch_metadata.policer_tbkt, tbkt);
}

@pragma stage 4
@pragma policer_table two_color
table rx_policer {
    reads {
        capri_intrinsic.lif : exact;
    }
    actions {
        execute_rx_policer;
    }
    size : RX_POLICER_TABLE_SIZE;
}

action rx_policer_action(permitted_packets, permitted_bytes,
                             denied_packets, denied_bytes) {
    if (policer_metadata.rx_policer_color == POLICER_COLOR_RED) {
        add(scratch_metadata.policer_packets, denied_packets, 1);
        add(scratch_metadata.policer_bytes, denied_bytes,
            capri_p4_intrinsic.packet_len);
    } else {
        add(scratch_metadata.policer_packets, permitted_packets, 1);
        add(scratch_metadata.policer_bytes, permitted_bytes,
            capri_p4_intrinsic.packet_len);
    }
}

@pragma stage 5
@pragma table_write
table rx_policer_action {
    reads {
        capri_intrinsic.lif : exact;
    }
    actions {
        rx_policer_action;
    }
    size : RX_POLICER_TABLE_SIZE;
}

control process_policer {
    if (control_metadata.to_cpu == TRUE) {
        apply(copp);
        apply(copp_action);
    } else {
        apply(rx_policer);
        apply(rx_policer_action);
    }
}
