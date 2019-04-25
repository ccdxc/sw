
action pkt_dma_setup() {
    modify_field(capri_intr.tm_iport, TM_PORT_DMA);
    modify_field(scratch_qstate_hdr.c_index0, txdma_control.cindex);
    modify_field(scratch_metadata.qid, capri_txdma_intr.qid);
    modify_field(capri_p4_intr.packet_len,
                 p4_to_txdma_header.payload_len - APOLLO_I2E_HDR_SZ);
    if (predicate_header.redirect_to_arm == TRUE) {
        modify_field(capri_intr.tm_oport, TM_PORT_INGRESS);
        modify_field(txdma_control.payload_addr,
                     txdma_control.payload_addr + APOLLO_I2E_HDR_SZ);
        modify_field(p4_to_txdma_header.payload_len,
                     p4_to_txdma_header.payload_len - APOLLO_I2E_HDR_SZ);
    } else {
        modify_field(capri_intr.tm_oport, TM_PORT_EGRESS);
    }
}

@pragma stage 6
table pkt_dma_setup {
    actions {
        pkt_dma_setup;
    }
}

action pkt_dma() {
    /*
     * header order to ingress pipeline
     *  capri_intr
     *  capri_txdma_intr
     *  predicate_header
     *  packet
     * header order to egress pipeline
     *  capri_intr
     *  capri_p4_intr
     *  capri_txdma_intr
     *  predicate_header
     *  txdma_to_p4e_header
     *  i2e-metadata
     *  packet
     */
    modify_field(predicate_header.redirect_to_arm,
                 predicate_header.redirect_to_arm);
    modify_field(scratch_metadata.payload_addr, txdma_control.payload_addr);
    modify_field(scratch_metadata.payload_len, p4_to_txdma_header.payload_len);
    modify_field(scratch_metadata.lif, capri_intr.lif);
    modify_field(scratch_qstate_hdr.c_index0, txdma_control.cindex);

    modify_field(scratch_qstate_info.rxdma_cindex_addr,
                 txdma_control.rxdma_cindex_addr);
}

@pragma stage 7
table pkt_dma {
    actions {
        pkt_dma;
    }
}

control dma {
    apply(pkt_dma_setup);
    apply(pkt_dma);
}
