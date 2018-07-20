action rewrite(mac_da, mac_sa, ip_sa, ip_da, sport, dport,
               mac_sa_valid, mac_da_valid, ip_sa_valid, ip_da_valid,
               sport_valid, dport_valid) {
    if (rewrite_metadata.egress_tunnel_terminate == FALSE) {
        if (mac_da_valid == TRUE) {
            modify_field(ethernet_1.dstAddr, mac_da);
        }
        if (mac_sa_valid == TRUE) {
            modify_field(ethernet_1.srcAddr, mac_sa);
        }
        if (ipv4_1.valid == TRUE) {
            if (ip_sa_valid == TRUE) {
                modify_field(ipv4_1.srcAddr, ip_sa);
            }
            if (ip_da_valid == TRUE) {
                modify_field(ipv4_1.dstAddr, ip_da);
            }
        }
        if (ipv6_1.valid == TRUE) {
            if (ip_sa_valid == TRUE) {
                modify_field(ipv6_1.srcAddr, ip_sa);
            }
            if (ip_da_valid == TRUE) {
                modify_field(ipv6_1.dstAddr, ip_da);
            }
        }
        if (udp_1.valid == TRUE) {
            if (sport_valid == TRUE) {
                modify_field(udp_1.srcPort, sport);
            }
            if (dport_valid == TRUE) {
                modify_field(udp_1.dstPort, dport);
            }
        }
    } else {
        if (mac_da_valid == TRUE) {
            modify_field(ethernet_2.dstAddr, mac_da);
        }
        if (mac_sa_valid == TRUE) {
            modify_field(ethernet_2.srcAddr, mac_sa);
        }
        if (ipv4_2.valid == TRUE) {
            if (ip_sa_valid == TRUE) {
                modify_field(ipv4_2.srcAddr, ip_sa);
            }
            if (ip_da_valid == TRUE) {
                modify_field(ipv4_2.dstAddr, ip_da);
            }
        }
        if (ipv6_2.valid == TRUE) {
            if (ip_sa_valid == TRUE) {
                modify_field(ipv6_2.srcAddr, ip_sa);
            }
            if (ip_da_valid == TRUE) {
                modify_field(ipv6_2.dstAddr, ip_da);
            }
        }
        if (udp_2.valid == TRUE) {
            if (sport_valid == TRUE) {
                modify_field(udp_2.srcPort, sport);
            }
            if (dport_valid == TRUE) {
                modify_field(udp_2.dstPort, dport);
            }
        }
    }
    if (tcp.valid == TRUE) {
        if (sport_valid == TRUE) {
            modify_field(tcp.srcPort, sport);
        }
        if (dport_valid == TRUE) {
            modify_field(tcp.dstPort, dport);
        }
    }

    modify_field(scratch_metadata.flag, mac_sa_valid);
    modify_field(scratch_metadata.flag, mac_da_valid);
    modify_field(scratch_metadata.flag, ip_sa_valid);
    modify_field(scratch_metadata.flag, ip_da_valid);
    modify_field(scratch_metadata.flag, sport_valid);
    modify_field(scratch_metadata.flag, dport_valid);
}

@pragma hbm_table
@pragma stage 2
table rewrite {
    reads {
        capri_i2e_metadata.nexthop_index  : exact;
    }
    actions {
        rewrite;
        nop;
    }
    size : REWRITE_TABLE_SIZE;
}

action tunnel_decap() {
    remove_header(ethernet_1);
    remove_header(ctag_1);
    remove_header(ipv4_1);
    remove_header(ipv6_1);
    remove_header(udp_1);
    remove_header(vxlan_1);
}

@pragma stage 2
table tunnel_decap {
    actions {
        tunnel_decap;
    }
}

action encap_vxlan(mac_da, mac_sa, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    add_header(ethernet_0);
    modify_field(ethernet_0.dstAddr, mac_da);
    modify_field(ethernet_0.srcAddr, mac_sa);
    if (vlan_valid == TRUE) {
        modify_field(ethernet_0.etherType, ETHERTYPE_CTAG);
        add_header(ctag_0);
        modify_field(ctag_0.vid, vlan_id);
    }

    if (ip_type == IPTYPE_IPV4) {
        if (vlan_valid == TRUE) {
            modify_field(ctag_0.etherType, ETHERTYPE_IPV4);
        } else{
            modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);
        }
        add_header(ipv4_0);
        modify_field(ipv4_0.srcAddr, ip_sa);
        modify_field(ipv4_0.dstAddr, ip_da);
    } else {
        if (vlan_valid == TRUE) {
            modify_field(ctag_0.etherType, ETHERTYPE_IPV6);
        } else{
            modify_field(ethernet_0.etherType, ETHERTYPE_IPV6);
        }
        add_header(ipv6_0);
        modify_field(ipv6_0.srcAddr, ip_sa);
        modify_field(ipv6_0.dstAddr, ip_da);
    }

    add_header(udp_0);
    add_header(vxlan_0);

    modify_field(scratch_metadata.flag, ip_type);
    modify_field(scratch_metadata.flag, vlan_valid);
}

@pragma stage 4
table tunnel_rewrite {
    reads {
        rewrite_metadata.tunnel_rewrite_index   : exact;
    }
    actions {
        nop;
        encap_vxlan;
    }
    size : TUNNEL_REWRITE_TABLE_SIZE;
}

control rewrite {
    if (rewrite_metadata.egress_tunnel_terminate == TRUE) {
        apply(tunnel_decap);
    }
    apply(rewrite);
    apply(tunnel_rewrite);
}
