/*****************************************************************************/
/* DDos accounting                                                           */
/*****************************************************************************/

header_type ddos_metadata_t {
    fields {
        ddos_src_vf_policer_idx       : 10;
        ddos_service_policer_idx      : 10;
        ddos_src_dst_policer_idx      : 11;

        ddos_src_vf_policer_color     : 2;
        ddos_service_policer_color    : 2;
        ddos_src_dst_policer_color    : 2;
    }
}

metadata ddos_metadata_t ddos_metadata;

action ddos_src_vf_hit(ddos_src_vf_base_policer_idx) {

    // Sample write so that compiler knows the size of this field.
    modify_field(scratch_metadata.ddos_src_vf_base_policer_idx,
                 ddos_src_vf_base_policer_idx);

    // SYN Packets
    if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN) {
        modify_field(ddos_metadata.ddos_src_vf_policer_idx,
                     ((ddos_src_vf_base_policer_idx << 2) + DDOS_TYPE_SYN));
    } else {

        // ICMP Packets
        if (flow_lkp_metadata.lkp_proto == IP_PROTO_ICMP) {
            modify_field(ddos_metadata.ddos_src_vf_policer_idx,
                         ((ddos_src_vf_base_policer_idx << 2) + DDOS_TYPE_ICMP));
        } else {

            // UDP Packets
            if (flow_lkp_metadata.lkp_proto == IP_PROTO_UDP) {
                modify_field(ddos_metadata.ddos_src_vf_policer_idx,
                             ((ddos_src_vf_base_policer_idx << 2) + DDOS_TYPE_UDP));
            } else {
                modify_field(ddos_metadata.ddos_src_vf_policer_idx,
                             ((ddos_src_vf_base_policer_idx << 2) + DDOS_TYPE_ANY));
            }

        }

    }
    //// SYN Packets
    //if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN) {
    //    modify_field(ddos_metadata.ddos_src_vf_policer_idx,
    //                 ((ddos_src_vf_base_policer_idx << 2) + DDOS_TYPE_SYN));
    //}
    //// ICMP Packets
    //if (flow_lkp_metadata.lkp_proto == IP_PROTO_ICMP) {
    //    modify_field(ddos_metadata.ddos_src_vf_policer_idx,
    //                 ((ddos_src_vf_base_policer_idx << 2) + DDOS_TYPE_ICMP));
    //}
    //// UDP Packets
    //if (flow_lkp_metadata.lkp_proto == IP_PROTO_UDP) {
    //    modify_field(ddos_metadata.ddos_src_vf_policer_idx,
    //                 ((ddos_src_vf_base_policer_idx << 2) + DDOS_TYPE_UDP));
    //}
    //// ALL/ANY Packets
    //if (ddos_metadata.ddos_src_vf_policer_idx == 0) {
    //    modify_field(ddos_metadata.ddos_src_vf_policer_idx,
    //                 ((ddos_src_vf_base_policer_idx << 2) + DDOS_TYPE_ANY));
    //}

}

action ddos_service_hit(ddos_service_base_policer_idx) {

    // Sample write so that compiler knows the size of this field.
    modify_field(scratch_metadata.ddos_service_base_policer_idx,
                 ddos_service_base_policer_idx);


    // SYN Packets
    if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN) {
        modify_field(ddos_metadata.ddos_service_policer_idx,
                     ((ddos_service_base_policer_idx << 2) + DDOS_TYPE_SYN));
    } else {

        // ICMP Packets
        if (flow_lkp_metadata.lkp_proto == IP_PROTO_ICMP) {
            modify_field(ddos_metadata.ddos_service_policer_idx,
                         ((ddos_service_base_policer_idx << 2) + DDOS_TYPE_ICMP));
        } else {

            // UDP Packets
            if (flow_lkp_metadata.lkp_proto == IP_PROTO_UDP) {
                modify_field(ddos_metadata.ddos_service_policer_idx,
                             ((ddos_service_base_policer_idx << 2) + DDOS_TYPE_UDP));
            } else {
                modify_field(ddos_metadata.ddos_service_policer_idx,
                             ((ddos_service_base_policer_idx << 2) + DDOS_TYPE_ANY));
            }

        }

    }

    //// SYN Packets
    //if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN) {
    //    modify_field(ddos_metadata.ddos_service_policer_idx,
    //                 ((ddos_service_base_policer_idx << 2) + DDOS_TYPE_SYN));
    //}
    //// ICMP Packets
    //if (flow_lkp_metadata.lkp_proto == IP_PROTO_ICMP) {
    //    modify_field(ddos_metadata.ddos_service_policer_idx,
    //                 ((ddos_service_base_policer_idx << 2) + DDOS_TYPE_ICMP));
    //}
    //// UDP Packets
    //if (flow_lkp_metadata.lkp_proto == IP_PROTO_UDP) {
    //    modify_field(ddos_metadata.ddos_service_policer_idx,
    //                 ((ddos_service_base_policer_idx << 2) + DDOS_TYPE_UDP));
    //}
    //// ALL/ANY Packets
    //if (ddos_metadata.ddos_service_policer_idx == 0) {
    //    modify_field(ddos_metadata.ddos_service_policer_idx,
    //                 ((ddos_service_base_policer_idx << 2) + DDOS_TYPE_ANY));
    //}

}

