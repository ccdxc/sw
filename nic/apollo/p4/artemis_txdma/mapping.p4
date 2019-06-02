/******************************************************************************/
/* IP mapping on TX direction in TXDMA                                        */
/******************************************************************************/
@pragma capi appdatafields nexthop_group_index dst_slot_id_valid dst_slot_id
@pragma capi hwfields_access_api
action mapping_info(entry_valid, nexthop_group_index,
                    dst_slot_id_valid, dst_slot_id,
                    hash1, hint1, hash2, hint2, hash3, hint3,
                    hash4, hint4, hash5, hint5, hash6, hint6,
                    hash7, hint7, hash8, hint8,
                    more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        //modify_field(egress_service_header.remote_vnic_mapping_tx_done, TRUE);
        //modify_field(txdma_to_p4e_header.nexthop_group_index,
        //             nexthop_group_index);
        //modify_field(rewrite_metadata.dst_slot_id_valid, dst_slot_id_valid);
        //modify_field(rewrite_metadata.dst_slot_id, dst_slot_id);

        // TMP: For now access them
        modify_field(scratch_metadata.remote_vnic_mapping_tx_done, TRUE);
        modify_field(scratch_metadata.nexthop_group_index, nexthop_group_index);
        modify_field(scratch_metadata.dst_slot_id_valid, dst_slot_id_valid);
        modify_field(scratch_metadata.dst_slot_id, dst_slot_id);


        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.vnic_mapping_hash) and setup lookup in overflow
        // table
        modify_field(scratch_metadata.vnic_mapping_hash,
                     scratch_metadata.vnic_mapping_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash1)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash2)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash3)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash4)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint4);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash5)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint5);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash6)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint6);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash7)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint7);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash8)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint8);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.vnic_mapping_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            //modify_field(egress_service_header.remote_vnic_mapping_tx_ohash,
            //             scratch_metadata.vnic_mapping_hint);
            // modify_field(control_metadata.remote_vnic_mapping_tx_ohash_lkp, TRUE);
            // TMP: For now instead of above code, access them from scratch data
            modify_field(scratch_metadata.remote_vnic_mapping_tx_ohash,
                         scratch_metadata.vnic_mapping_hint);
            modify_field(txdma_control.remote_vnic_mapping_tx_ohash_lkp, TRUE);
        }
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.vnic_mapping_hash, hash1);
    modify_field(scratch_metadata.vnic_mapping_hash, hash2);
    modify_field(scratch_metadata.vnic_mapping_hash, hash3);
    modify_field(scratch_metadata.vnic_mapping_hash, hash4);
    modify_field(scratch_metadata.vnic_mapping_hash, hash5);
    modify_field(scratch_metadata.vnic_mapping_hash, hash6);
    modify_field(scratch_metadata.vnic_mapping_hash, hash7);
    modify_field(scratch_metadata.vnic_mapping_hash, hash8);
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
        txdma_control.remote_vnic_mapping_tx_ohash : exact;
    }
    actions {
       mapping_info;
    }
    size : MAPPING_OHASH_TABLE_SIZE;
}

control mapping {
    // TMP: Fix this logic after proper placement of this table
    //if (control_metadata.direction == TX_FROM_HOST) {
        //if (egress_service_header.valid == FALSE) {
            apply(mapping);
        //}
        //if (txdma_control.remote_vnic_mapping_tx_ohash_lkp == TRUE) {
            apply(mapping_ohash);
        //}
    //}
}
