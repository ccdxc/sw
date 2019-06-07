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
    }
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
