/******************************************************************************/
/* VNIC mapping table (by VLAN)                                               */
/******************************************************************************/
action local_vnic_info_common(dir, mode, src_mac, local_vnic_tag, vpc_id,
                              epoch1_valid, epoch1, skip_src_dst_check1,
                              lpm_v4addr1, lpm_v6addr1, sacl_v4addr1,
                              sacl_v6addr1, resource_group1, overlay_mac1,
                              epoch2_valid, epoch2, skip_src_dst_check2,
                              lpm_v4addr2, lpm_v6addr2, sacl_v4addr2,
                              sacl_v6addr2, resource_group2, overlay_mac2) {
    if (service_header.valid == TRUE) {
        if (service_header.epoch == epoch1) {
            modify_field(scratch_metadata.use_epoch1, TRUE);
            modify_field(scratch_metadata.epoch_valid, epoch1_valid);
        } else {
            modify_field(scratch_metadata.use_epoch1, FALSE);
            modify_field(scratch_metadata.epoch_valid, epoch2_valid);
        }
    } else {
        if (epoch1 > epoch2) {
            modify_field(scratch_metadata.use_epoch1, TRUE);
            modify_field(scratch_metadata.epoch_valid, epoch1_valid);
        } else {
            modify_field(scratch_metadata.use_epoch1, FALSE);
            modify_field(scratch_metadata.epoch_valid, epoch2_valid);
        }
    }

    if (scratch_metadata.epoch_valid == FALSE) {
        if (dir == TX_FROM_HOST) {
            modify_field(control_metadata.p4i_drop_reason,
                         1 << P4I_DROP_VNIC_INFO_TX_MISS);
        } else {
            modify_field(control_metadata.p4i_drop_reason,
                         1 << P4I_DROP_VNIC_INFO_RX_MISS);
        }
        drop_packet();
    }

    if (scratch_metadata.use_epoch1 == TRUE) {
        modify_field(scratch_metadata.overlay_mac, overlay_mac1);
        modify_field(vnic_metadata.skip_src_dst_check, skip_src_dst_check1);
        modify_field(policer_metadata.resource_group, resource_group1);
        lpm_init(lpm_v4addr1, lpm_v6addr1);
        sacl_init(sacl_v4addr1, sacl_v6addr1);
        modify_field(service_header.epoch, epoch1);
    } else {
        modify_field(scratch_metadata.overlay_mac, overlay_mac2);
        modify_field(vnic_metadata.skip_src_dst_check, skip_src_dst_check2);
        modify_field(policer_metadata.resource_group, resource_group2);
        lpm_init(lpm_v4addr2, lpm_v6addr2);
        sacl_init(sacl_v4addr2, sacl_v6addr2);
        modify_field(service_header.epoch, epoch2);
    }
    modify_field(scratch_metadata.local_vnic_tag, local_vnic_tag);
    modify_field(scratch_metadata.vpc_id, vpc_id);
    modify_field(vnic_metadata.local_vnic_tag, scratch_metadata.local_vnic_tag);
    modify_field(vnic_metadata.vpc_id, scratch_metadata.vpc_id);
    modify_field(control_metadata.mode, mode);
    if (mode == APOLLO_MODE_DEFAULT) {
        modify_field(key_metadata.lkp_id, scratch_metadata.local_vnic_tag);
    } else {
        modify_field(key_metadata.lkp_id, scratch_metadata.vpc_id);
    }

    if (dir == TX_FROM_HOST) {
        if (src_mac != scratch_metadata.overlay_mac) {
            modify_field(control_metadata.p4i_drop_reason,
                         1 << P4I_DROP_SRC_MAC_MISMATCH);
            drop_packet();
        }
    }
}

action local_vnic_info_tx(mode, local_vnic_tag, vpc_id,
                          epoch1_valid, epoch1, skip_src_dst_check1,
                          lpm_v4addr1, lpm_v6addr1, sacl_v4addr1, sacl_v6addr1,
                          resource_group1, overlay_mac1,
                          epoch2_valid, epoch2, skip_src_dst_check2,
                          lpm_v4addr2, lpm_v6addr2, sacl_v4addr2, sacl_v6addr2,
                          resource_group2, overlay_mac2,
                          mirror_en, mirror_session) {
    // if c1 indicate miss, drop
    //  modify_field(control_metadata.p4i_drop_reason,
    //               1 << P4I_DROP_VNIC_INFO_TX_MISS);
    //  drop_packet();

    // validate source mac
    if (ethernet_1.srcAddr == 0) {
        modify_field(control_metadata.p4i_drop_reason,
                     1 << P4I_DROP_SRC_MAC_ZERO);
        drop_packet();
    }

    modify_field(scratch_metadata.flag, mirror_en);
    if (scratch_metadata.flag == TRUE) {
        modify_field(control_metadata.mirror_session, mirror_session);
    }

    local_vnic_info_common(TX_FROM_HOST, mode, ethernet_1.srcAddr, local_vnic_tag,
                           vpc_id, epoch1_valid, epoch1, skip_src_dst_check1,
                           lpm_v4addr1, lpm_v6addr1, sacl_v4addr1, sacl_v6addr1,
                           resource_group1, overlay_mac1,
                           epoch2_valid, epoch2, skip_src_dst_check2,
                           lpm_v4addr2, lpm_v6addr2, sacl_v4addr2, sacl_v6addr2,
                           resource_group2, overlay_mac2);
}

