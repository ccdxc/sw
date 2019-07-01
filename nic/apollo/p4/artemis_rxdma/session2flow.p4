
action session2flow (pad0, iflow_ipaf, iflow_parent_hint, iflow_ind, 
                     pad1, rflow_ipaf, rflow_parent_hint, rflow_ind) {
    modify_field(scratch_metadata.field7, pad0);
    modify_field(scratch_metadata.flag, iflow_ipaf);
    modify_field(scratch_metadata.flag, iflow_parent_hint);
    modify_field(scratch_metadata.field23, iflow_ind);

    modify_field(scratch_metadata.field7, pad1);
    modify_field(scratch_metadata.flag, rflow_ipaf);
    modify_field(scratch_metadata.flag, rflow_parent_hint);
    modify_field(scratch_metadata.field23, rflow_ind);
}

@pragma stage 2
@pragma hbm_table
@pragma index_table
table session2flow {
    reads {
        capri_rxdma_intr.qid   : exact;
    }
    actions {
        session2flow;
    }
    size : SESSION_TABLE_SIZE;
}

control session2flow {
    apply(session2flow);
}
