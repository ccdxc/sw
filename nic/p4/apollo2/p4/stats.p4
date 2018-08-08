/*****************************************************************************/
/* Ingress VNIC stats                                                        */
/*****************************************************************************/
action vnic_tx_stats(in_packets, in_bytes) {
    add(scratch_metadata.in_packets, in_packets, 1);
    add(scratch_metadata.in_bytes, in_bytes, capri_p4_intrinsic.packet_len);

    if ((service_header.local_ip_mapping_done == FALSE) or
        (service_header.flow_done == FALSE)) {
        add_header(service_header);
        modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
    } else {
        // TODO: Does this functionality need to go to some other place ?
        modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
        add_header(capri_p4_intrinsic);
        add_header(capri_rxdma_intrinsic);
        add_header(p4_to_rxdma_header);
        // Splitter offset should point to here
        modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                     (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                      APOLLO_P4_TO_RXDMA_HDR_SZ));
        add_header(p4_to_txdma_header);
        add_header(apollo_i2e_metadata);
    }
}

@pragma stage 5
@pragma index_table
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
    add(scratch_metadata.in_bytes, out_bytes, capri_p4_intrinsic.packet_len);
}

@pragma stage 3
@pragma index_table
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
