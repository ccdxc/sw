/*****************************************************************************/
/* Policy (unified for IPv4, IPv6 and L2)                                    */
/*****************************************************************************/
@pragma capi appdatafields session_index
@pragma capi hwfields_access_api
action flow_hash(entry_valid, session_index, pad,
                 hash1, hint1, hash2, hint2, hash3, hint3,
                 more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(p4i_to_p4e_header.session_index, session_index);

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
            modify_field(p4i_to_p4e_header.session_index, 0);
        }
    } else {
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(p4i_to_p4e_header.session_index, 0);
        modify_field(p4i_to_p4e_header.flow_miss, TRUE);
        modify_field(control_metadata.flow_miss, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flow_data_pad, pad);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
    modify_field(scratch_metadata.flow_hash, hash3);
}

@pragma stage 1
@pragma hbm_table
table flow {
    reads {
        key_metadata.tenant_id          : exact;
        key_metadata.src                : exact;
        key_metadata.dst                : exact;
        key_metadata.proto              : exact;
        key_metadata.sport              : exact;
        key_metadata.dport              : exact;
        key_metadata.vlan               : exact;
        key_metadata.tcp_flags          : exact;
        key_metadata.ktype              : exact;
        key_metadata.ingress_port       : exact;
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
        ingress_recirc_header.flow_ohash   : exact;
    }
    actions {
        flow_hash;
    }
    size : FLOW_OHASH_TABLE_SIZE;
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
}
