
action pkt_dma() {
    // Representation of all the headers added by txdma
    add_header(capri_intrinsic);
    add_header(capri_p4_intrinsic);
    add_header(capri_txdma_intrinsic);
    add_header(predicate_header);
    add_header(txdma_to_p4e_header);
    // i2e-metadata and the pkt payload come next

    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
    modify_field(txdma_to_p4e_header.vcn_id, p4_to_txdma_header.vcn_id);

    modify_field(scratch_metadata.payload_len, p4_to_txdma_header.payload_len);
    modify_field(scratch_metadata.payload_addr, txdma_control.payload_addr);
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
