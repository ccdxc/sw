/*****************************************************************************/
/* Classic mode processing                                                   */
/*****************************************************************************/
action registered_macs(dst_lport, multicast_en, 
                       tunnel_rewrite_en, tunnel_rewrite_index, 
                       dst_if_label, flow_learn, l4_profile_en, l4_profile_idx) {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
    modify_field(qos_metadata.qos_class_id, capri_intrinsic.tm_oq);

    // hit action
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
        if (control_metadata.if_label_check_en == TRUE) {
            if (control_metadata.src_if_label != dst_if_label) {
                if (control_metadata.if_label_check_fail_drop == TRUE) {
                    // GS: Uplink -> *
                    //     - Packets on wrong uplink
                    modify_field(control_metadata.drop_reason, DROP_IF_LABEL_MISMATCH);
                    drop_packet();
                } else {
                    // GS: MSeg_EPs -> *. Packets on other lif
                    //     - Packet will hit reg_mac, but if_label_check_fail_drop is FALSE
                    //     - Treat it as MISS
                    modify_field(control_metadata.registered_mac_miss, TRUE);
                }
            }
        }
    }
    if (multicast_en == TRUE) {
        modify_field(capri_intrinsic.tm_replicate_en, multicast_en);
        modify_field(capri_intrinsic.tm_replicate_ptr, dst_lport);
    } else {
        modify_field(control_metadata.dst_lport, dst_lport);
        // Pkt Uplink -> Host (Access Vlan). Inp Props drive clear_promiscous_repl: 1
        // Pkt has a flow hit and ALG replication has to happen to CPU.
        // clear_promiscuous_repl is killing the replication.
        modify_field(control_metadata.clear_promiscuous_repl, FALSE);
    }
    if (flow_learn == TRUE) {
        modify_field(control_metadata.flow_learn, flow_learn);
    }
    if (tunnel_rewrite_en == TRUE) {
        modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    }
    if (l4_profile_en == TRUE) {
        modify_field(l4_metadata.profile_idx, l4_profile_idx);
    }

    // Miss Action
    if (p4plus_to_p4.dst_lport_valid == TRUE) {
        // return;
    }
    modify_field(control_metadata.registered_mac_miss, TRUE);
    if (flow_lkp_metadata.lkp_reg_mac_vrf == 0) {
        modify_field(control_metadata.drop_reason, DROP_INPUT_PROPERTIES_MISS);
        drop_packet();
    }
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_UNICAST) {
        if (control_metadata.has_prom_host_lifs == TRUE) {
            modify_field(control_metadata.flow_learn, TRUE);
        }
    }
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
        if (control_metadata.mseg_bm_mc_repls == TRUE) {
            modify_field(control_metadata.flow_learn, TRUE);
        }
    }
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST) {
        if (control_metadata.mseg_bm_bc_repls == TRUE) {
            modify_field(control_metadata.flow_learn, TRUE);
        }
    }
    modify_field(scratch_metadata.src_if_label, dst_if_label);
    modify_field(scratch_metadata.flag, tunnel_rewrite_en);
    modify_field(scratch_metadata.flag, l4_profile_en);

    // // Current assumption is for input_properties miss case we don't
    // // need to honor promiscuous receivers list.
    // if (flow_lkp_metadata.lkp_classic_vrf == 0) {
    //     modify_field(control_metadata.drop_reason, DROP_INPUT_PROPERTIES_MISS);
    //     drop_packet();
    // }
    // modify_field(control_metadata.registered_mac_miss, TRUE);

    // if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
    //     modify_field(capri_intrinsic.tm_replicate_en, TRUE);
    //     add(capri_intrinsic.tm_replicate_ptr,
    //         control_metadata.flow_miss_idx, 1);
    // } else {
    //     if (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST) {
    //         modify_field(capri_intrinsic.tm_replicate_en, TRUE);
    //         modify_field(capri_intrinsic.tm_replicate_ptr,
    //                      control_metadata.flow_miss_idx);
    //     } else {
    //         modify_field(capri_intrinsic.tm_replicate_en, TRUE);
    //         add(capri_intrinsic.tm_replicate_ptr,
    //             control_metadata.flow_miss_idx, 2);
    //     }
    // }
}

@pragma stage 2
table registered_macs {
    reads {
        entry_inactive.registered_macs    : exact;
        flow_lkp_metadata.lkp_reg_mac_vrf : exact;
        flow_lkp_metadata.lkp_dstMacAddr  : exact;
    }
    actions {
        registered_macs;
    }
    size : REGISTERED_MACS_TABLE_SIZE;
}


@pragma stage 2
@pragma overflow_table registered_macs
table registered_macs_otcam {
    reads {
        entry_inactive.registered_macs    : ternary;
        flow_lkp_metadata.lkp_reg_mac_vrf : ternary;
        flow_lkp_metadata.lkp_dstMacAddr  : ternary;
    }
    actions {
        registered_macs;
    }
    size : REGISTERED_MACS_OTCAM_TABLE_SIZE;
}

control process_registered_macs {

    apply(registered_macs);
    apply(registered_macs_otcam);

    // if (control_metadata.registered_mac_launch == 1) {
    //     apply(registered_macs);
    //     apply(registered_macs_otcam);
    // }
}
