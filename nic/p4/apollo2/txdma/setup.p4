
#define TXDMA_QSTATE \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host_rings, total_rings, pid, \
    p_index0, c_index0, sw_cindex0, ring_size, ring_base, rxdma_cindex_addr

#define TXDMA_QSTATE_DVEC_SCRATCH(_scratch_qstate_hdr, _scratch_txdma_qstate) \
    modify_field(_scratch_qstate_hdr.pc, pc);                                 \
    modify_field(_scratch_qstate_hdr.rsvd, rsvd);                             \
    modify_field(_scratch_qstate_hdr.cosA, cosA);                             \
    modify_field(_scratch_qstate_hdr.cosB, cosB);                             \
    modify_field(_scratch_qstate_hdr.cos_sel, cos_sel);                       \
    modify_field(_scratch_qstate_hdr.eval_last, eval_last);                   \
    modify_field(_scratch_qstate_hdr.host_rings, host_rings);                 \
    modify_field(_scratch_qstate_hdr.total_rings, total_rings);               \
    modify_field(_scratch_qstate_hdr.pid, pid);                               \
    modify_field(_scratch_qstate_hdr.p_index0, p_index0);                     \
    modify_field(_scratch_qstate_hdr.c_index0, c_index0);                     \
                                                                              \
    modify_field(_scratch_txdma_qstate.sw_cindex0, sw_cindex0);               \
    modify_field(_scratch_txdma_qstate.ring_base, ring_base);                 \
    modify_field(_scratch_txdma_qstate.ring_size, ring_size);                 \
    modify_field(_scratch_txdma_qstate.rxdma_cindex_addr, rxdma_cindex_addr)


action read_qstate_info (TXDMA_QSTATE) {
    // in txdma 
    //          check sw_cindex0, pindex0
    //          tbl-wr sw_cindex0++
    //          doorbell(dma) cindex0
    //          dma to rxdma_qstate cindex
    // d-vector
    TXDMA_QSTATE_DVEC_SCRATCH(scratch_qstate_hdr, scratch_txdma_qstate);

    if (scratch_txdma_qstate.sw_cindex0 == scratch_qstate_hdr.p_index0) {
        modify_field(predicate_header.txdma_drop_event, TRUE);
    } else {
        modify_field(txdma_control.control_addr,
                scratch_txdma_qstate.ring_base + 
                (scratch_txdma_qstate.sw_cindex0 * PKTQ_PAGE_SIZE));
        modify_field(txdma_control.payload_addr, txdma_control.control_addr + (1<<6));
        modify_field(scratch_txdma_qstate.sw_cindex0, 
                scratch_txdma_qstate.sw_cindex0 + 1);
        modify_field(txdma_control.cindex, scratch_txdma_qstate.sw_cindex0);
        modify_field(txdma_control.rxdma_cindex_addr, 
                        scratch_txdma_qstate.rxdma_cindex_addr);
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
