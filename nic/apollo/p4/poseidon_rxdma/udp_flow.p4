action udp_flow_info() {
    // invalid entry
    // if q_delete : NOP : may be in the next state, set continue = True
    // if enqueue : NOP : continue = True
    // else: flow miss packet (first or later packets until flow is installed)
    //  if allocated (in prev stage) this is a pull-up case : NOP, continue = True
    //  else:
    //      - allocate this slot, flow_allocated = True,
    //      - continue = True : check following slots for pull-up
    //modify_field(udp_scratch.entry_valid, entry_valid);
    modify_field(udp_scratch.udp_flow_lkp_result,
                 udp_flow_metadata.udp_flow_lkp_result);
    modify_field(udp_scratch.udp_flow_hit, p4_to_rxdma_header.udp_flow_hit);
    modify_field(p4_to_rxdma_header.udp_flow_lkp_continue, 1);

    // valid entry - occupied by another flow
    // if q_delete : NOP : may be in the next state, set continue = True
    // if enqueue : NOP : continue = True
    // else: flow miss packet (first or later packets until flow is installed)
    //  NOP, continue = True
    //modify_field(udp_scratch.entry_valid, entry_valid);
    modify_field(udp_scratch.udp_flow_lkp_result,
                 udp_flow_metadata.udp_flow_lkp_result);
    modify_field(p4_to_rxdma_header.udp_flow_lkp_continue, 1);

    // valid entry - occupied by this flow
    //  - if flow_q_found (earlier stage)
    //      - pullup: record q_tbl_idx, free this slot
    //      - continue = 0
    //  - flow_q_found = True
    // if q_delete :
    //  - free the slot (valid = 0, clear the key), set continue = False,
    //  - record q_tbl_idx
    //  - continue = 0
    // if enqueue:
    //  - record q_tbl_idx
    //  - continue = 0
    // if flow miss pkt: BUG ??
    //modify_field(udp_scratch.entry_valid, entry_valid);
    modify_field(udp_scratch.udp_queue_delete,
                 p4_to_rxdma_header.udp_queue_delete);
    modify_field(udp_scratch.udp_q_counter, p4_to_rxdma_header.udp_q_counter);
    modify_field(p4_to_rxdma_header.udp_flow_lkp_continue, 0);
    modify_field(udp_scratch.udp_flow_lkp_result,
                 udp_flow_metadata.udp_flow_lkp_result);
    modify_field(udp_flow_metadata.udp_qid_tbl_idx, 1);
}

@pragma stage 0
@pragma hbm_table
table udp_flow_hash0 {
    reads {
        p4_to_rxdma_header.flow_ktype   : exact;
        p4_to_rxdma_header.flow_src     : exact;
        p4_to_rxdma_header.flow_dst     : exact;
        p4_to_rxdma_header.flow_proto   : exact;
        p4_to_rxdma_header.flow_dport   : exact;
        p4_to_rxdma_header.flow_sport   : exact;
    }
    actions {
        udp_flow_info;
    }
    size : 1024;
}

@pragma stage 0
@pragma hbm_table
//@pragma table_write
@pragma overflow_table udp_flow_hash0
table udp_flow_overflow0 {
    reads {
        p4_to_rxdma_header.udp_oflow_index : exact;
    }
    actions {
        udp_flow_info;
    }
    size : 1024;
}

@pragma stage 1
@pragma hbm_table
table udp_flow_hash1 {
    reads {
        p4_to_rxdma_header.flow_ktype   : exact;
        p4_to_rxdma_header.flow_src     : exact;
        p4_to_rxdma_header.flow_dst     : exact;
        p4_to_rxdma_header.flow_proto   : exact;
        p4_to_rxdma_header.flow_dport   : exact;
        p4_to_rxdma_header.flow_sport   : exact;
    }
    actions {
        udp_flow_info;
    }
    size : 1024;
}

@pragma stage 1
@pragma hbm_table
//@pragma table_write
@pragma overflow_table udp_flow_hash1
table udp_flow_overflow1 {
    reads {
        p4_to_rxdma_header.udp_oflow_index : exact;
    }
    actions {
        udp_flow_info;
    }
    size : 1024;
}

@pragma stage 2
@pragma hbm_table
table udp_flow_hash2 {
    reads {
        p4_to_rxdma_header.flow_ktype   : exact;
        p4_to_rxdma_header.flow_src     : exact;
        p4_to_rxdma_header.flow_dst     : exact;
        p4_to_rxdma_header.flow_proto   : exact;
        p4_to_rxdma_header.flow_dport   : exact;
        p4_to_rxdma_header.flow_sport   : exact;
    }
    actions {
        udp_flow_info;
    }
    size : 1024;
}

@pragma stage 2
@pragma hbm_table
//@pragma table_write
@pragma overflow_table udp_flow_hash2
table udp_flow_overflow2 {
    reads {
        p4_to_rxdma_header.udp_oflow_index : exact;
    }
    actions {
        udp_flow_info;
    }
    size : 1024;
}

