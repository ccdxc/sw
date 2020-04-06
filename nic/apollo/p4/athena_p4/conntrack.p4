action conntrack(valid_flag, flow_type, flow_state, timestamp) {

    modify_field(scratch_metadata.flow_type, flow_type);
    modify_field(scratch_metadata.flow_state, flow_state);
    modify_field(scratch_metadata.timestamp, timestamp);
    modify_field(scratch_metadata.flag, valid_flag);
}




@pragma stage 2
@pragma hbm_table
@pragma index_table
@pragma capi_bitfields_struct
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
