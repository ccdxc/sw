action pkt_enqueue(PKTQ_QSTATE) {
    // in rxdma
    //          check sw_pindex0, cindex0
    //          tbl-wr sw_pindex0++
    //          doorbell(dma) pindex0
    // in txdma
    //          check sw_cindex0, pindex0
    //          tbl-wr sw_cindex0++
    //          doorbell(dma) cindex0

    // Clear the hw controlled ttl to prevent TTL drop
    modify_field(capri_p4_intr.recirc_count, 0);
    // Increment the local ttl
    modify_field(lpm_metadata.recirc_count, lpm_metadata.recirc_count + 1);

    // Are we done with processing SACLs...?!
    if (lpm_metadata.sacl_base_addr == 0) {
        // If so, Copy the data that needs to go to TXDMA
        modify_field(rx_to_tx_hdr.rx_packet, p4_to_rxdma.rx_packet);
        modify_field(rx_to_tx_hdr.payload_len, capri_p4_intr.packet_len);
        modify_field(rx_to_tx_hdr.vpc_id, p4_to_rxdma.vpc_id);
        modify_field(rx_to_tx_hdr.vnic_id, (p4_to_rxdma.vnic_info_key&0x7FE)>>1);
        modify_field(rx_to_tx_hdr.iptype, p4_to_rxdma.iptype);

        // d-vector
        PKTQ_QSTATE_DVEC_SCRATCH(scratch_qstate_hdr, scratch_qstate_info);

        // k-vector
        modify_field(scratch_metadata.dma_size, capri_p4_intr.packet_len);
    }
}

@pragma stage 7
@pragma raw_index_table
@pragma table_write
table txdma_enqueue {
    reads {
        capri_rxdma_intr.qstate_addr    : exact;
    }
    actions {
        pkt_enqueue;
    }
}

control pkt_enqueue {
    if (capri_intr.drop == 0) {
        apply(txdma_enqueue);
    }
}
