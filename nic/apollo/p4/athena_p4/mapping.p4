/******************************************************************************/
/* Local IP mapping                                                           */
/******************************************************************************/
@pragma capi appdatafields vpc_id vpc_id_valid xlate_index ip_type local_vnic_tag
@pragma capi hwfields_access_api
action local_ip_mapping_info(entry_valid, local_vnic_tag,
                             vpc_id, vpc_id_valid, xlate_index, ip_type,
                             hash1, hint1, hash2, hint2, hash3, hint3,
                             hash4, hint4, hash5, hint5, hash6, hint6,
                             hash7, hint7, hash8, hint8, hash9, hint9,
                             hash10, hint10, more_hashes, more_hints) {

    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        if (vpc_id_valid == TRUE) {
            modify_field(vnic_metadata.vpc_id, vpc_id);
        }
        if (control_metadata.direction == RX_FROM_SWITCH) {
            if ((mpls_dst.valid == TRUE) and (ip_type == IP_TYPE_PUBLIC)) {
                modify_field(p4i_apollo_i2e.dnat_required, TRUE);
            }
        }
        if (local_vnic_tag != 0) {
            modify_field(vnic_metadata.local_vnic_tag, local_vnic_tag);
        }
        modify_field(service_header.local_mapping_done, TRUE);
        modify_field(p4i_apollo_i2e.xlate_index, xlate_index);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.local_ip_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.local_ip_hash,
                     scratch_metadata.local_ip_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash1)) {
            modify_field(scratch_metadata.local_ip_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash2)) {
            modify_field(scratch_metadata.local_ip_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash3)) {
            modify_field(scratch_metadata.local_ip_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash4)) {
            modify_field(scratch_metadata.local_ip_hint, hint4);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash5)) {
            modify_field(scratch_metadata.local_ip_hint, hint5);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash6)) {
            modify_field(scratch_metadata.local_ip_hint, hint6);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash7)) {
            modify_field(scratch_metadata.local_ip_hint, hint7);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash8)) {
            modify_field(scratch_metadata.local_ip_hint, hint8);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash9)) {
            modify_field(scratch_metadata.local_ip_hint, hint9);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash10)) {
            modify_field(scratch_metadata.local_ip_hint, hint10);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.local_ip_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(service_header.local_mapping_ohash,
                         scratch_metadata.local_ip_hint);
            modify_field(control_metadata.local_mapping_ohash_lkp, TRUE);
        } else {
            if (vnic_metadata.skip_src_dst_check == FALSE) {
                modify_field(control_metadata.p4i_drop_reason,
                             1 << P4I_DROP_SRC_DST_CHECK_FAIL);
                drop_packet();
            }
        }
    } else {
        if (vnic_metadata.skip_src_dst_check == FALSE) {
            modify_field(control_metadata.p4i_drop_reason,
                         1 << P4I_DROP_SRC_DST_CHECK_FAIL);
            drop_packet();
        }
    }

    modify_field(scratch_metadata.flag, ip_type);
    modify_field(scratch_metadata.flag, vpc_id_valid);
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
}

