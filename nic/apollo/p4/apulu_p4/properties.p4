/******************************************************************************/
/* LIF info                                                                   */
/******************************************************************************/
action lif_info(direction, lif_type, vnic_id, bd_id, vpc_id,
                pinned_nexthop_type, pinned_nexthop_id) {
    modify_field(control_metadata.rx_packet, lif_type);
    modify_field(p4i_i2e.rx_packet, direction);
    modify_field(control_metadata.lif_type, lif_type);
    modify_field(vnic_metadata.vnic_id, vnic_id);
    modify_field(vnic_metadata.bd_id, bd_id);
    modify_field(vnic_metadata.vpc_id, vpc_id);
    modify_field(p4i_i2e.nexthop_type, pinned_nexthop_type);
    modify_field(p4i_i2e.nexthop_id, pinned_nexthop_id);
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

@pragma stage 0
table lif2 {
    reads {
        arm_to_p4i.lif  : exact;
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

    // keys for local mapping lookup
    if (control_metadata.rx_packet == FALSE) {
        if (ipv4_1.valid == TRUE) {
            modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_IPV4);
            modify_field(key_metadata.local_mapping_lkp_addr, ipv4_1.srcAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.vpc_id);
        } else {
            modify_field(key_metadata.local_mapping_lkp_type,
                         KEY_TYPE_MAC);
            modify_field(key_metadata.local_mapping_lkp_addr,
                         ethernet_1.srcAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.bd_id);
        }
    } else {
        if (ipv4_1.valid == TRUE) {
            modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_IPV4);
            modify_field(key_metadata.local_mapping_lkp_addr, ipv4_1.dstAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.vpc_id);
        } else {
            modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_MAC);
            modify_field(key_metadata.local_mapping_lkp_addr,
                         ethernet_1.dstAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.bd_id);
        }
    }

    // keys for mapping lookup
    if ((control_metadata.l2_enabled == FALSE) or
        ((ipv4_1.valid == TRUE) and
        (scratch_metadata.mac == ethernet_1.dstAddr))) {
        modify_field(p4i_i2e.mapping_lkp_type, KEY_TYPE_IPV4);
        modify_field(p4i_i2e.mapping_lkp_addr, ipv4_1.dstAddr);
        modify_field(p4i_i2e.mapping_lkp_id, vnic_metadata.vpc_id);
    } else {
        modify_field(p4i_i2e.mapping_lkp_type, KEY_TYPE_MAC);
        modify_field(p4i_i2e.mapping_lkp_type, ethernet_1.dstAddr);
        modify_field(p4i_i2e.mapping_lkp_id, vnic_metadata.bd_id);
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
    // on miss, drop the packet

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

    // keys for local mapping lookup
    if (ipv4_2.valid == TRUE) {
        modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_IPV4);
        modify_field(key_metadata.local_mapping_lkp_type, ipv4_2.dstAddr);
        modify_field(key_metadata.local_mapping_lkp_id, vnic_metadata.vpc_id);
    } else {
        modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_MAC);
        modify_field(key_metadata.local_mapping_lkp_type, ethernet_2.dstAddr);
        modify_field(key_metadata.local_mapping_lkp_id, vnic_metadata.bd_id);
    }

    // keys for mapping lookup
    if ((control_metadata.l2_enabled == FALSE) or
        ((ipv4_2.valid == TRUE) and
        (scratch_metadata.mac == ethernet_2.dstAddr))) {
        modify_field(p4i_i2e.mapping_lkp_type, KEY_TYPE_IPV4);
        modify_field(p4i_i2e.mapping_lkp_addr, ipv4_2.dstAddr);
        modify_field(p4i_i2e.mapping_lkp_id, vnic_metadata.vpc_id);
    } else {
        modify_field(p4i_i2e.mapping_lkp_type, KEY_TYPE_MAC);
        modify_field(p4i_i2e.mapping_lkp_type, ethernet_2.dstAddr);
        modify_field(p4i_i2e.mapping_lkp_id, vnic_metadata.bd_id);
    }
}

@pragma stage 1
table vni {
    reads {
        vxlan_1.vni : exact;
    }
    actions {
        vni_info;
    }
    size : VNI_HASH_TABLE_SIZE;
}

@pragma stage 1
@pragma overflow_table vni
table vni_otcam {
    reads {
        vxlan_1.vni : ternary;
    }
    actions {
        vni_info;
    }
    size : VNI_OTCAM_TABLE_SIZE;
}

control input_properties {
    if (arm_to_p4i.valid == TRUE) {
        apply(lif2);
    } else {
        apply(lif);
    }
    if ((control_metadata.rx_packet == TRUE) and
        (control_metadata.to_device_ip == TRUE) and
        (vxlan_1.valid == TRUE)) {
        apply(key_tunneled);
        apply(vni);
        apply(vni_otcam);
        apply(service_mapping);
        apply(service_mapping_otcam);
    } else {
        apply(vlan);
    }
}

/******************************************************************************/
/* VPC info                                                                   */
/******************************************************************************/
action vpc_info(vni, vrmac) {
    modify_field(rewrite_metadata.vni, vni);
    modify_field(rewrite_metadata.vrmac, vrmac);
}

@pragma stage 2
@pragma index_table
table vpc {
    reads {
        p4e_i2e.vpc_id  : exact;
    }
    actions {
        vpc_info;
    }
    size : VPC_TABLE_SIZE;
}

/******************************************************************************/
/* BD info                                                                    */
/******************************************************************************/
action bd_info(vni, vrmac) {
    if (vnic_metadata.egress_bd_id == 0) {
        // return;
    }

    modify_field(rewrite_metadata.vni, vni);
    modify_field(rewrite_metadata.vrmac, vrmac);
}

@pragma stage 3
@pragma index_table
table bd {
    reads {
        vnic_metadata.egress_bd_id  : exact;
    }
    actions {
        bd_info;
    }
    size : BD_TABLE_SIZE;
}

control output_properties {
    apply(vpc);
    apply(bd);
}
