/*****************************************************************************/
/* Ingress policer                                                           */
/*****************************************************************************/
header_type policer_metadata_t {
    fields {
        ingress_policer_index : 11;
        egress_policer_index  : 11;
        egress_policer_color  : 1;
    }
}

metadata policer_metadata_t policer_metadata;

action execute_ingress_policer(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                               color_aware, rsvd, axi_wr_pend,
                               burst, rate, tbkt) {
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        modify_field(control_metadata.drop_reason, DROP_INGRESS_POLICER);
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

@pragma stage 3
@pragma policer_table two_color
table ingress_policer {
    reads {
        policer_metadata.ingress_policer_index : exact;
    }
    actions {
        execute_ingress_policer;
    }
    size : INGRESS_POLICER_TABLE_SIZE;
}

action ingress_policer_action(cos_en, cos, dscp_en, dscp,
                              permitted_packets, permitted_bytes,
                              denied_packets, denied_bytes) {
    if (control_metadata.drop_reason == DROP_INGRESS_POLICER) {
        add(scratch_metadata.policer_packets, denied_packets, 1);
        add(scratch_metadata.policer_bytes, denied_bytes,
            control_metadata.packet_len);
    } else {
        add(scratch_metadata.policer_packets, permitted_packets, 1);
        add(scratch_metadata.policer_bytes, permitted_bytes,
            control_metadata.packet_len);
        modify_field(qos_metadata.cos_en, cos_en);
        modify_field(qos_metadata.dscp_en, dscp_en);
        modify_field(qos_metadata.cos, cos);
        modify_field(qos_metadata.dscp, dscp);
    }
}

@pragma stage 4
@pragma table_write
table ingress_policer_action {
    reads {
        policer_metadata.ingress_policer_index : exact;
    }
    actions {
        ingress_policer_action;
    }
    default_action : ingress_policer_action;
    size : INGRESS_POLICER_TABLE_SIZE;
}

control process_ingress_policer {
    apply(ingress_policer);
    apply(ingress_policer_action);
}

/*****************************************************************************/
/* Egress policer                                                            */
/*****************************************************************************/
action execute_egress_policer(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                              color_aware, rsvd, axi_wr_pend,
                              burst, rate, tbkt) {
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        modify_field(policer_metadata.egress_policer_color, POLICER_COLOR_RED);
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
table egress_policer {
    reads {
        policer_metadata.egress_policer_index : exact;

    }
    actions {
        execute_egress_policer;
    }
    size : EGRESS_POLICER_TABLE_SIZE;
}

action egress_policer_action(permitted_packets, permitted_bytes,
                             denied_packets, denied_bytes) {
    if (policer_metadata.egress_policer_color == POLICER_COLOR_RED) {
        add(scratch_metadata.policer_packets, denied_packets, 1);
        add(scratch_metadata.policer_bytes, denied_bytes,
            control_metadata.packet_len);
    } else {
        add(scratch_metadata.policer_packets, permitted_packets, 1);
        add(scratch_metadata.policer_bytes, permitted_bytes,
            control_metadata.packet_len);
    }
}

@pragma stage 5
@pragma table_write
table egress_policer_action {
    reads {
        policer_metadata.egress_policer_index : exact;
    }
    actions {
        egress_policer_action;
    }
    size : EGRESS_POLICER_TABLE_SIZE;
}

control process_egress_policer {
    if (control_metadata.cpu_copy == TRUE) {
        apply(copp);
        apply(copp_action);
    } else {
        apply(egress_policer);
        apply(egress_policer_action);
    }
}
