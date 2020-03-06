action session_info(valid_flag, skip_flow_log, conntrack_id, timestamp, smac,
                    h2s_epoch_vnic_value, h2s_epoch_vnic_id, h2s_epoch_mapping_value, h2s_epoch_mapping_id,
                    h2s_throttle_bw1_id, h2s_throttle_bw2_id,
                    h2s_vnic_statistics_id, h2s_vnic_statistics_mask,
                    h2s_vnic_histogram_packet_len_id, h2s_vnic_histogram_latency_id,
                    h2s_slow_path_tcp_flags_match, h2s_session_rewrite_id, h2s_egress_action,
                    h2s_allowed_flow_state_bitmap,
                    s2h_epoch_vnic_value, s2h_epoch_vnic_id, s2h_epoch_mapping_value, s2h_epoch_mapping_id,
                    s2h_throttle_bw1_id, s2h_throttle_bw2_id,
                    s2h_vnic_statistics_id, s2h_vnic_statistics_mask,
                    s2h_vnic_histogram_packet_len_id, s2h_vnic_histogram_latency_id,
                    s2h_slow_path_tcp_flags_match, s2h_session_rewrite_id, s2h_egress_action,
                    s2h_allowed_flow_state_bitmap
                    ) {
    if (valid_flag == TRUE) {
        modify_field(control_metadata.skip_flow_log, skip_flow_log);
        if (conntrack_id != 0) {
            modify_field(control_metadata.conntrack_index, conntrack_id);
            modify_field(control_metadata.conntrack_index_valid, TRUE);
        }

        if (control_metadata.direction == TX_FROM_HOST) {
            if (ethernet_1.srcAddr != smac) {
                modify_field(control_metadata.flow_miss, TRUE);
            }
            modify_field(scratch_metadata.mac, smac);

            if (h2s_epoch_vnic_id != 0) {
                modify_field(control_metadata.epoch1_id, h2s_epoch_vnic_id);
                modify_field(control_metadata.epoch1_value, h2s_epoch_vnic_value);
                modify_field(control_metadata.epoch1_id_valid, TRUE);
            }
            if (h2s_epoch_mapping_id != 0) {
                modify_field(control_metadata.epoch2_id, h2s_epoch_mapping_id);
                modify_field(control_metadata.epoch2_value, h2s_epoch_mapping_value);
                modify_field(control_metadata.epoch2_id_valid, TRUE);
            }

            if (h2s_throttle_bw1_id != 0) {
                modify_field(control_metadata.throttle_bw1_id, h2s_throttle_bw1_id);
                modify_field(control_metadata.throttle_bw1_id_valid, TRUE);
            }
            if (h2s_throttle_bw2_id != 0) {
                modify_field(control_metadata.throttle_bw2_id, h2s_throttle_bw2_id);
                modify_field(control_metadata.throttle_bw2_id_valid, TRUE);
            }

            if (h2s_vnic_statistics_id != 0) {
                modify_field(control_metadata.vnic_statistics_id, h2s_vnic_statistics_id);
                modify_field(control_metadata.statistics_id_valid, TRUE);
                modify_field(control_metadata.vnic_statistics_mask, h2s_vnic_statistics_mask);
            }

            if (h2s_vnic_histogram_packet_len_id != 0) {
                modify_field(control_metadata.histogram_packet_len_id, h2s_vnic_histogram_packet_len_id);
                modify_field(control_metadata.histogram_packet_len_id_valid, TRUE);
            }
            if (h2s_vnic_histogram_latency_id != 0) {
                modify_field(control_metadata.histogram_latency_id, h2s_vnic_histogram_latency_id);
                modify_field(control_metadata.histogram_latency_id_valid, TRUE);
            }

            if (control_metadata.l2_vnic == FALSE) {
                if (h2s_session_rewrite_id != 0) {
                    modify_field(control_metadata.session_rewrite_id, h2s_session_rewrite_id);
                }
                else {
                    modify_field(control_metadata.flow_miss, TRUE);
                }
            }

            if ((tcp.flags & h2s_slow_path_tcp_flags_match) != 0) {
                modify_field(control_metadata.flow_miss, TRUE);
            }
            modify_field(scratch_metadata.tcp_flags, h2s_slow_path_tcp_flags_match);

            modify_field(control_metadata.egress_action, h2s_egress_action);
            modify_field(control_metadata.allowed_flow_state_bitmap, h2s_allowed_flow_state_bitmap);

        }
        if (control_metadata.direction == RX_FROM_SWITCH) {
            if (s2h_epoch_vnic_id != 0) {
                modify_field(control_metadata.epoch1_id, s2h_epoch_vnic_id);
                modify_field(control_metadata.epoch1_value, s2h_epoch_vnic_value);
                modify_field(control_metadata.epoch1_id_valid, TRUE);
            }
            if (s2h_epoch_mapping_id != 0) {
                modify_field(control_metadata.epoch2_id, s2h_epoch_mapping_id);
                modify_field(control_metadata.epoch2_value, s2h_epoch_mapping_value);
                modify_field(control_metadata.epoch2_id_valid, TRUE);
            }

            if (s2h_throttle_bw1_id != 0) {
                modify_field(control_metadata.throttle_bw1_id, s2h_throttle_bw1_id);
                modify_field(control_metadata.throttle_bw1_id_valid, TRUE);
            }
            if (s2h_throttle_bw2_id != 0) {
                modify_field(control_metadata.throttle_bw2_id, s2h_throttle_bw2_id);
                modify_field(control_metadata.throttle_bw2_id_valid, TRUE);
            }

            if (s2h_vnic_statistics_id != 0) {
                modify_field(control_metadata.vnic_statistics_id, s2h_vnic_statistics_id);
                modify_field(control_metadata.statistics_id_valid, TRUE);
                modify_field(control_metadata.vnic_statistics_mask, s2h_vnic_statistics_mask);
            }

            if (s2h_vnic_histogram_packet_len_id != 0) {
                modify_field(control_metadata.histogram_packet_len_id, s2h_vnic_histogram_packet_len_id);
                modify_field(control_metadata.histogram_packet_len_id_valid, TRUE);
            }
            if (s2h_vnic_histogram_latency_id != 0) {
                modify_field(control_metadata.histogram_latency_id, s2h_vnic_histogram_latency_id);
                modify_field(control_metadata.histogram_latency_id_valid, TRUE);
            }
            if (control_metadata.l2_vnic == FALSE) {
                if (s2h_session_rewrite_id != 0) {
                    modify_field(control_metadata.session_rewrite_id, s2h_session_rewrite_id);
                }
                else {
                    modify_field(control_metadata.flow_miss, TRUE);
                }
            }

            if ((tcp.flags & s2h_slow_path_tcp_flags_match) != 0) {
                modify_field(control_metadata.flow_miss, TRUE);
            }
            modify_field(scratch_metadata.tcp_flags, s2h_slow_path_tcp_flags_match);

            modify_field(control_metadata.egress_action, s2h_egress_action);
            modify_field(control_metadata.allowed_flow_state_bitmap, s2h_allowed_flow_state_bitmap);
        }

        modify_field(scratch_metadata.timestamp, timestamp);
        modify_field(scratch_metadata.flag, valid_flag);
    }
    else {
        modify_field(control_metadata.flow_miss, TRUE);
    }
}

