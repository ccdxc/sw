action resource_pool_classified_info(policer_index) {
    modify_field(policer_metadata.classified_policer_index, policer_index);
}

action execute_policer_common(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                              color_aware, rsvd, axi_wr_pend,
                              burst, rate, tbkt) {
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

action execute_agg_policer(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                           color_aware, rsvd, axi_wr_pend,
                           burst, rate, tbkt) {
    modify_field(policer_metadata.agg_packet_len, capri_p4_intrinsic.packet_len);
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        modify_field(policer_metadata.agg_policer_color, POLICER_COLOR_RED);
        drop_packet();
    }

    execute_policer_common(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                              color_aware, rsvd, axi_wr_pend,
                              burst, rate, tbkt);
}

action execute_classified_policer(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                                  color_aware, rsvd, axi_wr_pend,
                                  burst, rate, tbkt) {
    modify_field(policer_metadata.classified_packet_len, capri_p4_intrinsic.packet_len);
    if ((entry_valid == TRUE) and ((tbkt >> 39) == 1)) {
        modify_field(policer_metadata.classified_policer_color, POLICER_COLOR_RED);
        drop_packet();
    }

    execute_policer_common(entry_valid, pkt_rate, rlimit_en, rlimit_prof,
                              color_aware, rsvd, axi_wr_pend,
                              burst, rate, tbkt);
}

action policer_stats_common(color, packet_len,
                            permitted_packets, permitted_bytes,
                            denied_packets, denied_bytes) {
    if (color == POLICER_COLOR_RED) {
        add(scratch_metadata.policer_packets, denied_packets, 1);
        add(scratch_metadata.policer_bytes, denied_bytes, packet_len);
    } else {
        add(scratch_metadata.policer_packets, permitted_packets, 1);
        add(scratch_metadata.policer_bytes, permitted_bytes, packet_len);
    }
}

action agg_policer_stats(permitted_packets, permitted_bytes,
                         denied_packets, denied_bytes) {
    policer_stats_common(policer_metadata.agg_policer_color,
                         policer_metadata.agg_packet_len,
                         permitted_packets, permitted_bytes,
                         denied_packets, denied_bytes);
}

action classified_policer_stats(permitted_packets, permitted_bytes,
                         denied_packets, denied_bytes) {
    policer_stats_common(policer_metadata.classified_policer_color,
                         policer_metadata.classified_packet_len,
                         permitted_packets, permitted_bytes,
                         denied_packets, denied_bytes);
}

@pragma stage 3
table resource_pool_classified_tx {
    reads {
        p4e_apollo_i2e.resource_group   : exact;
        p4e_apollo_i2e.traffic_class    : exact;
    }
    actions {
        resource_pool_classified_info;
    }
    size : RESOURCE_POOL_CLASSIFIED_TABLE_SIZE;
}

@pragma stage 3
@pragma overflow_table resource_pool_classified_tx
table resource_pool_classified_tx_otcam {
    reads {
        p4e_apollo_i2e.resource_group   : ternary;
        p4e_apollo_i2e.traffic_class    : ternary;
    }
    actions {
        resource_pool_classified_info;
    }
    size : RESOURCE_POOL_CLASSIFIED_OTCAM_TABLE_SIZE;
}

@pragma stage 3
@pragma policer_table two_color
table agg_policer_tx {
    reads {
        policer_metadata.resource_group : exact;
    }
    actions {
        execute_agg_policer;
    }
    size : AGG_POLICER_TABLE_SIZE;
}

@pragma stage 4
@pragma table_write
table agg_policer_tx_stats {
    reads {
        policer_metadata.resource_group : exact;
    }
    actions {
        agg_policer_stats;
    }
    size : AGG_POLICER_TABLE_SIZE;
}

@pragma stage 4
@pragma policer_table two_color
table classified_policer_tx {
    reads {
        p4e_apollo_i2e.classified_policer_index : exact;
    }
    actions {
        execute_classified_policer;
    }
    size : CLASSIFIED_POLICER_TABLE_SIZE;
}

@pragma stage 5
@pragma table_write
table classified_policer_tx_stats {
    reads {
        p4e_apollo_i2e.classified_policer_index : exact;
    }
    actions {
        classified_policer_stats;
    }
    size : CLASSIFIED_POLICER_TABLE_SIZE;
}

@pragma stage 3
table resource_pool_classified_rx {
    reads {
        policer_metadata.resource_group     : exact;
        policer_metadata.traffic_class      : exact;
    }
    actions {
        resource_pool_classified_info;
    }
    size : RESOURCE_POOL_CLASSIFIED_TABLE_SIZE;
}

@pragma stage 3
@pragma overflow_table resource_pool_classified_rx
table resource_pool_classified_rx_otcam {
    reads {
        policer_metadata.resource_group     : ternary;
        policer_metadata.traffic_class      : ternary;
    }
    actions {
        resource_pool_classified_info;
    }
    size : RESOURCE_POOL_CLASSIFIED_OTCAM_TABLE_SIZE;
}

@pragma stage 1
@pragma policer_table two_color
table agg_policer_rx {
    reads {
        policer_metadata.resource_group : exact;
    }
    actions {
        execute_agg_policer;
    }
    size : AGG_POLICER_TABLE_SIZE;
}

@pragma stage 2
@pragma table_write
table agg_policer_rx_stats {
    reads {
        policer_metadata.resource_group : exact;
    }
    actions {
        agg_policer_stats;
    }
    size : AGG_POLICER_TABLE_SIZE;
}

@pragma stage 1
@pragma policer_table two_color
table classified_policer_rx {
    reads {
        p4e_apollo_i2e.classified_policer_index : exact;
    }
    actions {
        execute_classified_policer;
    }
    size : CLASSIFIED_POLICER_TABLE_SIZE;
}

@pragma stage 2
@pragma table_write
table classified_policer_rx_stats {
    reads {
        p4e_apollo_i2e.classified_policer_index : exact;
    }
    actions {
        classified_policer_stats;
    }
    size : CLASSIFIED_POLICER_TABLE_SIZE;
}

control ingress_resource_pool {
    if (control_metadata.direction == TX_FROM_HOST) {
        apply(agg_policer_tx);
        apply(agg_policer_tx_stats);
    } else {
        apply(resource_pool_classified_rx);
        apply(resource_pool_classified_rx_otcam);
    }
}

control egress_resource_pool {
    if (control_metadata.direction == TX_FROM_HOST) {
        apply(resource_pool_classified_tx);
        apply(resource_pool_classified_tx_otcam);
        apply(classified_policer_tx);
        apply(classified_policer_tx_stats);
    } else {
        apply(agg_policer_rx);
        apply(classified_policer_rx);
        apply(agg_policer_rx_stats);
        apply(classified_policer_rx_stats);
    }
}
