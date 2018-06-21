/*****************************************************************************/
/* Route lookup                                                              */
/*****************************************************************************/
action lpm_init(base_addr) {
    modify_field(lpm_metadata.addr, base_addr);
    modify_field(lpm_metadata.base_addr, base_addr);
}

action lpm_s0(done, data) {
    modify_field(scratch_metadata.lpm_data, data);
    modify_field(lpm_metadata.done, done);
    if (done == TRUE) {
        modify_field(rewrite_metadata.nexthop_index, scratch_metadata.lpm_data);
    } else {
        modify_field(lpm_metadata.addr, (lpm_metadata.base_addr +
                                         key_metadata.dst +
                                         scratch_metadata.lpm_data));
    }
}

action lpm_s1(done, data) {
    modify_field(scratch_metadata.lpm_data, data);
    modify_field(lpm_metadata.done, done);
    if (done == TRUE) {
        modify_field(rewrite_metadata.nexthop_index, scratch_metadata.lpm_data);
    } else {
        modify_field(lpm_metadata.addr, (lpm_metadata.base_addr +
                                         key_metadata.dst +
                                         scratch_metadata.lpm_data));
    }
}

action lpm_s2(data) {
    modify_field(scratch_metadata.lpm_data, data);
    modify_field(rewrite_metadata.nexthop_index, scratch_metadata.lpm_data);
}

@pragma stage 2
@pragma hbm_table
@pragma raw_index_table
table lpm_s0 {
    reads {
        lpm_metadata.addr : exact;
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
        lpm_metadata.addr : exact;
    }
    actions {
        lpm_s1;
    }
}

@pragma stage 5
@pragma hbm_table
@pragma raw_index_table
table lpm_s2 {
    reads {
        lpm_metadata.addr : exact;
    }
    actions {
        lpm_s2;
    }
}

control lpm_lookup {
    if (lpm_metadata.done == FALSE) {
        apply(lpm_s0);
    }
    if (lpm_metadata.done == FALSE) {
        apply(lpm_s1);
    }
    if (lpm_metadata.done == FALSE) {
        apply(lpm_s2);
    }
}
