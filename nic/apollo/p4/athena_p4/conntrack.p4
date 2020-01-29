action conntrack(flow_type, flow_state, timestamp) {

    modify_field(scratch_metadata.flow_type, flow_type);
    modify_field(scratch_metadata.flow_state, flow_state);
    modify_field(scratch_metadata.timestamp, timestamp);
}




@pragma stage 3
@pragma hbm_table
@pragma index_table
table conntrack {
    reads {
        control_metadata.conntrack_index : exact;
    }
    actions {
        conntrack;
    }
    size : CONNTRACK_TABLE_SIZE;
}


control conntrack_state_update {
    if (control_metadata.conntrack_index_valid == TRUE) {
        apply(conntrack);
    }
}