action ddos_src_dst_hit(ddos_src_dst_base_policer_idx) {

    // Sample write so that compiler knows the size of this field.
    modify_field(scratch_metadata.ddos_src_dst_base_policer_idx,
                 ddos_src_dst_base_policer_idx);


    // SYN Packets
    if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN) {
        modify_field(ddos_metadata.ddos_src_dst_policer_idx,
                     ((ddos_src_dst_base_policer_idx << 2) + DDOS_TYPE_SYN));
    } else {

        // ICMP Packets
        if (flow_lkp_metadata.lkp_proto == IP_PROTO_ICMP) {
            modify_field(ddos_metadata.ddos_src_dst_policer_idx,
                         ((ddos_src_dst_base_policer_idx << 2) + DDOS_TYPE_ICMP));
        } else {

            // UDP Packets
            if (flow_lkp_metadata.lkp_proto == IP_PROTO_UDP) {
                modify_field(ddos_metadata.ddos_src_dst_policer_idx,
                             ((ddos_src_dst_base_policer_idx << 2) + DDOS_TYPE_UDP));
            } else {
                modify_field(ddos_metadata.ddos_src_dst_policer_idx,
                             ((ddos_src_dst_base_policer_idx << 2) + DDOS_TYPE_ANY));
            }

        }

    }


    //// SYN Packets
    //if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN) {
    //    modify_field(ddos_metadata.ddos_src_dst_policer_idx,
    //                 ((ddos_src_dst_base_policer_idx << 2) + DDOS_TYPE_SYN));
    //}
    //// ICMP Packets
    //if (flow_lkp_metadata.lkp_proto == IP_PROTO_ICMP) {
    //    modify_field(ddos_metadata.ddos_src_dst_policer_idx,
    //                 ((ddos_src_dst_base_policer_idx << 2) + DDOS_TYPE_ICMP));
    //}
    //// UDP Packets
    //if (flow_lkp_metadata.lkp_proto == IP_PROTO_UDP) {
    //    modify_field(ddos_metadata.ddos_src_dst_policer_idx,
    //                 ((ddos_src_dst_base_policer_idx << 2) + DDOS_TYPE_UDP));
    //}
    //// ALL/ANY Packets
    //if (ddos_metadata.ddos_src_dst_policer_idx == 0) {
    //    modify_field(ddos_metadata.ddos_src_dst_policer_idx,
    //                 ((ddos_src_dst_base_policer_idx << 2) + DDOS_TYPE_ANY));
    //}

}
//
// This table should be called only for traffic coming from VF.
// Key will be only lif and it can be a index table so
// no overflow tcam needed.
//
@pragma stage 1
table ddos_src_vf {
    reads {
        control_metadata.src_lif : ternary;
    }
    actions {
        nop;
        ddos_src_vf_hit;
    }
    size : DDOS_SRC_VF_TABLE_SIZE;
}

@pragma stage 3
table ddos_src_dst {
    reads {
        flow_lkp_metadata.lkp_vrf   : ternary;
        flow_lkp_metadata.lkp_src   : ternary;
        flow_lkp_metadata.lkp_dst   : ternary;
        flow_lkp_metadata.lkp_proto : ternary;
        flow_lkp_metadata.lkp_dport : ternary;
    }
    actions {
        nop;
        ddos_src_dst_hit;
    }
    size : DDOS_SRC_DST_TABLE_SIZE;
}


@pragma stage 3
table ddos_service {
    reads {
        flow_lkp_metadata.lkp_vrf   : ternary;
        flow_lkp_metadata.lkp_dst   : ternary;
        flow_lkp_metadata.lkp_proto : ternary;
        flow_lkp_metadata.lkp_dport : ternary;
    }
    actions {
        nop;
        ddos_service_hit;
    }
    size : DDOS_SERVICE_TABLE_SIZE;
}

meter ddos_src_vf_policer {
    type           : packets;
    static         : ddos_src_vf_policer;
    result         : ddos_metadata.ddos_src_vf_policer_color;
    instance_count : DDOS_SRC_VF_POLICER_TABLE_SIZE;
}

