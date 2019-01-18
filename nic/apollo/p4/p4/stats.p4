/*****************************************************************************/
/* Ingress VNIC stats                                                        */
/*****************************************************************************/
action vnic_tx_stats(out_packets, out_bytes) {
    add(scratch_metadata.in_packets, out_packets, 1);
    add(scratch_metadata.in_bytes, out_bytes, capri_p4_intrinsic.packet_len);
}

@pragma stage 5
@pragma index_table
@pragma table_write
table vnic_tx_stats {
    reads {
        vnic_metadata.local_vnic_tag   : exact;
    }
    actions {
        vnic_tx_stats;
    }
    size : VNIC_STATS_TABLE_SIZE;
}

/*****************************************************************************/
/* Ingress drop stats                                                        */
/*****************************************************************************/
action p4i_drop_stats(drop_stats_pad, drop_stats_pkts) {
    modify_field(scratch_metadata.drop_stats_pad, drop_stats_pad);
    modify_field(scratch_metadata.drop_stats_pkts, drop_stats_pkts);
}

@pragma stage 5
@pragma table_write
table p4i_drop_stats {
    reads {
        control_metadata.p4i_drop_reason    : ternary;
    }
    actions {
        p4i_drop_stats;
    }
    size : DROP_STATS_TABLE_SIZE;
}

control ingress_stats {
    if (control_metadata.direction == TX_FROM_HOST) {
        apply(vnic_tx_stats);
    }
    if (capri_intrinsic.drop == TRUE) {
        apply(p4i_drop_stats);
    }
}

/*****************************************************************************/
/* Egress VNIC stats                                                         */
/*****************************************************************************/
action vnic_rx_stats(in_packets, in_bytes) {
    add(scratch_metadata.in_packets, in_packets, 1);
    add(scratch_metadata.in_bytes, in_bytes, capri_p4_intrinsic.packet_len);
}

@pragma stage 3
@pragma index_table
@pragma table_write
table vnic_rx_stats {
    reads {
        p4e_apollo_i2e.local_vnic_tag   : exact;
    }
    actions {
        vnic_rx_stats;
    }
    size : VNIC_STATS_TABLE_SIZE;
}

action p4e_drop_stats(drop_stats_pad, drop_stats_pkts) {
    modify_field(scratch_metadata.drop_stats_pad, drop_stats_pad);
    modify_field(scratch_metadata.drop_stats_pkts, drop_stats_pkts);
}

@pragma stage 5
@pragma table_write
table p4e_drop_stats {
    reads {
        control_metadata.p4e_drop_reason    : ternary;
    }
    actions {
        p4e_drop_stats;
    }
    size : DROP_STATS_TABLE_SIZE;
}

control egress_stats {
    if (control_metadata.direction == RX_FROM_SWITCH) {
        apply(vnic_rx_stats);
    }
    if (capri_intrinsic.drop == TRUE) {
        apply(p4e_drop_stats);
    }
}
