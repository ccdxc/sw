/*****************************************************************************/
/* Ingress policer                                                           */
/*****************************************************************************/
header_type policer_metadata_t {
    fields {
        ingress_policer_index : 11;
        egress_policer_index  : 11;
        ingress_policer_color : 1;
        egress_policer_color  : 1;
    }
}

metadata policer_metadata_t policer_metadata;

meter ingress_policer {
    type           : bytes;
    static         : ingress_policer;
    result         : policer_metadata.ingress_policer_color;
    instance_count : INGRESS_POLICER_TABLE_SIZE;
}

action execute_ingress_policer() {
    execute_meter(ingress_policer, policer_metadata.ingress_policer_index,
                  policer_metadata.ingress_policer_color);
    if (policer_metadata.ingress_policer_color == POLICER_COLOR_RED) {
        modify_field(control_metadata.drop_reason, DROP_INGRESS_POLICER);
        drop_packet();
    }
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
meter egress_policer {
    type           : bytes;
    static         : egress_policer;
    result         : policer_metadata.egress_policer_color;
    instance_count : EGRESS_POLICER_TABLE_SIZE;
}

action execute_egress_policer() {
    execute_meter(egress_policer, policer_metadata.egress_policer_index,
                  policer_metadata.egress_policer_color);
    if (policer_metadata.egress_policer_color == POLICER_COLOR_RED) {
        drop_packet();
    }
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
    apply(egress_policer);
    apply(egress_policer_action);
}
