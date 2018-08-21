/******************************************************************************/
/* VNIC mapping table (by VLAN)                                               */
/******************************************************************************/
action local_vnic_info_common(local_vnic_tag, vcn_id, skip_src_dst_check,
                              resource_group_1, resource_group_2,
                              lpm_addr_1, lpm_addr_2,
                              slacl_addr_1, slacl_addr_2, epoch1, epoch2) {
    modify_field(scratch_metadata.local_vnic_tag, local_vnic_tag);
    modify_field(vnic_metadata.local_vnic_tag, scratch_metadata.local_vnic_tag);
    modify_field(scratch_metadata.vcn_id, vcn_id);
    modify_field(vnic_metadata.vcn_id, scratch_metadata.vcn_id);
    modify_field(vnic_metadata.skip_src_dst_check, skip_src_dst_check);
    if (service_header.valid == TRUE) {
        if (service_header.epoch == epoch1) {
            modify_field(scratch_metadata.use_epoch1, TRUE);
        } else {
            modify_field(scratch_metadata.use_epoch1, FALSE);
        }
    } else {
        if (epoch1 > epoch2) {
            modify_field(scratch_metadata.use_epoch1, TRUE);
        } else {
            modify_field(scratch_metadata.use_epoch1, FALSE);
        }
    }
    if (scratch_metadata.use_epoch1 == TRUE) {
        modify_field(policer_metadata.resource_group, resource_group_1);
        lpm_init(lpm_addr_1);
        slacl_init(slacl_addr_1);
        modify_field(service_header.epoch, epoch1);
    } else {
        modify_field(policer_metadata.resource_group, resource_group_2);
        lpm_init(lpm_addr_2);
        slacl_init(slacl_addr_2);
        modify_field(service_header.epoch, epoch2);
    }
}

action local_vnic_info_tx(local_vnic_tag, vcn_id, skip_src_dst_check,
                          resource_group_1, resource_group_2,
                          lpm_addr_1, lpm_addr_2, slacl_addr_1, slacl_addr_2,
                          epoch1, epoch2, overlay_mac, src_slot_id) {
    modify_field(apollo_i2e_metadata.src_slot_id, src_slot_id);

    // validate source mac
    if (ethernet_1.srcAddr == 0) {
        modify_field(control_metadata.p4i_drop_reason, 1 << DROP_SRC_MAC_ZERO);
        drop_packet();
    }
    if (ethernet_1.srcAddr != overlay_mac) {
        modify_field(control_metadata.p4i_drop_reason, 1 << DROP_SRC_MAC_MISMATCH);
        drop_packet();
    }

    local_vnic_info_common(local_vnic_tag, vcn_id, skip_src_dst_check,
                           resource_group_1, resource_group_2,
                           lpm_addr_1, lpm_addr_2, slacl_addr_1, slacl_addr_2,
                           epoch1, epoch2);

    // scratch metadata
    modify_field(scratch_metadata.overlay_mac, overlay_mac);
}

action local_vnic_info_rx(local_vnic_tag, vcn_id, skip_src_dst_check,
                          resource_group_1, resource_group_2,
                          slacl_addr_1, slacl_addr_2,
                          epoch1, epoch2, src_slot_id) {
    modify_field(vnic_metadata.src_slot_id, src_slot_id);
    // Validations
    // mytep_ip is a table constant
    if (ipv4_1.dstAddr != scratch_metadata.mytep_ip) {
        drop_packet();
    }

    local_vnic_info_common(local_vnic_tag, vcn_id, skip_src_dst_check,
            resource_group_1, resource_group_2,
            0, 0, slacl_addr_1, slacl_addr_2,
            epoch1, epoch2);
}

@pragma stage 0
table local_vnic_by_vlan_tx {
    reads {
        ctag_1.vid : exact;
    }
    actions {
        local_vnic_info_tx;
    }
    size : LOCAL_VNIC_BY_VLAN_TABLE_SIZE;
}

/******************************************************************************/
/* VNIC mapping table (by slot)                                               */
/******************************************************************************/
/* miss action : drop packet */
@pragma stage 0
table local_vnic_by_slot_rx {
    reads {
        mpls[0].label : exact;
    }
    actions {
        local_vnic_info_rx;
    }
    size : LOCAL_VNIC_BY_SLOT_TABLE_SIZE;
}

@pragma stage 0
@pragma overflow_table local_vnic_by_slot_rx
table local_vnic_by_slot_rx_otcam {
    reads {
        mpls[0].label : ternary;
    }
    actions {
        local_vnic_info_rx;
    }
    size : LOCAL_VNIC_BY_SLOT_OTCAM_TABLE_SIZE;
}

control ingress_vnic_info {
    if (control_metadata.direction == TX_FROM_HOST) {
        apply(local_vnic_by_vlan_tx);
    } else {
        apply(local_vnic_by_slot_rx);
        apply(local_vnic_by_slot_rx_otcam);
    }
}

action egress_local_vnic_info_rx(vr_mac, overlay_mac, overlay_vlan_id, subnet_id) {
    // Remove headers
    remove_egress_headers();
    remove_header(ethernet_1);
    remove_header(ipv4_1);
    remove_header(ipv6_1);
    remove_header(gre_1);
    remove_header(mpls[0]);
    remove_header(mpls[1]);

    // Add header towards host
    add_header(ethernet_0);
    add_header(ctag_0);

    modify_field(ethernet_0.dstAddr, overlay_mac);
    if (subnet_id == apollo_i2e_metadata.rvpath_subnet_id) {
        if (apollo_i2e_metadata.rvpath_overlay_mac != 0) {
            modify_field(ethernet_0.srcAddr, apollo_i2e_metadata.rvpath_overlay_mac);
        } else {
            modify_field(ethernet_0.srcAddr, vr_mac);
        }
    } else {
        // TODO:
    }
    modify_field(ethernet_0.etherType, ETHERTYPE_CTAG);
    modify_field(ctag_0.vid, overlay_vlan_id);
    if (ipv4_2.valid == TRUE) {
        modify_field(ctag_0.etherType, ETHERTYPE_IPV4);
    } else {
        if (ipv6_2.valid == TRUE) {
            modify_field(ctag_0.etherType, ETHERTYPE_IPV6);
        }
        // TODO: what happens to non-ip packets ?
    }

    // scratch metadata
    modify_field(scratch_metadata.subnet_id, subnet_id);
    modify_field(scratch_metadata.subnet_id, apollo_i2e_metadata.rvpath_subnet_id);
    modify_field(scratch_metadata.flag, ipv4_2.valid);
    modify_field(scratch_metadata.flag, ipv6_2.valid);
}

@pragma stage 0
@pragma index_table
table egress_local_vnic_info_rx {
    reads {
        apollo_i2e_metadata.local_vnic_tag : exact;
    }
    actions {
        egress_local_vnic_info_rx;
    }
    size : EGRESS_LOCAL_VNIC_INFO_TABLE_SIZE;
}

control egress_vnic_info {
    if (control_metadata.direction == RX_FROM_SWITCH) {
        apply(egress_local_vnic_info_rx);
    }
}
