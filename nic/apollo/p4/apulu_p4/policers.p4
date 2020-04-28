/******************************************************************************/
/* Tx policer                                                                 */
/******************************************************************************/
action vnic_policer_tx(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                       color_aware, rsvd, axi_wr_pend, burst, rate, tbkt) {
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        egress_drop(P4E_DROP_VNIC_POLICER_TX);
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
@pragma index_table
@pragma policer_table two_color
table vnic_policer_tx {
    reads {
        p4e_i2e.tx_policer_id   : exact;
    }
    actions {
        vnic_policer_tx;
    }
    size : VNIC_TABLE_SIZE;
}

/******************************************************************************/
/* Rx policer                                                                 */
/******************************************************************************/
action vnic_policer_rx(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                       color_aware, rsvd, axi_wr_pend, burst, rate, tbkt) {
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        egress_drop(P4E_DROP_VNIC_POLICER_RX);
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
@pragma index_table
@pragma policer_table two_color
table vnic_policer_rx {
    reads {
        vnic_metadata.rx_policer_id : exact;
    }
    actions {
        vnic_policer_rx;
    }
    size : VNIC_TABLE_SIZE;
}

/******************************************************************************/
/* Contol plane policing                                                      */
/******************************************************************************/
action copp(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
            color_aware, rsvd, axi_wr_pend, burst, rate, tbkt) {
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        egress_drop(P4E_DROP_COPP);
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
@pragma index_table
@pragma policer_table two_color
@pragma token_refresh_rate 4000
table copp {
    reads {
        p4e_i2e.copp_policer_id : exact;
    }
    actions {
        copp;
    }
    size : COPP_TABLE_SIZE;
}

control policers {
    apply(vnic_policer_rx);
    apply(vnic_policer_tx);
    if (control_metadata.copp_policer_valid == TRUE) {
        apply(copp);
    }
}
