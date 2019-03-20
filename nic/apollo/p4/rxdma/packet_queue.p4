action pkt_enqueue(PKTQ_QSTATE) {
    // in rxdma
    //          check sw_pindex0, cindex0
    //          tbl-wr sw_pindex0++
    //          doorbell(dma) pindex0
    // in txdma
    //          check sw_cindex0, pindex0
    //          tbl-wr sw_cindex0++
    //          doorbell(dma) cindex0
    // d-vector
    PKTQ_QSTATE_DVEC_SCRATCH(scratch_qstate_hdr, scratch_qstate_txdma_fte_q);

    // k-vector
    modify_field(scratch_metadata.sacl_result, p4_to_rxdma_header.sacl_result);
    modify_field(scratch_metadata.qid, capri_rxdma_intr.qid);
    modify_field(scratch_metadata.dma_size, (capri_p4_intr.packet_len +
                                             APOLLO_PREDICATE_HDR_SZ +
                                             APOLLO_P4_TO_TXDMA_HDR_SZ +
                                             APOLLO_I2E_HDR_SZ));

    // Ring0 goes to TxDMA and Ring1 goes to FTE
    // choose the ring based on sacl_result
    // check if the selected ring is full
    // increment pindex for the ring
    // compute the packet buffer address and dma desc+pkt to the packet buffer
    // ring door bell if it is TxDMA ring
    // for rings to FTE - ARM is polling so we really don't need a hw queue
    // set total rings = 1 to make the FTE rings invisible to hw, but use the
    // pindex1, cindex1... to manage the FTE rings
    // use cpu_qid_hash to select the fte ring (rss)
    modify_field(scratch_hash_results.cpu_qid_hash, hash_results.cpu_qid_hash);
}

@pragma stage 7
@pragma raw_index_table
@pragma table_write
table txdma_fte_queue {
    reads {
        // P4 pipeline must provide (LIF, Qtype, Qid)
        // TxDMA and FTE are two rings of the same Q (if it is allowed), if not
        // sacl action will compute and set qstate_addr for appropriate Q
        capri_rxdma_intr.qstate_addr : exact;
    }
    actions {
        pkt_enqueue;
    }
}

control pkt_enqueue {
    if (capri_intr.drop == 0) {
        // enqueue to FTE or TxDMA ring based on SF bit (I)
        apply(txdma_fte_queue);
    }
}
