/*****************************************************************************/
/* Ingress VNIC stats                                                        */
/*****************************************************************************/
action vnic_tx_stats(in_packets, in_bytes) {
    modify_field(scratch_metadata.in_packets, in_packets);
    modify_field(scratch_metadata.in_bytes, in_bytes);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
    add_header(capri_p4_intrinsic);

    if ((service_header.local_ip_mapping_done == FALSE) or
        (service_header.flow_done == FALSE)) {
        add_header(service_header);
    }
}

@pragma stage 5
// TODO: table write pragma has to be added - 
// NCC cannot place the table even with 69% sram utilization
//@pragma table_write
table vnic_tx_stats {
    reads {
        vnic_metadata.local_vnic_tag   : exact;
    }
    actions {
        vnic_tx_stats;
    }
    size : VNIC_STATS_TABLE_SIZE;
}

control ingress_stats {
    if (control_metadata.direction == TX_FROM_HOST) {
        apply(vnic_tx_stats);
    }
}

/*****************************************************************************/
/* Egress VNIC stats                                                         */
/*****************************************************************************/
action vnic_rx_stats(out_packets, out_bytes) {
    modify_field(scratch_metadata.in_packets, out_packets);
    modify_field(scratch_metadata.in_bytes, out_bytes);
}

@pragma stage 3
@pragma table_write
table vnic_rx_stats {
    reads {
        apollo_i2e_metadata.local_vnic_tag  : exact;
    }
    actions {
        vnic_rx_stats;
    }
    size : VNIC_STATS_TABLE_SIZE;
}

control egress_stats {
    if (control_metadata.direction == RX_FROM_SWITCH) {
        apply(vnic_rx_stats);
    }
}
