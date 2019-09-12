/******************************************************************************/
/* Local mapping                                                              */
/******************************************************************************/
action local_mapping_info(entry_valid, vnic_id,
                          hash1, hint1, hash2, hint2, hash3, hint3,
                          hash4, hint4, hash5, hint5, hash6, hint6,
                          hash7, hint7, hash8, hint8, hash9, hint9,
                          hash10, hint10, more_hashes, more_hints) {

    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        if (vnic_id != 0) {
            modify_field(vnic_metadata.vnic_id, vnic_id);
        }
        modify_field(ingress_recirc.local_mapping_done, TRUE);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.local_mapping_hash) and setup lookup in
        // overflow table
        modify_field(scratch_metadata.local_mapping_hash,
                     scratch_metadata.local_mapping_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash1)) {
            modify_field(scratch_metadata.local_mapping_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash2)) {
            modify_field(scratch_metadata.local_mapping_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash3)) {
            modify_field(scratch_metadata.local_mapping_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash4)) {
            modify_field(scratch_metadata.local_mapping_hint, hint4);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash5)) {
            modify_field(scratch_metadata.local_mapping_hint, hint5);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash6)) {
            modify_field(scratch_metadata.local_mapping_hint, hint6);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash7)) {
            modify_field(scratch_metadata.local_mapping_hint, hint7);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash8)) {
            modify_field(scratch_metadata.local_mapping_hint, hint8);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash9)) {
            modify_field(scratch_metadata.local_mapping_hint, hint9);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_mapping_hash == hash10)) {
            modify_field(scratch_metadata.local_mapping_hint, hint10);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.local_mapping_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(ingress_recirc.local_mapping_ohash,
                         scratch_metadata.local_mapping_hint);
            modify_field(control_metadata.local_mapping_ohash_lkp, TRUE);
        }
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.local_mapping_hash, hash1);
    modify_field(scratch_metadata.local_mapping_hash, hash2);
    modify_field(scratch_metadata.local_mapping_hash, hash3);
    modify_field(scratch_metadata.local_mapping_hash, hash4);
    modify_field(scratch_metadata.local_mapping_hash, hash5);
    modify_field(scratch_metadata.local_mapping_hash, hash6);
    modify_field(scratch_metadata.local_mapping_hash, hash7);
    modify_field(scratch_metadata.local_mapping_hash, hash8);
    modify_field(scratch_metadata.local_mapping_hash, hash9);
    modify_field(scratch_metadata.local_mapping_hash, hash10);
}

@pragma stage 2
@pragma hbm_table
table local_mapping {
    reads {
        key_metadata.local_mapping_lkp_type : exact;
        key_metadata.local_mapping_lkp_id   : exact;
        key_metadata.local_mapping_lkp_addr : exact;
    }
    actions {
        local_mapping_info;
    }
    size : LOCAL_MAPPING_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table local_mapping
table local_mapping_ohash {
    reads {
        ingress_recirc.local_mapping_ohash  : exact;
    }
    actions {
        local_mapping_info;
    }
    size : LOCAL_MAPPING_OHASH_TABLE_SIZE;
}

control local_mapping {
    if (ingress_recirc.valid == FALSE) {
        apply(local_mapping);
    }
    if (control_metadata.local_mapping_ohash_lkp == TRUE) {
        apply(local_mapping_ohash);
    }
}
