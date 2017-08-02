/*****************************************************************************/
/* Classic mode processing                                                   */
/*****************************************************************************/
action registered_macs_hit(dst_lif) {
    modify_field(capri_intrinsic.lif, dst_lif);
}

action registered_macs_miss() {
    if ((flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) and
        ((control_metadata.lif_filter & LIF_FILTER_ALL_MULTICAST) ==
         LIF_FILTER_ALL_MULTICAST)) {
        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
        modify_field(capri_intrinsic.tm_replicate_ptr,
                     control_metadata.flow_miss_idx);
    } else {
        if ((flow_lkp_metadata.pkt_type == PACKET_TYPE_BROADCAST) and
            ((control_metadata.lif_filter & LIF_FILTER_BROADCAST) ==
             LIF_FILTER_BROADCAST)) {
        modify_field(capri_intrinsic.tm_replicate_en, TRUE);
        add(capri_intrinsic.tm_replicate_ptr,
            control_metadata.flow_miss_idx, 1);
        } else {
            if ((control_metadata.lif_filter & LIF_FILTER_PROMISCUOUS) ==
                LIF_FILTER_PROMISCUOUS) {
                    modify_field(capri_intrinsic.tm_replicate_en, TRUE);
                    add(capri_intrinsic.tm_replicate_ptr,
                        control_metadata.flow_miss_idx, 2);
            } else {
                drop_packet();
            }
        }
    }
}

action registered_macs_otcam_hit(dst_lif) {
    registered_macs_hit(dst_lif);
}

action registered_macs_otcam_miss() {
    registered_macs_miss();
}

@pragma stage 2
table registered_macs {
    reads {
        flow_lkp_metadata.lkp_vrf        : exact;
        flow_lkp_metadata.lkp_dstMacAddr : exact;
    }
    actions {
        registered_macs_hit;
        registered_macs_miss;
    }
    size : REGISTERED_MACS_TABLE_SIZE;
}

@pragma stage 2
@pragma overflow_table registered_macs
table registered_macs_otcam {
    reads {
        flow_lkp_metadata.lkp_vrf        : ternary;
        flow_lkp_metadata.lkp_dstMacAddr : ternary;
    }
    actions {
        registered_macs_otcam_hit;
        registered_macs_otcam_miss;
    }
    size : REGISTERED_MACS_OTCAM_TABLE_SIZE;
}

control process_registered_macs {
    apply(registered_macs);
    apply(registered_macs_otcam);
}