@pragma stage 3
@pragma hbm_table
table udp_flow_hash3 {
    reads {
        p4_to_rxdma_header.flow_ktype   : exact;
        p4_to_rxdma_header.flow_src     : exact;
        p4_to_rxdma_header.flow_dst     : exact;
        p4_to_rxdma_header.flow_proto   : exact;
        p4_to_rxdma_header.flow_dport   : exact;
        p4_to_rxdma_header.flow_sport   : exact;
    }
    actions {
        udp_flow_info;
    }
    size : 1024;
}

@pragma stage 3
@pragma hbm_table
//@pragma table_write
@pragma overflow_table udp_flow_hash3
table udp_flow_overflow3 {
    reads {
        p4_to_rxdma_header.udp_oflow_index : exact;
    }
    actions {
        udp_flow_info;
    }
    size : 1024;
}

action udp_start_q () {
    // ring the door bell for the qid
    modify_field(udp_scratch.udp_flow_qid, p4_to_rxdma_header.udp_flow_qid);
}

@pragma stage 0
table udp_q_drain {
    // MPU only table
    actions {
        udp_start_q;
    }
}

@pragma stage 4
@pragma table_write
@pragma index_table
table udp_flow_qid_allocator{
    // single entry SRAM table
    reads {
        // define a key that is always zero
        udp_flow_metadata.zero : exact;
    }
    actions {
        qid_alloc_free;
    }
    size : 1;
}
action qid_alloc_free(qid_bitmap) {
    modify_field(udp_scratch.qid_bitmap, qid_bitmap);
    modify_field(udp_flow_metadata.udp_flow_qid, 1 /* for allocated qid*/);
}

@pragma stage 4
@pragma hbm_table
@pragma index_table
table udp_flow_qid_update_virtual {
    // this is physically the same table as udp_flow_qid_update
    reads {
        udp_flow_metadata.udp_qid_tbl_idx : exact;
    }
    actions {
        read_update_qid;
    }
    size : 4096;
}

@pragma stage 5
@pragma hbm_table
@pragma table_write
@pragma index_table
table udp_flow_qid_update {
    reads {
        udp_flow_metadata.udp_qid_tbl_idx : exact;
    }
    actions {
        read_update_qid;
    }
    size : 4096;
}

action read_update_qid(qid) {
    // read when this is a pullup case, update when allocated
    modify_field(udp_flow_metadata.udp_flow_qid, qid);
}

control udp_flow_queuing {
    if (p4_to_rxdma_header.udp_queue_enable == TRUE) {
        if (p4_to_rxdma_header.udp_queue_drain == TRUE) {
            // qid comes from P4 pipeline (preserved)
            apply(udp_q_drain);
        } else {
            // perform udp flow lookup.
            // if delete flag is set, delete an entry
            apply(udp_flow_overflow0);
            if (p4_to_rxdma_header.udp_flow_lkp_continue == TRUE) {
                // tables are applied in consecutive stages.. flow_miss is updated
                // by each overflow table and is evaluated in the following stage
                apply(udp_flow_overflow1);
                apply(udp_flow_overflow2);
                apply(udp_flow_overflow3);
            }
            // flow_lkp_result is encoded in two bits as -
            // +----------------+----------------+-----------------------------------+
            // | flow_q_found   | flow_allocated | Action
            // +----------------+----------------+-----------------------------------+
            // | 0              |   0            | if enqueue/delete : ERROR, DROP
            // |                |                | else (udp flow miss) : DROP (for now)
            // +----------------+----------------+-----------------------------------+
            // | 0              |   1            | if enqueue/delete : ERROR
            // |                |                | else : allocate_qid
            // +----------------+----------------+-----------------------------------+
            // | 1              |   0            | if enqueue : enqueue the pkt to qid
            // |                |                | if delete : free_qid
            // |                |                | else : ERROR
            // +----------------+----------------+-----------------------------------+
            // | 1              |   1            | pull-up case
            // |                |                | - get old qid
            // |                |                | if enqueue : update qid table
            // |                |                | if delete : NA
            // |                |                | else : NA
            // +----------------+----------------+-----------------------------------+
            if (p4_to_rxdma_header.udp_flow_lkp_result == 0x0) {
                // nothing to do
            } else {
                if (p4_to_rxdma_header.udp_flow_lkp_result == 0x3) {
                    apply(udp_flow_qid_update_virtual);
                } else {
                    apply(udp_flow_qid_allocator); // to free qid on delete action
                }
                apply(udp_flow_qid_update);
            }
        }
    } else if (p4_to_rxdma_header.udp_flow_hash_lkp == TRUE) {
        // never execute this.. added to fool NCC w/ a non-existant parent hash table
        apply(udp_flow_hash0);
        apply(udp_flow_hash1);
        apply(udp_flow_hash2);
        apply(udp_flow_hash3);
    }
}
