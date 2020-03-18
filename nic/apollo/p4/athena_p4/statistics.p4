#if 0
action histogram_update(lt_64, eq_64, lt_128, lt_256, lt_512, lt_1024,
                        lt_1518, ge_1518) {
    add_to_field(scratch_metadata.counter_rx, lt_64);
    add_to_field(scratch_metadata.counter_rx, eq_64);
    add_to_field(scratch_metadata.counter_rx, lt_128);
    add_to_field(scratch_metadata.counter_rx, lt_256);
    add_to_field(scratch_metadata.counter_rx, lt_512);
    add_to_field(scratch_metadata.counter_rx, lt_1024);
    add_to_field(scratch_metadata.counter_rx, lt_1518);
    add_to_field(scratch_metadata.counter_rx, ge_1518);
}

@pragma stage 2
@pragma hbm_table
@pragma table_write
@pragma index_table
table histogram {
    reads {
        p4i_to_p4e_header.histogram  : exact;
    }
    actions {
        histogram_update;
    }
    size : VNIC_TABLE_SIZE;
}

control statistics {
    if (control_metadata.flow_miss == FALSE) {
        if (control_metadata.histogram_valid == TRUE) {
            apply(histogram);
        }
    }
}
#endif


action p4i_stats(rx_from_host, rx_from_switch, rx_from_arm) {
    if (control_metadata.from_arm == TRUE) {
        modify_field(scratch_metadata.counter_rx, rx_from_arm);
    }
    else {
        if (control_metadata.direction == TX_FROM_HOST) {
            modify_field(scratch_metadata.counter_rx, rx_from_host);
        }
        else {
            modify_field(scratch_metadata.counter_rx, rx_from_switch);
        }
    }
}

@pragma stage 5
@pragma hbm_table
@pragma index_table
table p4i_stats {
    reads {
        control_metadata.stats_id : exact;
    }
    actions {
        p4i_stats;
    }
    size : 1;
}

action p4e_stats(tx_to_host, tx_to_switch, tx_to_arm) {
    if ((control_metadata.p4e_stats_flag & P4E_STATS_FLAG_TX_TO_HOST)  == P4E_STATS_FLAG_TX_TO_HOST) {
        modify_field(scratch_metadata.counter_tx, tx_to_host);
    }
    if ((control_metadata.p4e_stats_flag & P4E_STATS_FLAG_TX_TO_SWITCH) == P4E_STATS_FLAG_TX_TO_SWITCH) {
        modify_field(scratch_metadata.counter_tx, tx_to_switch);
    }
    if ((control_metadata.p4e_stats_flag & P4E_STATS_FLAG_TX_TO_ARM) == P4E_STATS_FLAG_TX_TO_ARM) {
        modify_field(scratch_metadata.counter_tx, tx_to_arm);
    }
}

@pragma stage 5
@pragma hbm_table
@pragma index_table
table p4e_stats {
    reads {
        control_metadata.stats_id : exact;
    }
    actions {
        p4e_stats;
    }
    size : 1;
}

control p4i_statistics {
    apply(p4i_stats);
}

control p4e_statistics {
    apply(p4e_stats);
}