action local_vnic_info_rx(mode, local_vnic_tag, vpc_id,
                          epoch1_valid, epoch1, skip_src_dst_check1,
                          lpm_v4addr1, lpm_v6addr1, sacl_v4addr1, sacl_v6addr1,
                          resource_group1,
                          epoch2_valid, epoch2, skip_src_dst_check2,
                          lpm_v4addr2, lpm_v6addr2, sacl_v4addr2, sacl_v6addr2,
                          resource_group2) {

    // if c1 indicate miss, drop
    //  modify_field(control_metadata.p4i_drop_reason,
    //               1 << P4I_DROP_VNIC_INFO_RX_MISS);
    //  drop_packet();

    // validate dst ip; mytep_ip is a table constant
    if (ipv4_1.dstAddr != scratch_metadata.mytep_ip) {
        modify_field(control_metadata.p4i_drop_reason,
                     1 << P4I_DROP_TEP_RX_DST_IP_MISMATCH);
        drop_packet();
    }

    modify_field(vnic_metadata.src_slot_id, mpls_src.label);
    modify_field(control_metadata.tunneled_packet, TRUE);
    local_vnic_info_common(RX_FROM_SWITCH, mode, 0, local_vnic_tag, vpc_id,
                           epoch1_valid, epoch1, skip_src_dst_check1,
                           lpm_v4addr1, lpm_v6addr1,
                           sacl_v4addr1, sacl_v6addr1, resource_group1, 0,
                           epoch2_valid, epoch2, skip_src_dst_check2,
                           lpm_v4addr2, lpm_v6addr2,
                           sacl_v4addr2, sacl_v6addr2, resource_group2, 0);
}

@pragma stage 0
table local_vnic_by_vlan_tx {
    reads {
        capri_intrinsic.lif : ternary;
        ctag_1.vid          : ternary;
        ethernet_1.srcAddr  : ternary;
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
        vxlan_1.vni     : exact;
        mpls_dst.label  : exact;
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
        vxlan_1.vni     : ternary;
        mpls_dst.label  : ternary;
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

action egress_local_vnic_info(vr_mac, overlay_mac, overlay_vlan_id, subnet_id,
                              src_slot_id, lif, qtype, qid, mirror_en, mirror_session) {
    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(rewrite_metadata.src_slot_id, src_slot_id);
        modify_field(capri_intrinsic.tm_oport, TM_PORT_UPLINK_1);
    } else {
        modify_field(scratch_metadata.flag, mirror_en);
        if (scratch_metadata.flag == TRUE) {
            modify_field(control_metadata.mirror_session, mirror_session);
        }

        // set oport if lif is non-zero
        if (lif != 0) {
            modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
            modify_field(capri_intrinsic.lif, lif);
            modify_field(capri_rxdma_intrinsic.qtype, qtype);
            modify_field(capri_rxdma_intrinsic.qid, qid);
        } else {
            modify_field(capri_intrinsic.tm_oport, TM_PORT_UPLINK_0);
        }

        // add header towards host
        if (ethernet_2.valid == 1) {
            copy_header(ethernet_0, ethernet_2);
            modify_field(ethernet_0.srcAddr, vr_mac);
        } else {
            add_header(ethernet_0);
            modify_field(ethernet_0.dstAddr, overlay_mac);
            if ((subnet_id == p4e_apollo_i2e.rvpath_subnet_id) and
                (p4e_apollo_i2e.rvpath_overlay_mac != 0)) {
                modify_field(ethernet_0.srcAddr, p4e_apollo_i2e.rvpath_overlay_mac);
            } else {
                modify_field(ethernet_0.srcAddr, vr_mac);
            }
        }

        if (overlay_vlan_id != 0) {
            add_header(ctag_0);
            modify_field(ctag_0.vid, overlay_vlan_id);
            if (ipv4_2.valid == TRUE) {
                modify_field(ctag_0.etherType, ETHERTYPE_IPV4);
            } else {
                if (ipv6_2.valid == TRUE) {
                    modify_field(ctag_0.etherType, ETHERTYPE_IPV6);
                }
                // TODO: what happens to non-ip packets ?
            }
            modify_field(ethernet_0.etherType, ETHERTYPE_CTAG);
        } else {
            if (ipv4_2.valid == TRUE) {
                modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);
            } else {
                if (ipv6_2.valid == TRUE) {
                    modify_field(ethernet_0.etherType, ETHERTYPE_IPV6);
                }
                // TODO: what happens to non-ip packets ?
            }
        }

        if (vxlan_1.valid == TRUE) {
            modify_field(scratch_metadata.packet_len, capri_p4_intrinsic.frame_size - offset_metadata.l2_2);
        } else {
            modify_field(scratch_metadata.packet_len, capri_p4_intrinsic.frame_size - offset_metadata.l3_2 + 14);
        }
        if (overlay_vlan_id != 0) {
            add_to_field(scratch_metadata.packet_len, 4);
        }
        modify_field(capri_p4_intrinsic.packet_len, scratch_metadata.packet_len);

        // remove headers
        remove_header(ethernet_1);
        remove_header(ethernet_2);
        remove_header(ipv4_1);
        remove_header(ipv6_1);
        remove_header(gre_1);
        remove_header(vxlan_1);
        remove_header(mpls_src);
        remove_header(mpls_dst);

        // scratch metadata
        modify_field(scratch_metadata.subnet_id, subnet_id);
    }
}

@pragma stage 0
@pragma index_table
table egress_local_vnic_info {
    reads {
        p4e_apollo_i2e.local_vnic_tag   : exact;
    }
    actions {
        egress_local_vnic_info;
    }
    size : EGRESS_LOCAL_VNIC_INFO_TABLE_SIZE;
}

control egress_vnic_info {
    apply(egress_local_vnic_info);
}
