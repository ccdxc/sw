/******************************************************************************/
/* LIF info                                                                   */
/******************************************************************************/
action lif_info(lif_type, vnic_id, bd_id, vpc_id,
                pinned_lif, pinned_qtype, pinned_qid) {
    modify_field(control_metadata.rx_packet, lif_type);
    modify_field(vnic_metadata.vnic_id, vnic_id);
    modify_field(vnic_metadata.bd_id, bd_id);
    modify_field(vnic_metadata.vpc_id, vpc_id);
    modify_field(control_metadata.pinned_lif, pinned_lif);
    modify_field(control_metadata.pinned_qtype, pinned_qtype);
    modify_field(control_metadata.pinned_qid, pinned_qid);
}

@pragma stage 0
table lif {
    reads {
        capri_intrinsic.lif : exact;
    }
    actions {
        lif_info;
    }
    size : LIF_TABLE_SIZE;
}

/******************************************************************************/
/* VLAN info                                                                  */
/******************************************************************************/
action vlan_info(vnic_id, bd_id, vpc_id, rmac) {
    if (vnic_id != 0) {
        modify_field(vnic_metadata.vnic_id, vnic_id);
    }
    if (bd_id != 0) {
        modify_field(vnic_metadata.bd_id, bd_id);
    }
    if (vpc_id != 0) {
        modify_field(vnic_metadata.vpc_id, vpc_id);
    }
    modify_field(scratch_metadata.mac, rmac);

    if (control_metadata.rx_packet == FALSE) {
        if (ipv4_1.valid == TRUE) {
            modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_IPV4);
            modify_field(key_metadata.local_mapping_lkp_type, ipv4_1.srcAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.vpc_id);
        } else {
            modify_field(key_metadata.local_mapping_lkp_type,
                         KEY_TYPE_MAC);
            modify_field(key_metadata.local_mapping_lkp_type,
                         ethernet_1.srcAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.bd_id);
        }
    } else {
        if (ipv4_1.valid == TRUE) {
            modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_IPV4);
            modify_field(key_metadata.local_mapping_lkp_type, ipv4_1.dstAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.vpc_id);
        } else {
            modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_MAC);
            modify_field(key_metadata.local_mapping_lkp_type,
                         ethernet_1.dstAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.bd_id);
        }
    }
}

@pragma stage 1
table vlan {
    reads {
        ctag_1.vid  : exact;
    }
    actions {
        vlan_info;
    }
    size : VLAN_TABLE_SIZE;
}

/******************************************************************************/
/* VNI info                                                                   */
/******************************************************************************/
action vni_info(vnic_id, bd_id, vpc_id, rmac) {
    if (vnic_id != 0) {
        modify_field(vnic_metadata.vnic_id, vnic_id);
    }
    if (bd_id != 0) {
        modify_field(vnic_metadata.bd_id, bd_id);
    }
    if (vpc_id != 0) {
        modify_field(vnic_metadata.vpc_id, vpc_id);
    }
    modify_field(scratch_metadata.mac, rmac);
    modify_field(control_metadata.tunneled_packet, TRUE);

    if (ipv4_2.valid == TRUE) {
        modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_IPV4);
        modify_field(key_metadata.local_mapping_lkp_type, ipv4_2.dstAddr);
        modify_field(key_metadata.local_mapping_lkp_id, vnic_metadata.vpc_id);
    } else {
        modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_MAC);
        modify_field(key_metadata.local_mapping_lkp_type, ethernet_2.dstAddr);
        modify_field(key_metadata.local_mapping_lkp_id, vnic_metadata.bd_id);
    }
}

@pragma stage 1
table vni {
    reads {
        vxlan_1.vni : exact;
    }
    actions {
        vlan_info;
    }
    size : VNI_HASH_TABLE_SIZE;
}

@pragma stage 1
@pragma overflow_table tunnel_rx
table vni_otcam {
    reads {
        vxlan_1.vni : ternary;
    }
    actions {
        vlan_info;
    }
    size : VNI_OTCAM_TABLE_SIZE;
}

control input_properties {
    apply(lif);
    if ((control_metadata.rx_packet == TRUE) and (vxlan_1.valid == TRUE)) {
        apply(vni);
    } else {
        apply(vlan);
    }
}
