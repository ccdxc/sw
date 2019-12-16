action session_info(valid_flag, timestamp, config1_epoch, config1_idx,
                    config2_epoch, config2_idx, config_substrate_src_ip,
                    throttle_pps, throttle_bw, counterset1, counterset2,
                    histogram, pop_hdr_flag) {

    if (control_metadata.direction == RX_FROM_SWITCH) {
        if (ipv4_1.srcAddr != config_substrate_src_ip) {
            modify_field(control_metadata.flow_miss, TRUE);
        }
    }

    if (pop_hdr_flag == TRUE) {
        remove_header(ethernet_1);
        remove_header(ctag_1);

        if (control_metadata.direction == RX_FROM_SWITCH) {

            remove_header(ipv4_1);
            remove_header(ipv6_1);

            remove_header(udp_1);
            remove_header(gre_1);

            remove_header(mpls_label1_1);
            remove_header(mpls_label2_1);
        }
    }

    modify_field(scratch_metadata.flag, valid_flag);
    modify_field(scratch_metadata.timestamp, timestamp);

    modify_field(control_metadata.config1_epoch, config1_epoch);
    modify_field(control_metadata.config1_idx, config1_idx);

    modify_field(control_metadata.config2_epoch, config2_epoch);
    modify_field(control_metadata.config2_idx, config2_idx);

    modify_field(scratch_metadata.config_substrate_src_ip, config_substrate_src_ip);

    modify_field(control_metadata.throttle_pps, throttle_pps);
    modify_field(control_metadata.throttle_bw, throttle_bw);

    modify_field(p4i_to_p4e_header.counterset1, counterset1);
    modify_field(p4i_to_p4e_header.counterset2, counterset2);
    modify_field(p4i_to_p4e_header.histogram, histogram);

    modify_field(scratch_metadata.pop_hdr_flag, pop_hdr_flag);

}

action session_info_rewrite(valid_flag, user_pkt_rewrite_type, user_pkt_rewrite_ip, encap_type, dmac, smac, vlan,
                    ip_ttl, ip_saddr, ip_daddr,
                    udp_sport, udp_dport,
                    mpls1_label, mpls2_label) {

    if (ipv6_1.valid == TRUE) {
        if (user_pkt_rewrite_type == L3REWRITE_SNAT) {
            modify_field(ipv6_1.srcAddr, user_pkt_rewrite_ip);
        }
        if (user_pkt_rewrite_type == L3REWRITE_DNAT) {
            modify_field(ipv6_1.dstAddr, user_pkt_rewrite_ip);
        }
    }

    if (ipv4_1.valid == TRUE) {
        if (user_pkt_rewrite_type == L3REWRITE_SNAT) {
            modify_field(ipv4_1.srcAddr, 0x12345678);
        }
        if (user_pkt_rewrite_type == L3REWRITE_DNAT) {
            modify_field(ipv4_1.dstAddr, 0x12345678);
        }
    }

    modify_field(scratch_metadata.flag, valid_flag);
    modify_field(scratch_metadata.user_pkt_rewrite_type, user_pkt_rewrite_type);
    modify_field(scratch_metadata.user_pkt_rewrite_ip, user_pkt_rewrite_ip);

    modify_field(scratch_metadata.encap_type, encap_type);
    modify_field(scratch_metadata.dmac, dmac);
    modify_field(scratch_metadata.smac, smac);
    add_header(ethernet_0);

    if (vlan != 0) {
        modify_field(ethernet_0.etherType, ETHERTYPE_VLAN);
        modify_field(ctag_0.vid, vlan);
        modify_field(ctag_0.etherType, ETHERTYPE_IPV4);
        add_header(ctag_0);
    }
    else {
        modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);
    }

    if (encap_type != REWRITE_ENCAP_L2) {
        add_header(ipv4_0);
        modify_field(scratch_metadata.ip_ttl, ip_ttl);
        modify_field(scratch_metadata.ip_saddr, ip_saddr);
        modify_field(scratch_metadata.ip_daddr, ip_daddr);

        if (encap_type == REWRITE_ENCAP_MPLSOUDP) {
            modify_field(scratch_metadata.udp_sport, udp_sport);
            modify_field(scratch_metadata.udp_dport, udp_dport);
            add_header(udp_0);
            modify_field(scratch_metadata.mpls1_label, mpls1_label);
            modify_field(scratch_metadata.mpls2_label, mpls2_label);
            add_header(mpls_label1_0);
            add_header(mpls_label2_0);
        }

        if (encap_type == REWRITE_ENCAP_MPLSOGRE) {
            modify_field(gre_0.proto, ETHERTYPE_MPLS_UNICAST);
            add_header(gre_0);
            modify_field(scratch_metadata.mpls1_label, mpls1_label);
            modify_field(scratch_metadata.mpls2_label, mpls2_label);
            add_header(mpls_label1_0);
            add_header(mpls_label2_0);
        }

    }
}

@pragma stage 3
@pragma hbm_table
@pragma table_write
@pragma index_table
table session {
    reads {
        p4i_to_p4e_header.session_index   : exact;
    }
    actions {
        session_info;
    }
    size : SESSION_TABLE_SIZE;
}

@pragma stage 0
@pragma hbm_table
@pragma index_table
table session_rewrite {
    reads {
        p4i_to_p4e_header.session_index   : exact;
    }
    actions {
        session_info_rewrite;
    }
    size : SESSION_TABLE_SIZE;
}

control session_info_lookup {
    if (control_metadata.flow_miss == FALSE) {
        apply(session);
    }
}

control session_info_encap_lookup {
    if (control_metadata.flow_miss == FALSE) {
        apply(session_rewrite);
    }
}
