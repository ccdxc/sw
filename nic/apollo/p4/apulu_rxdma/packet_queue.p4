action pkt_enqueue(PKTQ_QSTATE) {
    // in rxdma
    //          check sw_pindex0, cindex0
    //          tbl-wr sw_pindex0++
    //          doorbell(dma) pindex0
    // in txdma
    //          check sw_cindex0, pindex0
    //          tbl-wr sw_cindex0++
    //          doorbell(dma) cindex0
    if (lpm_metadata.sacl_base_addr == 0) {
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
