/*****************************************************************************/
/* Policy                                                                    */
/*****************************************************************************/
action policy_info(entry_valid, epoch, policy_index, hash1, hint1,
                   hash2, hint2, hash3, hint3, hash4, hint4, hashn, hintn) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        if (service_header.epoch == epoch) {
            modify_field(control_metadata.policy_index, policy_index);
        }
        modify_field(service_header.policy_done, TRUE);

        // if hardware register indicates miss, compare hints and setup
        // to perform lookup in overflow table
        modify_field(control_metadata.policy_ohash_lkp, TRUE);
        modify_field(service_header.policy_ohash, scratch_metadata.policy_hint);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.epoch, epoch);
    modify_field(scratch_metadata.policy_hash, hash1);
    modify_field(scratch_metadata.policy_hash, hash2);
    modify_field(scratch_metadata.policy_hash, hash3);
    modify_field(scratch_metadata.policy_hash, hash4);
    modify_field(scratch_metadata.policy_hash, hashn);
    modify_field(scratch_metadata.policy_hint, hint1);
    modify_field(scratch_metadata.policy_hint, hint2);
    modify_field(scratch_metadata.policy_hint, hint3);
    modify_field(scratch_metadata.policy_hint, hint4);
    modify_field(scratch_metadata.policy_hint, hintn);
}

action policy_stats(permit_packets, permit_bytes, deny_packets, deny_bytes,
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

@pragma stage 3
@pragma hbm_table
table policy {
    reads {
        control_metadata.subnet_id  : exact;
        key_metadata.ktype          : exact;
        key_metadata.src            : exact;
        key_metadata.dst            : exact;
        key_metadata.proto          : exact;
        key_metadata.sport          : exact;
        key_metadata.dport          : exact;
    }
    actions {
        policy_info;
    }
    size : POLICY_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
@pragma overflow_table policy
table policy_ohash {
    reads {
        service_header.policy_ohash   : exact;
    }
    actions {
        policy_info;
    }
    size : POLICY_OHASH_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table policy_stats {
    reads {
        control_metadata.policy_index   : exact;
    }
    actions {
        policy_stats;
    }
    size : POLICY_STATS_TABLE_SIZE;
}

control policy_lookup {
    if (service_header.valid == FALSE) {
        if (control_metadata.skip_policy_lkp == TRUE) {
            apply(policy);
        }
    }
    if (control_metadata.policy_ohash_lkp == TRUE) {
        apply(policy_ohash);
    }
    apply(policy_stats);
}
