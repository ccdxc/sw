/******************************************************************************/
/*                                                                            */
/******************************************************************************/
action local_ip_mapping_info(entry_valid, vcn_id, vcn_id_valid, xlate_index,
                             ip_type,
                             hash1, hint1, hash2, hint2,
                             hash3, hint3, hash4, hint4, hash5, hint5,
                             hash6, hint6, hash7, hint7, hash8, hint8,
                             hash9, hint9, hash10, hint10,
                             more_hashes, more_hints) {

    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        if (vcn_id_valid == TRUE) {
            modify_field(vnic_metadata.vcn_id, vcn_id);
        }

        if (control_metadata.direction == RX_FROM_SWITCH) {
            if ((mpls[0].valid == TRUE) and (ip_type == IP_TYPE_PUBLIC)) {
                modify_field(p4i_apollo_i2e.dnat_required, TRUE);
            }
        }
        modify_field(service_header.local_ip_mapping_done, TRUE);

        modify_field(p4i_apollo_i2e.xlate_index, xlate_index);

        // if hardware register indicates miss, compare hints and setup
        // to perform lookup in overflow table
        modify_field(service_header.local_ip_mapping_ohash,
                     scratch_metadata.local_ip_hint);
        modify_field(control_metadata.local_ip_mapping_ohash_lkp, TRUE);
    } else {
        if (vnic_metadata.skip_src_dst_check == FALSE) {
            drop_packet();
        }
    }

    modify_field(scratch_metadata.flag, ip_type);
    modify_field(scratch_metadata.flag, vcn_id_valid);
    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.local_ip_hash, hash1);
    modify_field(scratch_metadata.local_ip_hash, hash2);
    modify_field(scratch_metadata.local_ip_hash, hash3);
    modify_field(scratch_metadata.local_ip_hash, hash4);
    modify_field(scratch_metadata.local_ip_hash, hash5);
    modify_field(scratch_metadata.local_ip_hash, hash6);
    modify_field(scratch_metadata.local_ip_hash, hash7);
    modify_field(scratch_metadata.local_ip_hash, hash8);
    modify_field(scratch_metadata.local_ip_hash, hash9);
    modify_field(scratch_metadata.local_ip_hash, hash10);
    modify_field(scratch_metadata.flag, more_hashes);
    modify_field(scratch_metadata.local_ip_hint, hint1);
    modify_field(scratch_metadata.local_ip_hint, hint2);
    modify_field(scratch_metadata.local_ip_hint, hint3);
    modify_field(scratch_metadata.local_ip_hint, hint4);
    modify_field(scratch_metadata.local_ip_hint, hint5);
    modify_field(scratch_metadata.local_ip_hint, hint6);
    modify_field(scratch_metadata.local_ip_hint, hint7);
    modify_field(scratch_metadata.local_ip_hint, hint8);
    modify_field(scratch_metadata.local_ip_hint, hint9);
    modify_field(scratch_metadata.local_ip_hint, hint10);
    modify_field(scratch_metadata.local_ip_hint, more_hints);
}

@pragma stage 1
@pragma hbm_table
table local_ip_mapping {
    reads {
        vnic_metadata.local_vnic_tag        : exact;
        control_metadata.mapping_lkp_addr   : exact;
    }
    actions {
        local_ip_mapping_info;
    }
    size : VNIC_IP_MAPPING_TABLE_SIZE;
}

@pragma stage 1
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

action remote_vnic_mapping_tx_info(entry_valid, nexthop_index,
                                   hash1, hint1, hash2, hint2,
                                   hash3, hint3, hash4, hint4, hash5, hint5,
                                   hash6, hint6, hash7, hint7, hash8, hint8,
                                   more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(egress_service_header.remote_vnic_mapping_done, TRUE);

        modify_field(txdma_to_p4e_header.nexthop_index, nexthop_index);

        // if hardware register indicates miss, compare hints and setup
        // to perform lookup in overflow table
        modify_field(egress_service_header.remote_vnic_mapping_ohash,
                     scratch_metadata.remote_vnic_hint);
        modify_field(control_metadata.remote_vnic_mapping_ohash_lkp, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.remote_vnic_hash, hash1);
    modify_field(scratch_metadata.remote_vnic_hash, hash2);
    modify_field(scratch_metadata.remote_vnic_hash, hash3);
    modify_field(scratch_metadata.remote_vnic_hash, hash4);
    modify_field(scratch_metadata.remote_vnic_hash, hash5);
    modify_field(scratch_metadata.remote_vnic_hash, hash6);
    modify_field(scratch_metadata.remote_vnic_hash, hash7);
    modify_field(scratch_metadata.remote_vnic_hash, hash8);
    modify_field(scratch_metadata.flag, more_hints);
    modify_field(scratch_metadata.remote_vnic_hint, hint1);
    modify_field(scratch_metadata.remote_vnic_hint, hint2);
    modify_field(scratch_metadata.remote_vnic_hint, hint3);
    modify_field(scratch_metadata.remote_vnic_hint, hint4);
    modify_field(scratch_metadata.remote_vnic_hint, hint5);
    modify_field(scratch_metadata.remote_vnic_hint, hint6);
    modify_field(scratch_metadata.remote_vnic_hint, hint7);
    modify_field(scratch_metadata.remote_vnic_hint, hint8);
    modify_field(scratch_metadata.remote_vnic_hint, more_hashes);
}

@pragma stage 0
@pragma hbm_table
table remote_vnic_mapping_tx {
    reads {
        txdma_to_p4e_header.vcn_id  : exact;
        p4e_apollo_i2e.dst          : exact;
    }
    actions {
        remote_vnic_mapping_tx_info;
    }
    size : REMOTE_VNIC_MAPPING_TX_TABLE_SIZE;
}

@pragma stage 1
@pragma hbm_table
@pragma overflow_table remote_vnic_mapping_tx
table remote_vnic_mapping_tx_ohash {
    reads {
        egress_service_header.remote_vnic_mapping_ohash : exact;
    }
    actions {
        remote_vnic_mapping_tx_info;
    }
    size : REMOTE_VNIC_MAPPING_TX_OHASH_TABLE_SIZE;
}

control remote_vnic_mapping {
    if (control_metadata.direction == TX_FROM_HOST) {
        if (egress_service_header.valid == FALSE) {
            apply(remote_vnic_mapping_tx);
        }
        if (control_metadata.remote_vnic_mapping_ohash_lkp == TRUE) {
            apply(remote_vnic_mapping_tx_ohash);
        }
    }
}
