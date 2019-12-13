/*****************************************************************************/
/* RVPATH check                                                              */
/*****************************************************************************/
@pragma capi appdatafields vpc_id subnet_id overlay_mac
@pragma capi hwfields_access_api
action remote_vnic_mapping_rx_info(entry_valid, vpc_id, subnet_id, overlay_mac,
                                   hash1, hint1, hash2, hint2, hash3,
                                   hint3, hash4, hint4, hash5, hint5,
                                   hash6, hint6, hash7, hint7,
                                   hash8, hint8, hash9, hint9,
                                   hash10, hint10, more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicate hit, take the results
        modify_field(service_header.remote_vnic_mapping_rx_done, TRUE);
        modify_field(scratch_metadata.vpc_id, vpc_id);
        if (vnic_metadata.vpc_id != scratch_metadata.vpc_id) {
            modify_field(control_metadata.p4i_drop_reason,
                         1 << P4I_DROP_RVPATH_VPC_MISMATCH);
            drop_packet();
        } else {
            modify_field(p4i_apollo_i2e.rvpath_subnet_id, subnet_id);
            modify_field(p4i_apollo_i2e.rvpath_overlay_mac, overlay_mac);
        }

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
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash9)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint9);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.vnic_mapping_hash == hash10)) {
            modify_field(scratch_metadata.vnic_mapping_hint, hint10);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.vnic_mapping_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(service_header.remote_vnic_mapping_rx_ohash,
                         scratch_metadata.vnic_mapping_hint);
            modify_field(control_metadata.remote_vnic_mapping_rx_ohash_lkp, TRUE);
        } else {
            modify_field(control_metadata.p4i_drop_reason,
                         1 << P4I_DROP_RVPATH_SRC_IP_MISMATCH);
            drop_packet();
        }
    } else {
        modify_field(control_metadata.p4i_drop_reason,
                     1 << P4I_DROP_RVPATH_SRC_IP_MISMATCH);
        drop_packet();
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
    modify_field(scratch_metadata.vnic_mapping_hash, hash9);
    modify_field(scratch_metadata.vnic_mapping_hash, hash10);
}

@pragma stage 1
@pragma hbm_table
table remote_vnic_mapping_rx {
    reads {
        ipv4_1.srcAddr              : exact;
        vnic_metadata.src_slot_id   : exact;
    }
    actions {
        remote_vnic_mapping_rx_info;
    }
    size : REMOTE_VNIC_MAPPINGS_RX_TABLE_SIZE;
}

@pragma stage 2
@pragma hbm_table
@pragma overflow_table remote_vnic_mapping_rx
table remote_vnic_mapping_rx_ohash {
    reads {
        service_header.remote_vnic_mapping_rx_ohash : exact;
    }
    actions {
        remote_vnic_mapping_rx_info;
    }
    size : REMOTE_VNIC_MAPPINGS_RX_OHASH_TABLE_SIZE;
}

control rvpath_check {
    if (mpls_src.valid == TRUE) {
        if (service_header.valid == FALSE) {
            apply(remote_vnic_mapping_rx);
        }
        if (control_metadata.remote_vnic_mapping_rx_ohash_lkp == TRUE) {
            apply(remote_vnic_mapping_rx_ohash);
        }
    }
}
