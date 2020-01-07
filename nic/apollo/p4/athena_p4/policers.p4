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

@pragma stage 5
@pragma index_table
@pragma policer_table two_color
table policer_bw {
    reads {
        control_metadata.throttle_bw : exact;
    }
    actions {
        policer_bw;
    }
    size : POLICER_BW_SIZE;
}


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

@pragma stage 5
@pragma index_table
@pragma policer_table two_color
table policer_pps {
    reads {
        control_metadata.throttle_pps : exact;
    }
    actions {
        policer_pps;
    }
    size : POLICER_PPS_SIZE;
}


control policers {
    if (control_metadata.throttle_pps_valid == TRUE) {
        apply(policer_pps);
    }
    if (control_metadata.throttle_bw_valid == TRUE) {
        apply(policer_bw);
    }
}
