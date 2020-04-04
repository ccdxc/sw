/******************************************************************************/
/* Local mapping                                                              */
/******************************************************************************/
@pragma capi appdatafields vnic_id xlate_id allow_tagged_pkts tag_idx binding_check_enabled binding_id1 binding_id2 ip_type
@pragma capi hwfields_access_api
action local_mapping_info(entry_valid, vnic_id,
                          hash1, hint1, hash2, hint2, hash3, hint3,
                          hash4, hint4, hash5, hint5, hash6, hint6,
                          hash7, hint7, hash8, hint8, more_hashes, more_hints,
                          xlate_id, allow_tagged_pkts, tag_idx,
                          binding_check_enabled, binding_id1, binding_id2,
                          ip_type) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        if (vnic_id != 0) {
            modify_field(vnic_metadata.vnic_id, vnic_id);
        }
        modify_field(p4i_i2e.xlate_id, xlate_id);
        modify_field(control_metadata.binding_check_enabled,
                     binding_check_enabled);
        modify_field(vnic_metadata.binding_id, binding_id1);
        modify_field(p4i_to_arm.local_mapping_ip_type, ip_type);
        modify_field(p4i_to_rxdma.local_tag_idx, tag_idx);
        modify_field(scratch_metadata.binding_id, binding_id2);
        modify_field(scratch_metadata.flag, allow_tagged_pkts);
        if ((control_metadata.rx_packet == FALSE) and
            (scratch_metadata.flag == FALSE) and
            (ctag_1.valid == TRUE)) {
            ingress_drop(P4I_DROP_TAGGED_PKT_FROM_VNIC);
        }
        modify_field(control_metadata.local_mapping_done, TRUE);

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
        } else {
            modify_field(control_metadata.local_mapping_miss, TRUE);
            modify_field(control_metadata.local_mapping_done, TRUE);
        }
    } else {
        if (arm_to_p4i.local_mapping_override == FALSE) {
            modify_field(control_metadata.local_mapping_miss, TRUE);
        }
        modify_field(control_metadata.local_mapping_done, TRUE);
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

/******************************************************************************/
/* IP MAC binding table                                                       */
/******************************************************************************/
action binding_info(addr) {
    modify_field(scratch_metadata.ipv6_addr, addr);
    if (key_metadata.local_mapping_lkp_type == KEY_TYPE_IPV4) {
        if (ethernet_1.srcAddr != addr) {
            ingress_drop(P4I_DROP_MAC_IP_BINDING_FAIL);
        }
    } else {
        if ((ipv6_1.valid == TRUE) and (ipv6_1.srcAddr != addr)) {
            ingress_drop(P4I_DROP_MAC_IP_BINDING_FAIL);
        }
    }
}

@pragma stage 4
@pragma hbm_table
@pragma index_table
@pragma capi_bitfields_struct
table ip_mac_binding {
    reads {
        vnic_metadata.binding_id    : exact;
    }
    actions {
        binding_info;
    }
    size : IP_MAC_BINDING_TABLE_SIZE;
}

/******************************************************************************/
/* Service mapping table                                                      */
/******************************************************************************/
action service_mapping_info(xlate_id) {
    // if hardware register indicates hit, take the results
    modify_field(p4i_to_arm.service_xlate_id, xlate_id);
}

@pragma stage 5
table service_mapping {
    reads {
        vnic_metadata.vpc_id    : exact;
        key_metadata.dst        : exact;
        key_metadata.dport      : exact;
    }
    actions {
        service_mapping_info;
    }
    size : SERVICE_MAPPING_TABLE_SIZE;
}

@pragma stage 5
@pragma overflow_table service_mapping
table service_mapping_otcam {
    reads {
        vnic_metadata.vpc_id    : ternary;
        key_metadata.dst        : ternary;
        key_metadata.dport      : ternary;
    }
    actions {
        service_mapping_info;
    }
    size : SERVICE_MAPPING_OTCAM_TABLE_SIZE;
}

control local_mapping {
    if (control_metadata.local_mapping_ohash_lkp == FALSE) {
        apply(local_mapping);
    }
    if (control_metadata.local_mapping_ohash_lkp == TRUE) {
        apply(local_mapping_ohash);
    }
    if ((control_metadata.rx_packet == FALSE) and
        (control_metadata.binding_check_enabled == TRUE)){
        apply(ip_mac_binding);
    }
}

/******************************************************************************/
/* All mappings (local and remote)                                            */
/******************************************************************************/
@pragma capi appdatafields is_local nexthop_valid nexthop_type nexthop_id egress_bd_id dmaci
@pragma capi hwfields_access_api
action mapping_info(entry_valid, is_local, pad11, nexthop_valid,
                    nexthop_type, nexthop_id, egress_bd_id, dmaci,
                    hash1, hint1, hash2, hint2, hash3, hint3,
                    hash4, hint4, hash5, hint5, hash6, hint6,
                    hash7, hint7, hash8, hint8, more_hashes, more_hints) {
    if (p4e_i2e.nexthop_type == NEXTHOP_TYPE_VPC) {
        modify_field(p4e_i2e.vpc_id, p4e_i2e.mapping_lkp_id);
    }
    if (p4e_i2e.mapping_bypass == TRUE) {
        modify_field(rewrite_metadata.nexthop_type, p4e_i2e.nexthop_type);
        modify_field(egress_recirc.mapping_done, TRUE);
        modify_field(control_metadata.mapping_done, TRUE);
        // return
    }

    if ((p4e_to_arm.valid == TRUE) and
        (txdma_to_p4e.nexthop_type == NEXTHOP_TYPE_VPC)) {
        modify_field(p4e_to_arm.nexthop_type, txdma_to_p4e.nexthop_type);
        modify_field(p4e_to_arm.nexthop_id, txdma_to_p4e.nexthop_id);
        modify_field(egress_recirc.mapping_done, TRUE);
        modify_field(control_metadata.mapping_done, TRUE);
        // return
    }

    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        modify_field(vnic_metadata.egress_bd_id, egress_bd_id);
        modify_field(rewrite_metadata.dmaci, dmaci);
        modify_field(scratch_metadata.flag, nexthop_valid);
        if (nexthop_valid == TRUE) {
            if (p4e_to_arm.valid == TRUE) {
                if (p4e_i2e.rx_packet == 0) {
                    modify_field(p4e_to_arm.nexthop_type, nexthop_type);
                    modify_field(p4e_to_arm.nexthop_id, nexthop_id);
                } else {
                    modify_field(p4e_to_arm.nexthop_type, txdma_to_p4e.nexthop_type);
                    modify_field(p4e_to_arm.nexthop_id, txdma_to_p4e.nexthop_id);
                }
                modify_field(p4e_to_arm.mapping_hit, TRUE);
                modify_field(p4e_to_arm.is_local, is_local);
            } else {
                modify_field(rewrite_metadata.nexthop_type, nexthop_type);
                modify_field(p4e_i2e.nexthop_id, nexthop_id);
            }
        }
        modify_field(egress_recirc.mapping_done, TRUE);
        modify_field(control_metadata.mapping_done, TRUE);
        // return

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
            modify_field(egress_recirc.mapping_ohash,
                         scratch_metadata.mapping_hint);
            modify_field(control_metadata.mapping_ohash_lkp, TRUE);
        } else {
            modify_field(egress_recirc.mapping_done, TRUE);
            modify_field(control_metadata.mapping_done, TRUE);
        }
    } else {
        if (p4e_to_arm.valid == TRUE) {
            modify_field(p4e_to_arm.nexthop_type, txdma_to_p4e.nexthop_type);
            modify_field(p4e_to_arm.nexthop_id, txdma_to_p4e.nexthop_id);
        }
        modify_field(egress_recirc.mapping_done, TRUE);
        modify_field(control_metadata.mapping_done, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.pad11, pad11);
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
        p4e_i2e.mapping_lkp_type    : exact;
        p4e_i2e.mapping_lkp_id      : exact;
        p4e_i2e.mapping_lkp_addr    : exact;
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
        egress_recirc.mapping_ohash     : exact;
    }
    actions {
        mapping_info;
    }
    size : MAPPING_OHASH_TABLE_SIZE;
}

control mapping {
    if (egress_recirc.valid == FALSE) {
        apply(mapping);
    }
    if (control_metadata.mapping_ohash_lkp == TRUE) {
        apply(mapping_ohash);
    }
}