action execute_ddos_src_vf_policer() {
    execute_meter(ddos_src_vf_policer, ddos_metadata.ddos_src_vf_policer_idx,
                  ddos_metadata.ddos_src_vf_policer_color);
#if 0
    // We will drop the packet in the policer action routine based on
    // our current state, not just based on policer output.
    if (ddos_metadata.ddos_src_vf_policer_color == POLICER_COLOR_RED) {
        modify_field(control_metadata.ddos_src_vf_policer_drop, TRUE);
        drop_packet();
    }
#endif /* 0 */
}

@pragma stage 2
@pragma policer_table three_color
table ddos_src_vf_policer {
    reads {
        ddos_metadata.ddos_src_vf_policer_idx : exact;
    }
    actions {
        execute_ddos_src_vf_policer;
    }
    size : DDOS_SRC_VF_POLICER_TABLE_SIZE;
}



action ddos_src_vf_policer_action(ddos_src_vf_policer_idx,
                                  ddos_src_vf_policer_saved_color,
                                  ddos_src_vf_policer_dropped_packets) {

    /* dummy ops to keep compiler happy */
    /*
    modify_field(scratch_metadata.ddos_src_vf_policer_idx,
                 ddos_src_vf_policer_idx);
    */
    modify_field(scratch_metadata.ddos_src_vf_policer_saved_color,
                 ddos_src_vf_policer_saved_color);
    modify_field(scratch_metadata.ddos_src_vf_policer_dropped_packets,
                 ddos_src_vf_policer_dropped_packets);

    if (ddos_metadata.ddos_src_vf_policer_color > scratch_metadata.ddos_src_vf_policer_saved_color) {
        modify_field(scratch_metadata.ddos_src_vf_policer_saved_color,
                     ddos_metadata.ddos_src_vf_policer_color);

        // Need to also update the HBM or SRAM memory so that the Software can know about this change.
        // We will maintain memory for the total # of policers and scan them for any change and process
        // them in ARM CPU.
    }
    if (scratch_metadata.ddos_src_vf_policer_saved_color == POLICER_COLOR_RED) {
        add_to_field(scratch_metadata.ddos_src_vf_policer_dropped_packets, 1);
        modify_field(control_metadata.egress_ddos_src_vf_policer_drop, TRUE);
        drop_packet();
    }
    if (scratch_metadata.ddos_src_vf_policer_saved_color == POLICER_COLOR_YELLOW) {
       // TBD
       // We need to Random Drop.
        add_to_field(scratch_metadata.ddos_src_vf_policer_dropped_packets, 1);
        modify_field(control_metadata.egress_ddos_src_vf_policer_drop, TRUE);
       drop_packet();
    }
}

@pragma stage 3
table ddos_src_vf_policer_action {
    reads {
        ddos_metadata.ddos_src_vf_policer_idx : exact;
    }
    actions {
        nop;
        ddos_src_vf_policer_action;
    }
    default_action : nop;
    size : DDOS_SRC_VF_POLICER_TABLE_SIZE;
}

meter ddos_service_policer {
    type           : packets;
    static         : ddos_service_policer;
    result         : ddos_metadata.ddos_service_policer_color;
    instance_count : DDOS_SERVICE_POLICER_TABLE_SIZE;
}

action execute_ddos_service_policer() {
    execute_meter(ddos_service_policer, ddos_metadata.ddos_service_policer_idx,
                  ddos_metadata.ddos_service_policer_color);
#if 0
    // We will drop the packet in the policer action routine based on
    // our current state, not just based on policer output.
    if (ddos_metadata.ddos_service_policer_color == POLICER_COLOR_RED) {
        modify_field(control_metadata.ddos_service_policer_drop, TRUE);
        drop_packet();
    }
#endif /* 0 */
}

@pragma stage 2
@pragma policer_table three_color
table ddos_service_policer {
    reads {
        ddos_metadata.ddos_service_policer_idx : exact;
    }
    actions {
        execute_ddos_service_policer;
    }
    size : DDOS_SERVICE_POLICER_TABLE_SIZE;
}


action ddos_service_policer_action(ddos_service_policer_idx,
                                   ddos_service_policer_saved_color,
                                   ddos_service_policer_dropped_packets) {
    /* dummy ops to keep compiler happy */
    modify_field(scratch_metadata.ddos_service_policer_saved_color,
                 ddos_service_policer_saved_color);
    modify_field(scratch_metadata.ddos_service_policer_dropped_packets,
                 ddos_service_policer_dropped_packets);

    if (ddos_metadata.ddos_service_policer_color > scratch_metadata.ddos_service_policer_saved_color) {
        modify_field(scratch_metadata.ddos_service_policer_saved_color,
                     ddos_metadata.ddos_service_policer_color);

        // Need to also update the HBM or SRAM memory so that the Software can know about this change.
        // We will maintain memory for the total # of policers and scan them for any change and process
        // them in ARM CPU.
    }
    if (scratch_metadata.ddos_service_policer_saved_color == POLICER_COLOR_RED) {
        add_to_field(scratch_metadata.ddos_service_policer_dropped_packets, 1);
        modify_field(control_metadata.egress_ddos_service_policer_drop, TRUE);
        drop_packet();
    }
    if (scratch_metadata.ddos_service_policer_saved_color == POLICER_COLOR_YELLOW) {
       // TBD
       // We need to Random Drop.
        add_to_field(scratch_metadata.ddos_service_policer_dropped_packets, 1);
        modify_field(control_metadata.egress_ddos_service_policer_drop, TRUE);
        drop_packet();
    }

}

