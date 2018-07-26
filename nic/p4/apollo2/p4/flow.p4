/*****************************************************************************/
/* Policy                                                                    */
/*****************************************************************************/
action flow_hash(entry_valid, epoch, flow_index, hash1, hint1,
                 hash2, hint2, hash3, hint3, hash4, hint4, hashn, hintn) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        if (service_header.epoch == epoch) {
            modify_field(control_metadata.flow_index, flow_index);
        }
        modify_field(service_header.flow_done, TRUE);

        // if hardware register indicates miss, compare hints and setup
        // to perform lookup in overflow table
        modify_field(control_metadata.flow_ohash_lkp, TRUE);
        modify_field(service_header.flow_ohash, scratch_metadata.flow_hint);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.epoch, epoch);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
    modify_field(scratch_metadata.flow_hash, hash3);
    modify_field(scratch_metadata.flow_hash, hash4);
    modify_field(scratch_metadata.flow_hash, hashn);
    modify_field(scratch_metadata.flow_hint, hint1);
    modify_field(scratch_metadata.flow_hint, hint2);
    modify_field(scratch_metadata.flow_hint, hint3);
    modify_field(scratch_metadata.flow_hint, hint4);
    modify_field(scratch_metadata.flow_hint, hintn);
}

action flow_info(permit_packets, permit_bytes, deny_packets, deny_bytes,
                  nexthop_index, drop) {
    modify_field(scratch_metadata.flag, drop);
    if (drop == FALSE) {
        modify_field(scratch_metadata.in_packets, permit_packets);
        modify_field(scratch_metadata.in_bytes, permit_bytes);
        modify_field(rewrite_metadata.nexthop_index, nexthop_index);
    } else {
        modify_field(scratch_metadata.in_packets, deny_packets);
        modify_field(scratch_metadata.in_bytes, deny_bytes);
    }
}

@pragma stage 2
@pragma hbm_table
table flow {
    reads {
        vnic_metadata.vcn_id        : exact;
        key_metadata.ktype          : exact;
        key_metadata.src            : exact;
        key_metadata.dst            : exact;
        key_metadata.proto          : exact;
        key_metadata.sport          : exact;
        key_metadata.dport          : exact;
    }
    actions {
        flow_hash;
    }
    size : POLICY_TABLE_SIZE;
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
    size : POLICY_OHASH_TABLE_SIZE;
}

// TODO: move this to stage 4. NCC is failing
@pragma stage 5
@pragma hbm_table
table flow_info {
    reads {
        control_metadata.flow_index   : exact;
    }
    actions {
        flow_info;
    }
    size : POLICY_INFO_TABLE_SIZE;
}

control flow_lookup {
    if (service_header.valid == FALSE) {
        if (control_metadata.skip_flow_lkp == TRUE) {
            apply(flow);
        }
    }
    if (control_metadata.flow_ohash_lkp == TRUE) {
        apply(flow_ohash);
    }
    apply(flow_info);
}
