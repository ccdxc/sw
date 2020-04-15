/*****************************************************************************/
/* DNAT Mapping Table (unified for IPv4 and IPv6)                            */
/*****************************************************************************/
@pragma capi appdatafields addr_type addr
@pragma capi hwfields_access_api
action dnat(pad1, pad2, addr_type, addr, epoch,
                hash1, hint1, hash2, hint2,
                hash3, hint3, hash4, hint4,
                hash5, hint5, more_hashes, more_hints, entry_valid) {

    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(ingress_recirc_header.dnat_done, TRUE);
        modify_field(key_metadata.src, addr);
        modify_field(key_metadata.ktype, addr_type);
        modify_field(p4i_to_p4e_header.dnat_epoch, epoch);


        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.dnat_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.dnat_hash,
                     scratch_metadata.dnat_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.dnat_hash == hash1)) {
            modify_field(scratch_metadata.dnat_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.dnat_hash == hash2)) {
            modify_field(scratch_metadata.dnat_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.dnat_hash == hash3)) {
            modify_field(scratch_metadata.dnat_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.dnat_hash == hash4)) {
            modify_field(scratch_metadata.dnat_hint, hint4);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.dnat_hash == hash5)) {
            modify_field(scratch_metadata.dnat_hint, hint5);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.dnat_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(control_metadata.dnat_ohash_lkp, TRUE);
            modify_field(ingress_recirc_header.dnat_ohash, scratch_metadata.dnat_hint);
        } else {
            modify_field(ingress_recirc_header.dnat_done, TRUE);
        }
    }
    else {
        modify_field(ingress_recirc_header.dnat_done, TRUE);
    }
    modify_field(scratch_metadata.pad32, pad1);
    modify_field(scratch_metadata.pad3, pad2);
    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.dnat_hash, hash1);
    modify_field(scratch_metadata.dnat_hash, hash2);
    modify_field(scratch_metadata.dnat_hash, hash3);
    modify_field(scratch_metadata.dnat_hash, hash4);
    modify_field(scratch_metadata.dnat_hash, hash5);
}

@pragma stage 1
@pragma hbm_table
@pragma capi_bitfields_struct
table dnat {
    reads {
        key_metadata.vnic_id            : exact;
        key_metadata.ktype              : exact;
        key_metadata.src                : exact;
    }
    actions {
        dnat;
    }
    size : DNAT_TABLE_SIZE;
}

@pragma stage 2
@pragma hbm_table
@pragma overflow_table dnat
table dnat_ohash {
    reads {
        ingress_recirc_header.dnat_ohash : exact;
    }
    actions {
        dnat;
    }
    size : DNAT_OHASH_TABLE_SIZE;
}

control dnat_lookup {
    if (ingress_recirc_header.valid == FALSE) {
        if (control_metadata.skip_dnat_lkp == FALSE) {
            apply(dnat);
        }
    }
    if (control_metadata.dnat_ohash_lkp == TRUE) {
        apply(dnat_ohash);
    }
}
