/*****************************************************************************/
/* CoPP processing                                                           */
/*****************************************************************************/
header_type copp_metadata_t {
    fields {
        policer_index : 5;
        policer_color : 2;
    }
}

metadata copp_metadata_t copp_metadata;

meter copp {
    type           : bytes;
    static         : copp;
    result         : copp_metadata.policer_color;
    instance_count : COPP_TABLE_SIZE;
}

action execute_copp() {
    execute_meter(copp, copp_metadata.policer_index,
                  copp_metadata.policer_color);
    if (copp_metadata.policer_color == POLICER_COLOR_RED) {
        drop_packet();
    }
}

@pragma stage 4
@pragma policer_table two_color
table copp {
    reads {
        copp_metadata.policer_index : exact;
    }
    actions {
        execute_copp;
    }
    size : COPP_TABLE_SIZE;
}

action copp_action(permitted_packets, permitted_bytes,
                   denied_packets, denied_bytes) {
    /* update stats */
    if (copp_metadata.policer_color == POLICER_COLOR_RED) {
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
table copp_action {
    reads {
        copp_metadata.policer_index : exact;
    }
    actions {
        copp_action;
    }
    size : COPP_TABLE_SIZE;
}
control process_copp {
    if (control_metadata.cpu_copy == TRUE) {
        apply(copp);
        apply(copp_action);
    }
}