@pragma stage 1
@pragma hbm_table
table local_ip_mapping {
    reads {
        key_metadata.lkp_id                 : exact;
        control_metadata.mapping_lkp_addr   : exact;
    }
    actions {
        local_ip_mapping_info;
    }
    size : VNIC_IP_MAPPING_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table local_ip_mapping
table local_ip_mapping_ohash {
    reads {
        service_header.local_mapping_ohash  : exact;
    }
    actions {
        local_ip_mapping_info;
    }
    size : VNIC_IP_MAPPING_OHASH_TABLE_SIZE;
}

/******************************************************************************/
/* Local mapping (mac based)                                                  */
/******************************************************************************/
@pragma capi appdatafields vpc_id vpc_id_valid local_vnic_tag
@pragma capi hwfields_access_api
action local_l2_mapping_info(entry_valid, local_vnic_tag, vpc_id, vpc_id_valid,
                             pad, hash1, hint1, hash2, hint2, hash3, hint3,
                             hash4, hint4, more_hashes, more_hints) {

    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        if (vpc_id_valid == TRUE) {
            modify_field(vnic_metadata.vpc_id, vpc_id);
        }
        if (local_vnic_tag != 0) {
            modify_field(vnic_metadata.local_vnic_tag, local_vnic_tag);
        }
        modify_field(service_header.local_mapping_done, TRUE);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.local_ip_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.local_ip_hash,
                     scratch_metadata.local_ip_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash1)) {
            modify_field(scratch_metadata.local_ip_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash2)) {
            modify_field(scratch_metadata.local_ip_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash3)) {
            modify_field(scratch_metadata.local_ip_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.local_ip_hash == hash4)) {
            modify_field(scratch_metadata.local_ip_hint, hint4);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.local_ip_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

        if (scratch_metadata.hint_valid == TRUE) {
            modify_field(service_header.local_mapping_ohash,
                         scratch_metadata.local_ip_hint);
            modify_field(control_metadata.local_mapping_ohash_lkp, TRUE);
        } else {
            if (vnic_metadata.skip_src_dst_check == FALSE) {
                modify_field(control_metadata.p4i_drop_reason,
                             1 << P4I_DROP_SRC_DST_CHECK_FAIL);
                drop_packet();
            }
        }
    } else {
        if (vnic_metadata.skip_src_dst_check == FALSE) {
            modify_field(control_metadata.p4i_drop_reason,
                         1 << P4I_DROP_SRC_DST_CHECK_FAIL);
            drop_packet();
        }
    }

    modify_field(scratch_metadata.flag, vpc_id_valid);
    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.pad6, pad);
    modify_field(scratch_metadata.local_ip_hash, hash1);
    modify_field(scratch_metadata.local_ip_hash, hash2);
    modify_field(scratch_metadata.local_ip_hash, hash3);
    modify_field(scratch_metadata.local_ip_hash, hash4);
}

@pragma stage 1
@pragma hbm_table
table local_l2_mapping {
    reads {
        key_metadata.lkp_id                         : exact;
        control_metadata.local_l2_mapping_lkp_addr  : exact;
    }
    actions {
        local_l2_mapping_info;
    }
    size : VNIC_IP_MAPPING_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table local_l2_mapping
table local_l2_mapping_ohash {
    reads {
        service_header.local_mapping_ohash  : exact;
    }
    actions {
        local_l2_mapping_info;
    }
    size : VNIC_IP_MAPPING_OHASH_TABLE_SIZE;
}

control local_mapping {
    if (service_header.valid == FALSE) {
        if (control_metadata.mode == APOLLO_MODE_DEFAULT) {
            apply(local_ip_mapping);
        } else {
            apply(local_l2_mapping);
        }
    }
    if (control_metadata.local_mapping_ohash_lkp == TRUE) {
        if (control_metadata.mode == APOLLO_MODE_DEFAULT) {
            apply(local_ip_mapping_ohash);
        } else {
            apply(local_l2_mapping_ohash);
        }
    }
}

/******************************************************************************/
/* IP mapping on TX direction                                                 */
/******************************************************************************/
@pragma capi appdatafields nexthop_group_index dst_slot_id_valid dst_slot_id
@pragma capi hwfields_access_api
action remote_vnic_mapping_tx_info(entry_valid, nexthop_group_index,
                                   dst_slot_id_valid, dst_slot_id,
                                   hash1, hint1, hash2, hint2, hash3, hint3,
                                   hash4, hint4, hash5, hint5, hash6, hint6,
                                   hash7, hint7, hash8, hint8,
                                   more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(egress_service_header.remote_vnic_mapping_tx_done, TRUE);
        modify_field(txdma_to_p4e_header.nexthop_group_index,
                     nexthop_group_index);
        modify_field(rewrite_metadata.dst_slot_id_valid, dst_slot_id_valid);
        modify_field(rewrite_metadata.dst_slot_id, dst_slot_id);

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
            modify_field(egress_service_header.remote_vnic_mapping_tx_ohash,
                         scratch_metadata.vnic_mapping_hint);
            modify_field(control_metadata.remote_vnic_mapping_tx_ohash_lkp, TRUE);
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
table remote_vnic_mapping_tx {
    reads {
        txdma_to_p4e_header.vpc_id  : exact;
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
        egress_service_header.remote_vnic_mapping_tx_ohash : exact;
    }
    actions {
        remote_vnic_mapping_tx_info;
    }
    size : REMOTE_VNIC_MAPPING_TX_OHASH_TABLE_SIZE;
}

control remote_vnic_mapping_tx {
    if (control_metadata.direction == TX_FROM_HOST) {
        if (egress_service_header.valid == FALSE) {
            apply(remote_vnic_mapping_tx);
        }
        if (control_metadata.remote_vnic_mapping_tx_ohash_lkp == TRUE) {
            apply(remote_vnic_mapping_tx_ohash);
        }
    }
}