@pragma stage 1
@pragma hbm_table
@pragma table_write
@pragma index_table
@pragma capi_bitfields_struct
table session_info{
    reads {
        control_metadata.session_index   : exact;
    }
    actions {
        session_info;
    }
    size : SESSION_TABLE_SIZE;
}

#define SESSION_REWRITE_COMMON_FIELDS       valid_flag,                                 \
                    strip_outer_encap_flag, strip_l2_header_flag, strip_vlan_tag_flag

action session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS) {
    if (valid_flag == TRUE) {

        if (strip_l2_header_flag == TRUE) {
            remove_header(ethernet_1);
        }
        
        if (strip_vlan_tag_flag == TRUE) {
            remove_header(ctag_1);
        }

        if (control_metadata.direction == RX_FROM_SWITCH) {
            if (strip_outer_encap_flag == TRUE) {
                remove_header(ipv4_1); /* IPv4 underlay only */
                remove_header(udp_1);
                remove_header(mpls_src);
                remove_header(mpls_dst);
                remove_header(mpls_label3_1);
            }
        }

        modify_field(scratch_metadata.packet_len, p4i_to_p4e_header.packet_len);

        modify_field(scratch_metadata.flag, strip_outer_encap_flag);
        modify_field(scratch_metadata.flag, strip_l2_header_flag);
        modify_field(scratch_metadata.flag, strip_vlan_tag_flag);

#if 0
        modify_field(ipv4_2.srcAddr, scratch_metadata.addr);
        modify_field(ipv4_2.dstAddr, scratch_metadata.addr);
        modify_field(ipv6_2.srcAddr, nat_address);
        modify_field(ipv6_2.dstAddr, nat_address);
        modify_field(control_metadata.egress_action, egress_action);
#endif
    }
    else {
        modify_field(control_metadata.flow_miss, TRUE);
    }

    modify_field(scratch_metadata.flag, valid_flag);
}

