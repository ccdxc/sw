/******************************************************************************/
/* Endpoint mapping table                                                     */
/******************************************************************************/
action ep_mapping_info(entry_valid,
                       epoch1, nexthop_index1, epoch2, nexthop_index2,
                       hash1, hint1, hash2, hint2, hash3, hint3, hash4, hint4,
                       hash5, hint5, hash6, hint6, hash7, hint7, hash8, hint8,
                       hashn, hintn) {

    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        if (service_header.epoch == epoch1) {
            modify_field(rewrite_metadata.nexthop_index, nexthop_index1);
        }
        if (service_header.epoch == epoch2) {
            modify_field(rewrite_metadata.nexthop_index, nexthop_index2);
        }
        modify_field(service_header.ep_mapping_done, TRUE);

        // if hardware register indicates miss, compare hints and setup
        // to perform lookup in overflow table
        modify_field(service_header.ep_mapping_ohash, hint1);
        modify_field(control_metadata.ep_mapping_ohash_lkp, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.epoch, epoch1);
    modify_field(scratch_metadata.epoch, epoch2);
    modify_field(scratch_metadata.ep_hash, hash1);
    modify_field(scratch_metadata.ep_hash, hash2);
    modify_field(scratch_metadata.ep_hash, hash3);
    modify_field(scratch_metadata.ep_hash, hash4);
    modify_field(scratch_metadata.ep_hash, hash5);
    modify_field(scratch_metadata.ep_hash, hash6);
    modify_field(scratch_metadata.ep_hash, hash7);
    modify_field(scratch_metadata.ep_hash, hash8);
    modify_field(scratch_metadata.ep_hash, hashn);
    modify_field(scratch_metadata.ep_hint, hint1);
    modify_field(scratch_metadata.ep_hint, hint2);
    modify_field(scratch_metadata.ep_hint, hint3);
    modify_field(scratch_metadata.ep_hint, hint4);
    modify_field(scratch_metadata.ep_hint, hint5);
    modify_field(scratch_metadata.ep_hint, hint6);
    modify_field(scratch_metadata.ep_hint, hint7);
    modify_field(scratch_metadata.ep_hint, hint8);
    modify_field(scratch_metadata.ep_hint, hintn);
}

@pragma stage 1
@pragma hbm_table
table ep_mapping {
    reads {
        control_metadata.subnet_id  : exact;
        key_metadata.dst            : exact;
    }
    actions {
        ep_mapping_info;
    }
    size : EP_MAPPING_TABLE_SIZE;
}

@pragma stage 2
@pragma hbm_table
@pragma overflow_table ep_mapping
table ep_mapping_ohash {
    reads {
        service_header.ep_mapping_ohash : exact;
    }
    actions {
        ep_mapping_info;
    }
    size : EP_MAPPING_OHASH_TABLE_SIZE;
}

control ep_lookup {
    if (service_header.valid == FALSE) {
        apply(ep_mapping);
    }
    if (control_metadata.ep_mapping_ohash_lkp == TRUE) {
        apply(ep_mapping_ohash);
    }
}
