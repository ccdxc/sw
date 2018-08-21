/*****************************************************************************/
/* Policy                                                                    */
/*****************************************************************************/
action flow_hash(entry_valid, flow_index,
                 hash1, hint1, hash2, hint2, hash3, hint3, hash4, hint4,
                 hash5, hint5, more_hashes, more_hints, ohash_entry) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(service_header.flow_done, TRUE);
        modify_field(control_metadata.flow_index, flow_index);

        // if hardware register indicates miss, compare hints and setup
        // to perform lookup in overflow table
        modify_field(control_metadata.flow_ohash_lkp, TRUE);
        modify_field(service_header.flow_ohash, scratch_metadata.flow_hint);
    } else {
        modify_field(service_header.flow_done, TRUE);
        modify_field(control_metadata.flow_index, 0);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
    modify_field(scratch_metadata.flow_hash, hash3);
    modify_field(scratch_metadata.flow_hash, hash4);
    modify_field(scratch_metadata.flow_hash, hash5);
    modify_field(scratch_metadata.flow_hint, hint1);
    modify_field(scratch_metadata.flow_hint, hint2);
    modify_field(scratch_metadata.flow_hint, hint3);
    modify_field(scratch_metadata.flow_hint, hint4);
    modify_field(scratch_metadata.flow_hint, hint5);
    modify_field(scratch_metadata.flag, more_hashes);
    modify_field(scratch_metadata.flow_hint, more_hints);
    modify_field(scratch_metadata.flag, ohash_entry);
}

action flow_info(permit_packets, permit_bytes, deny_packets, deny_bytes, drop) {
    modify_field(scratch_metadata.flag, drop);
    if (drop == FALSE) {
        add(scratch_metadata.in_packets, permit_packets, 1);
        add(scratch_metadata.in_bytes, permit_bytes, capri_p4_intrinsic.packet_len);
    } else {
        modify_field(control_metadata.p4i_drop_reason, 1 << DROP_FLOW_HIT);
        add(scratch_metadata.in_packets, deny_packets, 1);
        add(scratch_metadata.in_bytes, deny_bytes, capri_p4_intrinsic.packet_len);
        drop_packet();
    }
}

@pragma stage 2
@pragma hbm_table
table flow {
    reads {
        vnic_metadata.local_vnic_tag    : exact;
        key_metadata.ktype              : exact;
        key_metadata.src                : exact;
        key_metadata.dst                : exact;
        key_metadata.proto              : exact;
        key_metadata.sport              : exact;
        key_metadata.dport              : exact;
    }
    actions {
        flow_hash;
    }
    size : FLOW_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table flow
table flow_ohash {
    reads {
        service_header.flow_ohash   : exact;
    }
    actions {
        flow_hash;
    }
    size : FLOW_OHASH_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
@pragma table_write
table flow_info {
    reads {
        control_metadata.flow_index   : exact;
    }
    actions {
        flow_info;
    }
    size : FLOW_INFO_TABLE_SIZE;
}

control flow_lookup {
    if (service_header.valid == FALSE) {
        if (control_metadata.skip_flow_lkp == FALSE) {
            apply(flow);
        }
    }
    if (control_metadata.flow_ohash_lkp == TRUE) {
        apply(flow_ohash);
    }
    apply(flow_info);
}
