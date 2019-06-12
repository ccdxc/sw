/******************************************************************************/
/* IP mapping on TX direction in TXDMA                                        */
/******************************************************************************/
@pragma capi appdatafields nexthop_group_index
@pragma capi hwfields_access_api
action mapping_info(entry_valid, nexthop_group_index,
                    dst_slot_id_valid, dst_slot_id,
                    hash1, hint1, hash2, hint2, hash3, hint3,
                    hash4, hint4, hash5, hint5, hash6, hint6,
                    hash7, hint7, hash8, hint8,
                    more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(session_info_hint.nexthop_idx, nexthop_group_index);

        // Tx: For NH_TYPE=IP, set src_ip flag to 01 (public IP from
        // local mapping table index) and if mapping table finds entry then
        // src_ip rewrite flag to 10 (from service mapping table index)
        modify_field(session_info_hint.tx_rewrite_flags_src_ip, 10);

        // Rx: For NH_TYPE=IP, set the dst_ip flag to 01 (CA from mapping -
        // from local-ip mapping table index)and if mapping table finds entry then
        // dst_ip rewrite flag to 10 (CA from service mapping table index)
        modify_field(session_info_hint.rx_rewrite_flags_dst_ip, 10);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.mapping_hash) and setup lookup in overflow
        // table
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.mapping_hash == hash1)) {
            modify_field(scratch_metadata.mapping_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.mapping_hash == hash2)) {
            modify_field(scratch_metadata.mapping_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.mapping_hash == hash3)) {
            modify_field(scratch_metadata.mapping_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.mapping_hash == hash4)) {
            modify_field(scratch_metadata.mapping_hint, hint4);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.mapping_hash == hash5)) {
            modify_field(scratch_metadata.mapping_hint, hint5);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.mapping_hash == hash6)) {
            modify_field(scratch_metadata.mapping_hint, hint6);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.mapping_hash == hash7)) {
            modify_field(scratch_metadata.mapping_hint, hint7);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.mapping_hash == hash8)) {
            modify_field(scratch_metadata.mapping_hint, hint8);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.mapping_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(txdma_control.mapping_ohash, scratch_metadata.mapping_hint);
            modify_field(txdma_control.mapping_ohash_lkp, TRUE);
        }
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.mapping_hash, hash1);
    modify_field(scratch_metadata.mapping_hash, hash2);
    modify_field(scratch_metadata.mapping_hash, hash3);
    modify_field(scratch_metadata.mapping_hash, hash4);
    modify_field(scratch_metadata.mapping_hash, hash5);
    modify_field(scratch_metadata.mapping_hash, hash6);
    modify_field(scratch_metadata.mapping_hash, hash7);
    modify_field(scratch_metadata.mapping_hash, hash8);
}

@pragma stage 0
@pragma hbm_table
table mapping {
    reads {
        txdma_control.vpc_id   : exact;
        rx_to_tx_hdr.remote_ip : exact;
    }
    actions {
        mapping_info;
    }
    size : MAPPING_TABLE_SIZE;
}

@pragma stage 1
@pragma hbm_table
@pragma overflow_table mapping
table mapping_ohash {
    reads {
        txdma_control.mapping_ohash : exact;
    }
    actions {
       mapping_info;
    }
    size : MAPPING_OHASH_TABLE_SIZE;
}

control mapping {
    // TMP: Fix this logic after proper placement of this table
    //if (control_metadata.direction == TX_FROM_HOST) {
        apply(mapping);
        //if (txdma_control.mapping_ohash_lkp == TRUE) {
            apply(mapping_ohash);
        //}
    //}
}
