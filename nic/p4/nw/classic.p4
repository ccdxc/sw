/*****************************************************************************/
/* Classic mode processing                                                   */
/*****************************************************************************/
action registered_macs(dst_lport, multicast_en) {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);

    // hit action
    if (multicast_en == TRUE) {
        modify_field(capri_intrinsic.tm_replicate_en, multicast_en);
        modify_field(capri_intrinsic.tm_replicate_ptr, dst_lport);
    } else {
        modify_field(control_metadata.dst_lport, dst_lport);
    }

    // miss action
    if (control_metadata.uplink == FALSE) {
        // return
    }

    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
        modify_field(capri_intrinsic.tm_replicate_ptr,
                     control_metadata.flow_miss_idx);
    } else {
        if (flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST) {
            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
            add(capri_intrinsic.tm_replicate_ptr,
                control_metadata.flow_miss_idx, 1);
        } else {
            modify_field(capri_intrinsic.tm_replicate_en, TRUE);
            add(capri_intrinsic.tm_replicate_ptr,
                control_metadata.flow_miss_idx, 2);
        }
    }
}

@pragma stage 3
table registered_macs {
    reads {
        entry_inactive.registered_macs   : exact;
        flow_lkp_metadata.lkp_vrf        : exact;
        flow_lkp_metadata.lkp_dstMacAddr : exact;
    }
    actions {
        registered_macs;
    }
    size : REGISTERED_MACS_TABLE_SIZE;
}

@pragma stage 3
@pragma overflow_table registered_macs
table registered_macs_otcam {
    reads {
        entry_inactive.registered_macs   : ternary;
        flow_lkp_metadata.lkp_vrf        : ternary;
        flow_lkp_metadata.lkp_dstMacAddr : ternary;
    }
    actions {
        registered_macs;
    }
    size : REGISTERED_MACS_OTCAM_TABLE_SIZE;
}

control process_registered_macs {
    if (control_metadata.nic_mode == NIC_MODE_CLASSIC) {
        apply(registered_macs);
        apply(registered_macs_otcam);
    }
}
