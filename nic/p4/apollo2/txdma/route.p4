/*****************************************************************************/
/* Route lookup                                                              */
/*****************************************************************************/
action lpm_done(data) {
    if (data >> 15 == 1) {
        modify_field(txdma_to_p4e_header.nexthop_index, data);
    } else {
        modify_field(p4_to_txdma_header.vcn_id, data);
    }
}

action lpm_s0(done, data) {
    modify_field(scratch_metadata.lpm_data, data);
    modify_field(lpm_metadata.done, done);
    if (done == TRUE) {
        lpm_done(scratch_metadata.lpm_data);
    } else {
        modify_field(p4_to_txdma_header.lpm_addr, (p4_to_txdma_header.lpm_base_addr +
                                         p4_to_txdma_header.lpm_dst +
                                         scratch_metadata.lpm_data));
    }
}

action lpm_s1(done, data) {
    modify_field(scratch_metadata.lpm_data, data);
    modify_field(lpm_metadata.done, done);
    if (done == TRUE) {
        lpm_done(scratch_metadata.lpm_data);
    } else {
        modify_field(p4_to_txdma_header.lpm_addr, (p4_to_txdma_header.lpm_base_addr +
                                         p4_to_txdma_header.lpm_dst +
                                         scratch_metadata.lpm_data));
    }
}

action lpm_s2(data) {
    modify_field(scratch_metadata.lpm_data, data);
    lpm_done(scratch_metadata.lpm_data);
}

@pragma stage 2
@pragma hbm_table
@pragma raw_index_table
table lpm_s0 {
    reads {
        p4_to_txdma_header.lpm_addr : exact;
    }
    actions {
        lpm_s0;
    }
}

@pragma stage 3
@pragma hbm_table
@pragma raw_index_table
table lpm_s1 {
    reads {
        p4_to_txdma_header.lpm_addr : exact;
    }
    actions {
        lpm_s1;
    }
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table lpm_s2 {
    reads {
        p4_to_txdma_header.lpm_addr : exact;
    }
    actions {
        lpm_s2;
    }
}

control route {
    if (predicate_header.lpm_bypass == FALSE) {
        apply(lpm_s0);
        if (lpm_metadata.done == FALSE) {
            apply(lpm_s1);
        }
        if (lpm_metadata.done == FALSE) {
            apply(lpm_s2);
        }
    }
}
