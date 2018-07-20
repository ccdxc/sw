/******************************************************************************/
/*                                                                            */
/******************************************************************************/
action local_ip_mapping_info(entry_valid, hash1, hint1, hash2, hint2,
                             hash3, hint3, hash4, hint4, hash5, hint5,
                             hash6, hint6, hash7, hint7, hash8, hint8,
                             hashn, hintn) {

    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(service_header.local_ip_mapping_done, TRUE);

        // if hardware register indicates miss, compare hints and setup
        // to perform lookup in overflow table
        modify_field(service_header.local_ip_mapping_ohash,
                     scratch_metadata.local_ip_hint);
        modify_field(control_metadata.local_ip_mapping_ohash_lkp, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.local_ip_hash, hash1);
    modify_field(scratch_metadata.local_ip_hash, hash2);
    modify_field(scratch_metadata.local_ip_hash, hash3);
    modify_field(scratch_metadata.local_ip_hash, hash4);
    modify_field(scratch_metadata.local_ip_hash, hash5);
    modify_field(scratch_metadata.local_ip_hash, hash6);
    modify_field(scratch_metadata.local_ip_hash, hash7);
    modify_field(scratch_metadata.local_ip_hash, hash8);
    modify_field(scratch_metadata.local_ip_hash, hashn);
    modify_field(scratch_metadata.local_ip_hint, hint1);
    modify_field(scratch_metadata.local_ip_hint, hint2);
    modify_field(scratch_metadata.local_ip_hint, hint3);
    modify_field(scratch_metadata.local_ip_hint, hint4);
    modify_field(scratch_metadata.local_ip_hint, hint5);
    modify_field(scratch_metadata.local_ip_hint, hint6);
    modify_field(scratch_metadata.local_ip_hint, hint7);
    modify_field(scratch_metadata.local_ip_hint, hint8);
    modify_field(scratch_metadata.local_ip_hint, hintn);
}

@pragma stage 1
@pragma hbm_table
table local_ip_mapping {
    reads {
        vnic_metadata.vnic_id               : exact;
        control_metadata.mapping_lkp_addr   : exact;
    }
    actions {
        local_ip_mapping_info;
    }
    size : VNIC_IP_MAPPING_TABLE_SIZE;
}

@pragma stage 2
@pragma hbm_table
@pragma overflow_table local_ip_mapping
table local_ip_mapping_ohash {
    reads {
        service_header.local_ip_mapping_ohash : exact;
    }
    actions {
        local_ip_mapping_info;
    }
    size : VNIC_IP_MAPPING_OHASH_TABLE_SIZE;
}

control local_ip_mapping {
    if (service_header.valid == FALSE) {
        apply(local_ip_mapping);
    }
    if (control_metadata.local_ip_mapping_ohash_lkp == TRUE) {
        apply(local_ip_mapping_ohash);
    }
}
