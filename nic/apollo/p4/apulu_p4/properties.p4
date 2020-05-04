/******************************************************************************/
/* LIF info                                                                   */
/******************************************************************************/
action lif_info(direction, lif_type, vnic_id, bd_id, vpc_id, vrmac,
                learn_enabled, pinned_nexthop_type, pinned_nexthop_id) {
    modify_field(control_metadata.rx_packet, direction);
    modify_field(p4i_i2e.rx_packet, direction);
    modify_field(control_metadata.lif_type, lif_type);
    modify_field(vnic_metadata.vnic_id, vnic_id);
    modify_field(vnic_metadata.bd_id, bd_id);
    modify_field(vnic_metadata.vpc_id, vpc_id);
    if (arm_to_p4i.flow_lkp_id_override == FALSE) {
        modify_field(key_metadata.flow_lkp_id, bd_id);
    } else {
        modify_field(key_metadata.flow_lkp_id, arm_to_p4i.flow_lkp_id);
    }
    modify_field(p4i_i2e.skip_stats_update, arm_to_p4i.skip_stats_update);
    modify_field(p4i_to_arm.sw_meta, arm_to_p4i.sw_meta);
    modify_field(p4i_i2e.nexthop_type, pinned_nexthop_type);
    modify_field(p4i_i2e.nexthop_id, pinned_nexthop_id);
    modify_field(control_metadata.learn_enabled, learn_enabled);
    modify_field(vnic_metadata.vrmac, vrmac);
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
        modify_field(scratch_metadata.mac, rmac);
        modify_field(vnic_metadata.bd_id, bd_id);
        if (arm_to_p4i.flow_lkp_id_override == FALSE) {
            modify_field(key_metadata.flow_lkp_id, bd_id);
        }
    } else {
        modify_field(scratch_metadata.mac, vnic_metadata.vrmac);
    }
    if (vpc_id != 0) {
        modify_field(vnic_metadata.vpc_id, vpc_id);
    }

    // keys for local mapping lookup
    if (control_metadata.rx_packet == FALSE) {
        if (ipv4_1.valid == TRUE) {
            modify_field(key_metadata.local_mapping_lkp_type, KEY_TYPE_IPV4);
            modify_field(key_metadata.local_mapping_lkp_addr, ipv4_1.srcAddr);
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.vpc_id);
        } else {
            modify_field(key_metadata.local_mapping_lkp_id,
                         vnic_metadata.bd_id);
            if (arp.valid == TRUE) {
                modify_field(key_metadata.local_mapping_lkp_type,
                             KEY_TYPE_IPV4);
                modify_field(key_metadata.local_mapping_lkp_addr,
                             arp.senderIpAddr);
            } else {
                modify_field(key_metadata.local_mapping_lkp_type,
                             KEY_TYPE_MAC);
                modify_field(key_metadata.local_mapping_lkp_addr,
                             ethernet_1.srcAddr);
            }
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
action vni_info(vnic_id, bd_id, vpc_id, rmac, is_l3_vnid) {
    // on miss, drop the packet

    if (vnic_id != 0) {
        modify_field(vnic_metadata.vnic_id, vnic_id);
    }
    if (bd_id != 0) {
        modify_field(vnic_metadata.bd_id, bd_id);
        if (arm_to_p4i.flow_lkp_id_override == FALSE) {
            modify_field(key_metadata.flow_lkp_id, bd_id);
        }
    }
    if (vpc_id != 0) {
        modify_field(vnic_metadata.vpc_id, vpc_id);
    }
    modify_field(scratch_metadata.mac, rmac);
    modify_field(control_metadata.tunneled_packet, TRUE);
    modify_field(p4i_to_arm.is_l3_vnid, is_l3_vnid);

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

/******************************************************************************/
/* VNIC info (Ingress)                                                        */
/******************************************************************************/
action vnic_info(epoch, meter_enabled, rx_mirror_session, tx_mirror_session,
                 tx_policer_id) {
    modify_field(p4i_to_arm.epoch, epoch);
    if ((control_metadata.flow_done == TRUE) and
        (control_metadata.flow_miss == FALSE) and
        (control_metadata.flow_epoch != epoch)) {
        modify_field(control_metadata.flow_done, FALSE);
        modify_field(ingress_recirc.defunct_flow, TRUE);
        // return;
    }
    modify_field(p4i_i2e.meter_enabled, meter_enabled);
    if (control_metadata.rx_packet == TRUE) {
        modify_field(p4i_i2e.mirror_session, rx_mirror_session);
    } else {
        modify_field(p4i_i2e.tx_policer_id, tx_policer_id);
        modify_field(p4i_i2e.mirror_session, tx_mirror_session);
    }
}

@pragma stage 4
@pragma index_table
table vnic {
    reads {
        vnic_metadata.vnic_id   : exact;
    }
    actions {
        vnic_info;
    }
    size : VNIC_TABLE_SIZE;
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
    apply(vnic);
}

/******************************************************************************/
/* VPC info                                                                   */
/******************************************************************************/
action vpc_info(vni, vrmac, tos) {
    modify_field(rewrite_metadata.vni, vni);
    modify_field(rewrite_metadata.vrmac, vrmac);
    modify_field(rewrite_metadata.tunnel_tos, tos);

    // TTL decrement
    if (((control_metadata.rx_packet == FALSE) and
         (TX_REWRITE(rewrite_metadata.flags, TTL, DEC))) or
        ((control_metadata.rx_packet == TRUE) and
         (RX_REWRITE(rewrite_metadata.flags, TTL, DEC)))) {
        if (ipv4_1.valid == TRUE) {
            add(ipv4_1.ttl, ipv4_1.ttl, -1);
            modify_field(control_metadata.update_checksum, TRUE);
        } else {
            if (ipv6_1.valid == TRUE) {
                add(ipv6_1.hopLimit, ipv6_1.hopLimit, -1);
            }
        }
    }
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
action bd_info(vni, vrmac, tos) {
    if (vnic_metadata.egress_bd_id == 0) {
        // return;
    }

    modify_field(rewrite_metadata.vni, vni);
    modify_field(rewrite_metadata.vrmac, vrmac);
    modify_field(rewrite_metadata.tunnel_tos, tos);
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

/******************************************************************************/
/* Rx VNIC info (Egress)                                                      */
/******************************************************************************/
action rx_vnic_info(rx_policer_id) {
    modify_field(vnic_metadata.rx_policer_id, rx_policer_id);
}

@pragma stage 3
@pragma index_table
table rx_vnic {
    reads {
        vnic_metadata.rx_vnic_id    : exact;
    }
    actions {
        rx_vnic_info;
    }
    size : VNIC_TABLE_SIZE;
}

control output_properties {
    apply(vpc);
    apply(bd);
    apply(rx_vnic);
}