@pragma stage 3
table ddos_service_policer_action {
    reads {
        ddos_metadata.ddos_service_policer_idx : exact;
    }
    actions {
        nop;
        ddos_service_policer_action;
    }
    default_action : nop;
    size : DDOS_SERVICE_POLICER_TABLE_SIZE;
}

meter ddos_src_dst_policer {
    type           : packets;
    static         : ddos_src_dst_policer;
    result         : ddos_metadata.ddos_src_dst_policer_color;
    instance_count : DDOS_SRC_DST_POLICER_TABLE_SIZE;
}

action execute_ddos_src_dst_policer() {
    execute_meter(ddos_src_dst_policer, ddos_metadata.ddos_src_dst_policer_idx,
                  ddos_metadata.ddos_src_dst_policer_color);
#if 0
    // We will drop the packet in the policer action routine based on
    // our current state, not just based on policer output.
    if (ddos_metadata.ddos_src_dst_policer_color == POLICER_COLOR_RED) {
        modify_field(control_metadata.ddos_src_dst_policer_drop, TRUE);
        drop_packet();
    }
#endif /* 0 */
}

@pragma stage 2
@pragma policer_table three_color
table ddos_src_dst_policer {
    reads {
        ddos_metadata.ddos_src_dst_policer_idx : exact;
    }
    actions {
        execute_ddos_src_dst_policer;
    }
    size : DDOS_SRC_DST_POLICER_TABLE_SIZE;
}


action ddos_src_dst_policer_action(ddos_src_dst_policer_idx,
                                   ddos_src_dst_policer_saved_color,
                                   ddos_src_dst_policer_dropped_packets) {

    /* dummy ops to keep compiler happy */
    modify_field(scratch_metadata.ddos_src_dst_policer_saved_color,
                 ddos_src_dst_policer_saved_color);
    modify_field(scratch_metadata.ddos_src_dst_policer_dropped_packets,
                 ddos_src_dst_policer_dropped_packets);

    if (ddos_metadata.ddos_src_dst_policer_color > scratch_metadata.ddos_src_dst_policer_saved_color) {
        modify_field(scratch_metadata.ddos_src_dst_policer_saved_color,
                     ddos_metadata.ddos_src_dst_policer_color);

        // Need to also update the HBM or SRAM memory so that the Software can know about this change.
        // We will maintain memory for the total # of policers and scan them for any change and process
        // them in ARM CPU.
    }
    if (scratch_metadata.ddos_src_dst_policer_saved_color == POLICER_COLOR_RED) {
        add_to_field(scratch_metadata.ddos_src_dst_policer_dropped_packets, 1);
        modify_field(control_metadata.egress_ddos_src_dst_policer_drop, TRUE);
        drop_packet();
    }
    if (scratch_metadata.ddos_src_dst_policer_saved_color == POLICER_COLOR_YELLOW) {
       // TBD
       // We need to Random Drop.
       add_to_field(scratch_metadata.ddos_src_dst_policer_dropped_packets, 1);
       modify_field(control_metadata.egress_ddos_src_dst_policer_drop, TRUE);
       drop_packet();
    }

}

@pragma stage 3
table ddos_src_dst_policer_action {
    reads {
        ddos_metadata.ddos_src_dst_policer_idx : exact;
    }
    actions {
        nop;
        ddos_src_dst_policer_action;
    }
    default_action : nop;
    size : DDOS_SRC_DST_POLICER_TABLE_SIZE;
}

control process_ddos_ingress {
    if (control_metadata.flow_miss_ingress == TRUE) {
        apply(ddos_src_dst);
        apply(ddos_service);
    }
}

control process_ddos_egress {
    if (control_metadata.flow_miss_egress == TRUE) {
        apply(ddos_src_vf);

        apply(ddos_src_vf_policer);
        apply(ddos_src_dst_policer);
        apply(ddos_service_policer);

        apply(ddos_src_vf_policer_action);
        apply(ddos_src_dst_policer_action);
        apply(ddos_service_policer_action);
    }
}
