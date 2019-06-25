action pkt_dma_setup() {
    modify_field(scratch_qstate_hdr.c_index0, txdma_control.cindex);
    modify_field(scratch_metadata.qid, capri_txdma_intr.qid);
}

@pragma stage 6
table pkt_dma_setup {
    actions {
        pkt_dma_setup;
    }
}

action pkt_dma() {
    if (capri_p4_intr.recirc_count == 0) {
        modify_field(scratch_metadata.lif, capri_intr.lif);
        modify_field(scratch_qstate_hdr.c_index0, txdma_control.cindex);
        modify_field(txdma_predicate.pass_two, TRUE);
        modify_field(capri_p4_intr.recirc, TRUE);
    } else {
        modify_field(capri_p4_intr.recirc, FALSE);

        // Setup Intrinsic fields and DMA commands to create packet and inject to P4IG
        // Touching fields required for DMA commands in ASM code
        modify_field(txdma_control.payload_addr, txdma_control.payload_addr);
        modify_field(rx_to_tx_hdr.payload_len, rx_to_tx_hdr.payload_len);
        modify_field(txdma_control.rxdma_cindex_addr, txdma_control.rxdma_cindex_addr);
    }
}

@pragma stage 7
table pkt_dma {
    actions {
        pkt_dma;
    }
}

control pkt_dma {
    apply(pkt_dma_setup);
    apply(pkt_dma);
}
