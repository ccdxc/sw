/******************************************************************************/
/* mapping table to derive tags                                               */
/******************************************************************************/
@pragma capi appdatafields tag_idx
@pragma capi hwfields_access_api
action rxdma_mapping_info(entry_valid, tag_idx, more_hashes, hash1, hint1,
                          hash2, hint2, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(scratch_metadata.field20, tag_idx);
        modify_field(lpm_metadata.mapping_tag_idx, scratch_metadata.field20);
        modify_field(p4_to_rxdma.mapping_done, TRUE);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.mapping_hash) and setup lookup in
        // overflow table
        modify_field(scratch_metadata.mapping_hash,
                     scratch_metadata.mapping_hash);
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
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.mapping_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(lpm_metadata.mapping_ohash,
                         scratch_metadata.mapping_hint);
            modify_field(p4_to_rxdma.mapping_ohash_lkp, TRUE);
        } else {
            modify_field(p4_to_rxdma.mapping_done, TRUE);
        }
    } else {
        modify_field(p4_to_rxdma.mapping_done, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.mapping_hash, hash1);
    modify_field(scratch_metadata.mapping_hash, hash2);
}

@pragma stage 2
@pragma hbm_table
table rxdma_mapping {
    reads {
        p4_to_rxdma.iptype      : exact;
        p4_to_rxdma.vpc_id      : exact;
        rx_to_tx_hdr.remote_ip  : exact;
    }
    actions {
        rxdma_mapping_info;
    }
    size : MAPPING_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table rxdma_mapping
table rxdma_mapping_ohash {
    reads {
        lpm_metadata.mapping_ohash  : exact;
    }
    actions {
        rxdma_mapping_info;
    }
    size : MAPPING_OHASH_TABLE_SIZE;
}

control rxdma_mapping {
    if (p4_to_rxdma.mapping_done == FALSE) {
        if (p4_to_rxdma.mapping_ohash_lkp == FALSE) {
            apply(rxdma_mapping);
        } else {
            apply(rxdma_mapping_ohash);
        }
    }
}
