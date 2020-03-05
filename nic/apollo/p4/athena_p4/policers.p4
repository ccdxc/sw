action policer_bw(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                       color_aware, rsvd, axi_wr_pend, burst, rate, tbkt) {

    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
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
@pragma index_table
@pragma policer_table two_color
table policer_bw1 {
    reads {
        control_metadata.throttle_bw1_id : exact;
    }
    actions {
        policer_bw;
    }
    size : POLICER_BW_SIZE;
}

#if 0
action policer_pps(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                       color_aware, rsvd, axi_wr_pend, burst, rate, tbkt) {

    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
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
#endif

@pragma stage 4
@pragma index_table
@pragma policer_table two_color
table policer_bw2 {
    reads {
        control_metadata.throttle_bw2_id : exact;
    }
    actions {
        policer_bw;
    }
    size : POLICER_BW_SIZE;
}


control policers {
    if (control_metadata.throttle_bw1_id_valid == TRUE) {
        apply(policer_bw1);
    }
    if (control_metadata.throttle_bw2_id_valid == TRUE) {
        apply(policer_bw2);
    }
}