action session_rewrite(SESSION_REWRITE_COMMON_FIELDS) {
    session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS);
}

action session_rewrite_ipv4_snat(SESSION_REWRITE_COMMON_FIELDS,
                    ipv4_addr_snat) {
    
    session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS);

    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(ipv4_1.srcAddr, ipv4_addr_snat);
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        modify_field(ipv4_2.srcAddr, ipv4_addr_snat);
    }
}

action session_rewrite_ipv4_dnat(SESSION_REWRITE_COMMON_FIELDS,
                    ipv4_addr_dnat) {
    
    session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS);

    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(ipv4_1.dstAddr, ipv4_addr_dnat);
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        modify_field(ipv4_2.dstAddr, ipv4_addr_dnat);
    }
}

action session_rewrite_ipv4_pat(SESSION_REWRITE_COMMON_FIELDS,
                    ipv4_addr_spat, ipv4_addr_dpat, l4_port_spat, l4_port_dpat) {
    
    session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS);

    if (control_metadata.direction == TX_FROM_HOST) {
        if (ipv4_addr_spat != 0) {
            modify_field(ipv4_1.srcAddr, ipv4_addr_spat);
        }
        if (ipv4_addr_dpat != 0) {
            modify_field(ipv4_1.dstAddr, ipv4_addr_dpat);
        }
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        if (ipv4_addr_spat != 0) {
            modify_field(ipv4_2.srcAddr, ipv4_addr_spat);
        }
        if (ipv4_addr_dpat != 0) {
            modify_field(ipv4_2.dstAddr, ipv4_addr_dpat);
        }
    }
    if (l4_port_spat != 0) {
        modify_field(tcp.srcPort, l4_port_spat);
    }
    if (l4_port_dpat != 0) {
        modify_field(tcp.dstPort, l4_port_dpat);
    }
}

action session_rewrite_ipv6_snat(SESSION_REWRITE_COMMON_FIELDS,
                    ipv6_addr_snat) {
    
    session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS);

    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(ipv6_1.srcAddr, ipv6_addr_snat);
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        modify_field(ipv6_2.srcAddr, ipv6_addr_snat);
    }
}

action session_rewrite_ipv6_dnat(SESSION_REWRITE_COMMON_FIELDS,
                    ipv6_addr_dnat) {
    
    session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS);

    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(ipv6_1.dstAddr, ipv6_addr_dnat);
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        modify_field(ipv6_2.dstAddr, ipv6_addr_dnat);
    }
}


@pragma stage 5
@pragma hbm_table
@pragma index_table
@pragma capi_bitfields_struct
table session_rewrite {
    reads {
        control_metadata.session_rewrite_id: exact;
    }
    actions {
        session_rewrite;
        session_rewrite_ipv4_snat;
        session_rewrite_ipv4_dnat;
        session_rewrite_ipv4_pat;
        session_rewrite_ipv6_snat;
        session_rewrite_ipv6_dnat;
    }
    size : SESSION_TABLE_SIZE;
}
#define SESSION_REWRITE_ENCAP_COMMON_FIELDS       valid_flag,                   \
                    add_vlan_tag_flag, dmac, smac, vlan

