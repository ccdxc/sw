
action read_qstate_info (PKTQ_QSTATE) {
    // in rxdma 
    //          check sw_pindex0, cindex0
    //          tbl-wr sw_pindex0++
    //          doorbell(dma) pindex0
    // in txdma 
    //          check sw_cindex0, pindex0
    //          tbl-wr sw_cindex0++
    //          doorbell(dma) cindex0
    // d-vector
    PKTQ_QSTATE_DVEC_SCRATCH(scratch_qstate_hdr, scratch_qstate_txdma_fte_Q);

    if (scratch_qstate_txdma_fte_Q.sw_cindex0 == scratch_qstate_hdr.p_index0) {
        modify_field(predicate_header.txdma_drop_event, TRUE);
    } else {
        modify_field(txdma_control.control_addr,
                scratch_qstate_txdma_fte_Q.ring_base0 + 
                (scratch_qstate_txdma_fte_Q.sw_cindex0 * PKTQ_PAGE_SIZE));
        modify_field(txdma_control.payload_addr, txdma_control.control_addr + (1<<6));
        modify_field(scratch_qstate_txdma_fte_Q.sw_cindex0, 
                scratch_qstate_txdma_fte_Q.sw_cindex0 + 1);
        modify_field(txdma_control.cindex, scratch_qstate_txdma_fte_Q.sw_cindex0);
    }

}

@pragma stage 0
@pragma raw_index_table
@pragma table_write
table read_qstate {
    reads {
        capri_txdma_intr.qstate_addr   : exact;
    }
    actions {
        read_qstate_info;
    }
}

action read_control_info (data) {
    modify_field(scratch_metadata.data512, data);
    // Data is the predicate-header and p4_to_txdma_header. Populate fields 
    // Note: Skip copying into the txdma_drop_event predicate bit since
    // it is set by read_qstate table
    modify_field(predicate_header.lpm_bypass, data);
    modify_field(predicate_header.direction, data);
    modify_field(p4_to_txdma_header.lpm_addr, data);
    modify_field(p4_to_txdma_header.lpm_dst, data);
    modify_field(p4_to_txdma_header.lpm_base_addr, data);
    modify_field(p4_to_txdma_header.payload_len, data);
}

@pragma stage 1
@pragma raw_index_table
table read_control {
    reads {
        txdma_control.control_addr      : exact;
    }
    actions {
        read_control_info;
    }
}

control setup {
    apply(read_qstate);
    apply(read_control);
}
