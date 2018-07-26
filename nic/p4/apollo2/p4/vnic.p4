/******************************************************************************/
/* VNIC mapping table (by VLAN)                                               */
/******************************************************************************/
action local_vnic_info_common(local_vnic_tag, vcn_id, subnet_id, skip_src_dst_check,
                              resource_group_1,
                              resource_group_2,
                              lpm_addr_1, lpm_addr_2, slacl_addr_1, slacl_addr_2,
                              epoch1, epoch2, src_slot) {
    modify_field(vnic_metadata.local_vnic_tag, local_vnic_tag);
    modify_field(vnic_metadata.subnet_id, subnet_id);
    modify_field(vnic_metadata.skip_src_dst_check, skip_src_dst_check);
    modify_field(vnic_metadata.vcn_id, vcn_id);
    modify_field(vnic_metadata.src_slot, src_slot);
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

action local_vnic_info_tx(local_vnic_tag, vcn_id, subnet_id, skip_src_dst_check,
                       resource_group_1,
                       resource_group_2,
                       lpm_addr_1, lpm_addr_2, slacl_addr_1, slacl_addr_2,
                       epoch1, epoch2,
                       overlay_mac, src_slot) {
    local_vnic_info_common(local_vnic_tag, vcn_id, subnet_id, skip_src_dst_check,
            resource_group_1, resource_group_2,
            lpm_addr_1, lpm_addr_2, slacl_addr_1, slacl_addr_2,
            epoch1, epoch2, src_slot);

    // Validations
    if (ethernet_1.srcAddr != overlay_mac) {
        drop_packet();
    }
    modify_field(scratch_metadata.macsa, overlay_mac);
}

action local_vnic_info_rx(local_vnic_tag, vcn_id, subnet_id, skip_src_dst_check,
                       resource_group_1,
                       resource_group_2,
                       slacl_addr_1, slacl_addr_2,
                       epoch1, epoch2,
                       src_slot) {
    local_vnic_info_common(local_vnic_tag, vcn_id, subnet_id, skip_src_dst_check,
            resource_group_1, resource_group_2,
            0, 0, slacl_addr_1, slacl_addr_2,
            epoch1, epoch2, src_slot);

    if (mpls[1].valid == TRUE) {
        modify_field(rvpath_metadata.src_slot_id, mpls[1].label);
    }
    // Validations
    if (ipv4_1.dstAddr != scratch_metadata.mytep_ip) {
        drop_packet();
    }
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

action egress_local_vnic_info_rx(vr_mac, overlay_mac, vlan_id) {
    modify_field(vnic_metadata.vr_mac, vr_mac);
    modify_field(vnic_metadata.overlay_mac, overlay_mac);
    modify_field(vnic_metadata.vlan_id, vlan_id);
}

table egress_local_vnic_info_rx {
    reads {
        capri_i2e_metadata.local_vnic_tag : exact;
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
