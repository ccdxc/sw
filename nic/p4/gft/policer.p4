action rx_policer(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
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
@pragma policer_table two_color
table rx_ipolicer {
    reads {
        flow_action_metadata.policer_index : exact;
    }
    actions {
        rx_policer;
    }
    size : POLICER_TABLE_SIZE;
}

control rx_policer {
    apply(rx_ipolicer);
}