action session_rewrite_encap_common(SESSION_REWRITE_ENCAP_COMMON_FIELDS) {
    if (valid_flag == TRUE) {

        modify_field(ethernet_0.srcAddr, smac);
        modify_field(ethernet_0.dstAddr, dmac);
        add_header(ethernet_0);

        if (add_vlan_tag_flag == TRUE) {
            modify_field(ctag_0.vid, vlan);
            add_header(ctag_0);
        }
        modify_field(scratch_metadata.flag, add_vlan_tag_flag);

        modify_field(scratch_metadata.packet_len, p4i_to_p4e_header.packet_len);

    }
    else {
        modify_field(control_metadata.flow_miss, TRUE);
    }

    modify_field(scratch_metadata.flag, valid_flag);
}

action session_rewrite_encap_l2(SESSION_REWRITE_ENCAP_COMMON_FIELDS) {
    session_rewrite_encap_common(SESSION_REWRITE_ENCAP_COMMON_FIELDS);
}

action session_rewrite_encap_mplsoudp(SESSION_REWRITE_ENCAP_COMMON_FIELDS,      \
                    ipv4_da, ipv4_sa,                                           \
                    udp_sport, udp_dport,                                       \
                    mpls_label1, mpls_label2, mpls_label3) {
    session_rewrite_encap_common(SESSION_REWRITE_ENCAP_COMMON_FIELDS);

    modify_field(ipv4_0.dstAddr, ipv4_da);
    modify_field(ipv4_0.srcAddr, ipv4_sa);
    add_header(ipv4_0);

    modify_field(udp_0.srcPort, udp_sport);
    modify_field(udp_0.dstPort, udp_dport);
    add_header(udp_0);
    
    modify_field(scratch_metadata.mpls_label, mpls_label1);
    modify_field(scratch_metadata.mpls_label, mpls_label2);
    modify_field(scratch_metadata.mpls_label, mpls_label3);
    add_header(mpls_label1_0);
    add_header(mpls_label2_0);
    add_header(mpls_label3_0);
}

action session_rewrite_encap_geneve(SESSION_REWRITE_ENCAP_COMMON_FIELDS,        \
                    ipv4_da, ipv4_sa,                                           \
                    udp_sport, udp_dport,                                       \
                    vni, source_slot_id, destination_slot_id,                   \
                    sg_id1, sg_id2, sg_id3, sg_id4, sg_id5, sg_id6,             \
                    originator_physical_ip                                      \
                    ) {
    session_rewrite_encap_common(SESSION_REWRITE_ENCAP_COMMON_FIELDS);

    modify_field(ipv4_0.dstAddr, ipv4_da);
    modify_field(ipv4_0.srcAddr, ipv4_sa);
    add_header(ipv4_0);

    modify_field(udp_0.srcPort, udp_sport);
    modify_field(udp_0.dstPort, udp_dport);
    add_header(udp_0);

    modify_field(scratch_metadata.vni, vni);
    modify_field(scratch_metadata.source_slot_id, source_slot_id);
    modify_field(scratch_metadata.destination_slot_id, destination_slot_id);
    modify_field(scratch_metadata.sg_id, sg_id1);
    modify_field(scratch_metadata.sg_id, sg_id2);
    modify_field(scratch_metadata.sg_id, sg_id3);
    modify_field(scratch_metadata.sg_id, sg_id4);
    modify_field(scratch_metadata.sg_id, sg_id5);
    modify_field(scratch_metadata.sg_id, sg_id6);
    modify_field(scratch_metadata.originator_physical_ip, originator_physical_ip);
}

@pragma stage 5
@pragma hbm_table
@pragma index_table
@pragma capi_bitfields_struct
table session_rewrite_encap {
    reads {
        control_metadata.session_rewrite_id: exact;
    }
    actions {
        session_rewrite_encap_l2;
        session_rewrite_encap_mplsoudp;
        session_rewrite_encap_geneve;
    }
    size : SESSION_TABLE_SIZE;
}

control session_info_lookup {
    if (control_metadata.flow_miss == FALSE) {
        apply(session_info);
        apply(session_rewrite);
        apply(session_rewrite_encap);
    }
}
