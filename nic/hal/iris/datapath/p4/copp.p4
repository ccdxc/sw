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

action execute_copp(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                    color_aware, rsvd, axi_wr_pend,
                    burst, rate, tbkt) {
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        modify_field(copp_metadata.policer_color, POLICER_COLOR_RED);
        modify_field(control_metadata.egress_drop_reason, EGRESS_DROP_COPP);
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
@pragma token_refresh_rate 512
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
            policer_metadata.packet_len);
    } else {
        add(scratch_metadata.policer_packets, permitted_packets, 1);
        add(scratch_metadata.policer_bytes, permitted_bytes,
            policer_metadata.packet_len);
    }
}

@pragma stage 4
@pragma table_write
table copp_action {
    reads {
        copp_metadata.policer_index : exact;
    }
    actions {
        copp_action;
    }
    size : COPP_TABLE_SIZE;
}
