/******************************************************************************/
/* VNIC mapping table (by VLAN)                                               */
/******************************************************************************/
action local_vnic_info(vnic_id, vcn_id, subnet_id, skip_src_dst_check,
                       resource_group1_1, resource_group2_1,
                       resource_group1_2, resource_group2_2,
                       lpm_addr_1, lpm_addr_2, slacl_addr_1, slacl_addr_2,
                       epoch1, epoch2) {
    modify_field(vnic_metadata.vnic_id, vnic_id);
    modify_field(vnic_metadata.subnet_id, subnet_id);
    modify_field(vnic_metadata.skip_src_dst_check, skip_src_dst_check);
    modify_field(vnic_metadata.vcn_id, vcn_id);
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
        modify_field(policer_metadata.resource_group1, resource_group1_1);
        modify_field(policer_metadata.resource_group2, resource_group2_1);
        lpm_init(lpm_addr_1);
        slacl_init(slacl_addr_1);
        modify_field(service_header.epoch, epoch1);
    } else {
        modify_field(policer_metadata.resource_group1, resource_group1_2);
        modify_field(policer_metadata.resource_group2, resource_group2_2);
        lpm_init(lpm_addr_2);
        slacl_init(slacl_addr_2);
        modify_field(service_header.epoch, epoch2);
    }
    if (mpls[1].valid == TRUE) {
        modify_field(rvpath_metadata.src_slot_id, mpls[1].label);
    }
}

@pragma stage 0
table local_vnic_by_vlan {
    reads {
        ctag_1.vid : exact;
    }
    actions {
        local_vnic_info;
    }
    size : LOCAL_VNIC_BY_VLAN_TABLE_SIZE;
}

/******************************************************************************/
/* VNIC mapping table (by slot)                                               */
/******************************************************************************/
/* miss action : drop packet */
@pragma stage 0
table local_vnic_by_slot {
    reads {
        mpls[0].label : exact;
    }
    actions {
        local_vnic_info;
    }
    size : LOCAL_VNIC_BY_SLOT_TABLE_SIZE;
}

@pragma stage 0
@pragma overflow_table local_vnic_by_slot
table local_vnic_by_slot_otcam {
    reads {
        mpls[0].label : ternary;
    }
    actions {
        local_vnic_info;
    }
    size : LOCAL_VNIC_BY_SLOT_OTCAM_TABLE_SIZE;
}

control ingress_vnic_info {
    if (mpls[0].valid == TRUE) {
        apply(local_vnic_by_slot);
        apply(local_vnic_by_slot_otcam);
    } else {
        apply(local_vnic_by_vlan);
    }
}

action egress_local_vnic_info(vr_mac, overlay_mac, vlan_id) {
    modify_field(vnic_metadata.vr_mac, vr_mac);
    modify_field(vnic_metadata.overlay_mac, overlay_mac);
    modify_field(vnic_metadata.vlan_id, vlan_id);
}

table egress_local_vnic_info {
    reads {
        capri_i2e_metadata.vnic_id : exact;
    }
    actions {
        egress_local_vnic_info;
    }
    size : EGRESS_LOCAL_VNIC_INFO_TABLE_SIZE;
}

control egress_vnic_info {
    apply(egress_local_vnic_info);
}
