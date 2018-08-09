@pragma stage 7
@pragma raw_index_table
@pragma table_write
table txdma_fte_queue_table {
    reads {
        // P4 pipeline must provide (LIF, Qtype, Qid)
        // TxDMA and FTE are two rings of the same Q (if it is allowed), if not
        // slacl action will compute and set qstate_addr for appropriate Q
        capri_rxdma_intrinsic.qstate_addr : exact;
    }
    actions {
        pkt_enqueue;
    }
}

action pkt_enqueue (rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, p_index0, c_index0, 
                    arm_pindex1, arm_cindex1, // invisible to hw
                    sw_pindex0, 
                    ring_base0, ring_base1, ring_sz_mask0, ring_sz_mask1) {
    // d-vector
    modify_field (scratch_qstate_hdr.rsvd, rsvd);
    modify_field (scratch_qstate_hdr.cosA, cosA);
    modify_field (scratch_qstate_hdr.cosB, cosB);
    modify_field (scratch_qstate_hdr.cos_sel, cos_sel);
    modify_field (scratch_qstate_hdr.eval_last, eval_last);
    modify_field (scratch_qstate_hdr.host, host);
    modify_field (scratch_qstate_hdr.total, total);
    modify_field (scratch_qstate_hdr.pid, pid);
    modify_field (scratch_qstate_hdr.p_index0, p_index0);
    modify_field (scratch_qstate_hdr.c_index0, c_index0);

    modify_field (scratch_qstate_txdma_fte_Q.arm_pindex1, arm_pindex1);
    modify_field (scratch_qstate_txdma_fte_Q.arm_cindex1, arm_cindex1);
    modify_field (scratch_qstate_txdma_fte_Q.sw_pindex0, sw_pindex0);
    modify_field (scratch_qstate_txdma_fte_Q.ring_base0, ring_base0);
    modify_field (scratch_qstate_txdma_fte_Q.ring_base1, ring_base1);
    modify_field (scratch_qstate_txdma_fte_Q.ring_sz_mask0, ring_sz_mask0);
    modify_field (scratch_qstate_txdma_fte_Q.ring_sz_mask1, ring_sz_mask1);

    // k-vector
    modify_field (scratch_metadata.sl_result, p4_to_rxdma_header.sl_result);

    // Ring0 goes to TxDMA and Ring1 goes to FTE
    // choose the ring based on slacl_result
    // check if the selected ring is full
    // increment pindex for the ring
    // compute the packet buffer address and dma desc+pkt to the packet buffer
    // ring door bell if it is TxDMA ring
    // for rings to FTE - ARM is polling so we really don't need a hw queue
    // set total rings = 1 to make the FTE ring invisible to hw, but use the pindex1, cindex1
    // to manage the FTE ring
}

control pkt_enqueue {
    if (p4_to_rxdma_header.sl_result == 0x00) {
        // drop
    } else {
        // enqueue to FTE or TxDMA ring based on SF bit (I)
        apply(txdma_fte_queue_table);
    }
}
