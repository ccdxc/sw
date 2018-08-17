action pkt_dma() {
    /* Representation of all the headers added by txdma
     * capri_intr
     * capri_p4_intr
     * capri_txdma_intr
     * predicate_header
     * txdma_to_p4e_header
     * i2e-metadata and the pkt payload come next
     */

    modify_field(capri_intr.tm_iport, TM_PORT_DMA);
    modify_field(capri_intr.tm_oport, TM_PORT_EGRESS);
    modify_field(txdma_to_p4e_header.vcn_id, p4_to_txdma_header.vcn_id);

    modify_field(scratch_metadata.payload_len, p4_to_txdma_header.payload_len);
    modify_field(scratch_metadata.payload_addr, txdma_control.payload_addr);

    modify_field(scratch_qstate_hdr.c_index0, txdma_control.cindex);
    modify_field(scratch_metadata.qid, capri_txdma_intr.qid);
    modify_field(scratch_metadata.lif, capri_intr.lif);
}

@pragma stage 7
table pkt_dma {
    actions {
        pkt_dma;
    }
}

control dma {
    apply(pkt_dma);
}
