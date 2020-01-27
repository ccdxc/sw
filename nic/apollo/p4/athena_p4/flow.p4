/*****************************************************************************/
/* Policy (unified for IPv4, IPv6 and L2)                                    */
/*****************************************************************************/
@pragma capi appdatafields index index_type
@pragma capi hwfields_access_api
action flow_hash(entry_valid, index, index_type,
                 hash1, hint1, hash2, hint2,
                 more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(p4i_to_p4e_header.index, index);
        modify_field(control_metadata.index, index);
        modify_field(p4i_to_p4e_header.index_type, index_type);
        modify_field(p4i_to_p4e_header.direction, control_metadata.direction);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.flow_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.flow_hash,
                     scratch_metadata.flow_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash1)) {
            modify_field(scratch_metadata.flow_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash2)) {
            modify_field(scratch_metadata.flow_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.flow_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(control_metadata.flow_ohash_lkp, TRUE);
            modify_field(ingress_recirc_header.flow_ohash, scratch_metadata.flow_hint);
        } else {
            modify_field(ingress_recirc_header.flow_done, TRUE);
            modify_field(p4i_to_p4e_header.index, 0);
            modify_field(control_metadata.index, 0);
        }
    } else {
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(p4i_to_p4e_header.index, 0);
        modify_field(control_metadata.index, 0);
        modify_field(control_metadata.flow_miss, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
}

@pragma stage 1
@pragma hbm_table
table flow {
    reads {
        key_metadata.vnic_id            : exact;
        key_metadata.smac               : exact;
        key_metadata.dmac               : exact;
        key_metadata.src                : exact;
        key_metadata.dst                : exact;
        key_metadata.proto              : exact;
        key_metadata.sport              : exact;
        key_metadata.dport              : exact;
        key_metadata.ktype              : exact;
    }
    actions {
        flow_hash;
    }
    size : FLOW_TABLE_SIZE;
}

@pragma stage 2
@pragma hbm_table
@pragma overflow_table flow
table flow_ohash {
    reads {
        ingress_recirc_header.flow_ohash : exact;
    }
    actions {
        flow_hash;
    }
    size : FLOW_OHASH_TABLE_SIZE;
}

/*****************************************************************************/
/* Policy (unified for IPv4, IPv6 and L2)                                    */
/*****************************************************************************/
@pragma capi appdatafields index index_type
@pragma capi hwfields_access_api
action ipv4_flow_hash(entry_valid, index, index_type, pad,
                 hash1, hint1, hash2, hint2, hash3, hint3,
                 more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(p4i_to_p4e_header.index, index);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.flow_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.flow_hash,
                     scratch_metadata.flow_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash1)) {
            modify_field(scratch_metadata.flow_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash2)) {
            modify_field(scratch_metadata.flow_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash3)) {
            modify_field(scratch_metadata.flow_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.flow_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(control_metadata.flow_ohash_lkp, TRUE);
            modify_field(ingress_recirc_header.flow_ohash, scratch_metadata.flow_hint);
        } else {
            modify_field(ingress_recirc_header.flow_done, TRUE);
            modify_field(p4i_to_p4e_header.index, 0);
        }
    } else {
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(p4i_to_p4e_header.index, 0);
        modify_field(p4i_to_p4e_header.flow_miss, TRUE);
        modify_field(control_metadata.flow_miss, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flow_data_pad, pad);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
    modify_field(scratch_metadata.flow_hash, hash3);
}

@pragma stage 4
@pragma hbm_table
table ipv4_flow {
    reads {
        key_metadata.vnic_id    : exact;
        key_metadata.ipv4_src   : exact;
        key_metadata.ipv4_dst   : exact;
        key_metadata.proto      : exact;
        key_metadata.sport      : exact;
        key_metadata.dport      : exact;
    }
    actions {
        ipv4_flow_hash;
    }
    size : IPV4_FLOW_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
@pragma overflow_table ipv4_flow
table ipv4_flow_ohash {
    reads {
        ingress_recirc_header.flow_ohash : exact;
    }
    actions {
        ipv4_flow_hash;
    }
    size : IPV4_FLOW_OHASH_TABLE_SIZE;
}

control flow_lookup {
    if (ingress_recirc_header.valid == FALSE) {
        if (control_metadata.skip_flow_lkp == FALSE) {
            apply(flow);
        }
    }
    if (control_metadata.flow_ohash_lkp == TRUE) {
        apply(flow_ohash);
    }
    // Dummy v4 table for generating FTL structs/apis
    // Never launched as the predicate never been set
    if (control_metadata.launch_v4 == TRUE) {
        apply(ipv4_flow);
        apply(ipv4_flow_ohash);
    }
}
