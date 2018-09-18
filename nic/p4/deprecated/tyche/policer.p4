/*****************************************************************************/
/* Common actions                                                            */
/*****************************************************************************/
action execute_policer(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
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

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
@pragma stage 5
@pragma policer_table two_color
table rx_parent_policer {
    reads {
        flow_action_metadata.parent_policer_index : exact;
    }
    actions {
        execute_policer;
    }
    size : PARENT_POLICER_TABLE_SIZE;
}

@pragma stage 5
@pragma policer_table two_color
table rx_child_policer {
    reads {
        flow_action_metadata.child_policer_index : exact;
    }
    actions {
        execute_policer;
    }
    size : PARENT_POLICER_TABLE_SIZE;
}

control rx_policers {
    apply(rx_parent_policer);
    apply(rx_child_policer);
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
@pragma stage 5
@pragma policer_table two_color
table tx_parent_policer {
    reads {
        flow_action_metadata.parent_policer_index : exact;
    }
    actions {
        execute_policer;
    }
    size : PARENT_POLICER_TABLE_SIZE;
}

@pragma stage 5
@pragma policer_table two_color
table tx_child_policer {
    reads {
        flow_action_metadata.child_policer_index : exact;
    }
    actions {
        execute_policer;
    }
    size : PARENT_POLICER_TABLE_SIZE;
}

control tx_policers {
    apply(tx_parent_policer);
    apply(tx_child_policer);
}
