/******************************************************************************
 * ECMP
 *****************************************************************************/
action ecmp_info(nexthop_type, num_nexthops, tunnel_id4, tunnel_id3,
                 tunnel_id2, tunnel_id1, nexthop_base) {
    modify_field(scratch_metadata.num_nexthops, num_nexthops);
    if (scratch_metadata.num_nexthops == 0) {
        egress_drop(P4E_DROP_NEXTHOP_INVALID);
        // return;
    }

    if (nexthop_type == NEXTHOP_TYPE_TUNNEL) {
        if (scratch_metadata.nexthop_id == 1) {
            modify_field(scratch_metadata.nexthop_id, tunnel_id1);
        }
        if (scratch_metadata.nexthop_id == 2) {
            modify_field(scratch_metadata.nexthop_id, tunnel_id2);
        }
        if (scratch_metadata.nexthop_id == 3) {
            modify_field(scratch_metadata.nexthop_id, tunnel_id3);
        }
        if (scratch_metadata.nexthop_id == 4) {
            modify_field(scratch_metadata.nexthop_id, tunnel_id4);
        }
    } else {
        modify_field(scratch_metadata.nexthop_id, nexthop_base +
                     (p4e_i2e.entropy_hash % scratch_metadata.num_nexthops));
    }
    modify_field(rewrite_metadata.nexthop_type, nexthop_type);
    modify_field(p4e_i2e.nexthop_id, scratch_metadata.nexthop_id);
}

@pragma stage 2
@pragma index_table
table ecmp {
    reads {
        p4e_i2e.nexthop_id  : exact;
    }
    actions {
        ecmp_info;
    }
    size : ECMP_TABLE_SIZE;
}

/******************************************************************************
 * Tunnel
 *****************************************************************************/
action tunnel_info(nexthop_base, num_nexthops, vni, ip_type, dipo, dmaci) {
    modify_field(scratch_metadata.num_nexthops, num_nexthops);
    if (scratch_metadata.num_nexthops == 0) {
        egress_drop(P4E_DROP_NEXTHOP_INVALID);
        // return;
    }

    modify_field(scratch_metadata.nexthop_id, nexthop_base +
                 (p4e_i2e.entropy_hash % scratch_metadata.num_nexthops));
    modify_field(p4e_i2e.nexthop_id, scratch_metadata.nexthop_id);

    modify_field(rewrite_metadata.ip_type, ip_type);
    if (ip_type == IPTYPE_IPV4) {
        modify_field(ipv4_0.dstAddr, dipo);
    } else {
        modify_field(ipv6_0.dstAddr, dipo);
    }
    modify_field(rewrite_metadata.tunnel_dmaci, dmaci);
    modify_field(rewrite_metadata.tunnel_vni, vni);
}

@pragma stage 3
@pragma index_table
table tunnel {
    reads {
        p4e_i2e.nexthop_id  : exact;
    }
    actions {
        tunnel_info;
    }
    size : TUNNEL_TABLE_SIZE;
}

/******************************************************************************
 * Nexthop
 *****************************************************************************/
action encap_vlan(vlan) {
    if (ctag_1.valid == FALSE) {
        add_header(ctag_1);
        modify_field(ctag_1.etherType, ethernet_1.etherType);
        modify_field(ethernet_1.etherType, ETHERTYPE_VLAN);
        add(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len, 4);
    }
    modify_field(ctag_1.pcp, 0);
    modify_field(ctag_1.dei, 0);
    modify_field(ctag_1.vid, vlan);
}

action ipv4_vxlan_encap(dmac, smac) {
    // remove headers
    remove_header(ctag_1);

    // add tunnel headers
    add_header(ethernet_0);
    add_header(ipv4_0);
    add_header(udp_0);
    add_header(vxlan_0);

    modify_field(scratch_metadata.ip_totallen, capri_p4_intrinsic.packet_len);
    if (ctag_1.valid == TRUE) {
        subtract_from_field(scratch_metadata.ip_totallen, 4);
        modify_field(ethernet_1.etherType, ctag_1.etherType);
    }
    // account for new headers that are added
    // 8 bytes of UDP header, 8 bytes of vxlan header, 20 bytes of IP header
    add_to_field(scratch_metadata.ip_totallen, 20 + 8 + 8);

    modify_field(ethernet_0.dstAddr, dmac);
    modify_field(ethernet_0.srcAddr, smac);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);

    modify_field(ipv4_0.version, 4);
    modify_field(ipv4_0.ihl, 5);
    modify_field(ipv4_0.totalLen, scratch_metadata.ip_totallen);
    modify_field(ipv4_0.ttl, 64);
    modify_field(ipv4_0.protocol, IP_PROTO_UDP);
    modify_field(ipv4_0.srcAddr, rewrite_metadata.device_ipv4_addr);

    modify_field(udp_0.srcPort, (0xC000 | p4e_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_VXLAN);
    subtract(udp_0.len, scratch_metadata.ip_totallen, 20);

    modify_field(vxlan_0.flags, 0x8);
    if (TX_REWRITE(rewrite_metadata.flags, VNI, FROM_TUNNEL)) {
        modify_field(vxlan_0.vni, rewrite_metadata.tunnel_vni);
    } else {
        modify_field(vxlan_0.vni, rewrite_metadata.vni);
    }
    modify_field(vxlan_0.reserved, 0);
    modify_field(vxlan_0.reserved2, 0);

    add(capri_p4_intrinsic.packet_len, scratch_metadata.ip_totallen, 14);
}

action ipv6_vxlan_encap(dmac, smac) {
    // remove headers
    remove_header(ctag_1);

    // add tunnel headers
    add_header(ethernet_0);
    add_header(ipv6_0);
    add_header(udp_0);
    add_header(vxlan_0);

    modify_field(scratch_metadata.ip_totallen, capri_p4_intrinsic.packet_len);
    if (ctag_1.valid == TRUE) {
        subtract_from_field(scratch_metadata.ip_totallen, 4);
        modify_field(ethernet_1.etherType, ctag_1.etherType);
    }
    // account for new headers that are added
    // 8 bytes of UDP header, 8 bytes of vxlan header
    add_to_field(scratch_metadata.ip_totallen, 8 + 8);

    modify_field(ethernet_0.dstAddr, dmac);
    modify_field(ethernet_0.srcAddr, smac);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV6);

    modify_field(ipv6_0.version, 6);
    modify_field(ipv6_0.payloadLen, scratch_metadata.ip_totallen);
    modify_field(ipv6_0.hopLimit, 64);
    modify_field(ipv6_0.nextHdr, IP_PROTO_UDP);
    modify_field(ipv6_0.srcAddr, rewrite_metadata.device_ipv6_addr);

    modify_field(udp_0.srcPort, (0xC000 | p4e_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_VXLAN);
    modify_field(udp_0.len, scratch_metadata.ip_totallen);

    modify_field(vxlan_0.flags, 0x8);
    if (TX_REWRITE(rewrite_metadata.flags, VNI, FROM_TUNNEL)) {
        modify_field(vxlan_0.vni, rewrite_metadata.tunnel_vni);
    } else {
        modify_field(vxlan_0.vni, rewrite_metadata.vni);
    }
    modify_field(vxlan_0.reserved, 0);
    modify_field(vxlan_0.reserved2, 0);

    add(capri_p4_intrinsic.packet_len, scratch_metadata.ip_totallen, (40 + 14));
}

action nexthop_info(lif, qtype, qid, port, vlan, dmaco, smaco, dmaci,
                    tunnel2_id) {
    if (p4e_i2e.nexthop_id == 0) {
        egress_drop(P4E_DROP_NEXTHOP_INVALID);
    }

    if (tunnel2_id != 0) {
        modify_field(control_metadata.apply_tunnel2, TRUE);
        modify_field(rewrite_metadata.tunnel2_id, tunnel2_id);
        modify_field(rewrite_metadata.tunnel2_vni, vlan);
    }

    if (control_metadata.rx_packet == FALSE) {
        if (TX_REWRITE(rewrite_metadata.flags, DMAC, FROM_MAPPING)) {
            modify_field(ethernet_1.dstAddr, rewrite_metadata.dmaci);
        } else {
            if (TX_REWRITE(rewrite_metadata.flags, DMAC, FROM_NEXTHOP)) {
                modify_field(ethernet_1.dstAddr, dmaci);
            } else {
                if (TX_REWRITE(rewrite_metadata.flags, DMAC, FROM_TUNNEL)) {
                    modify_field(ethernet_1.dstAddr,
                                 rewrite_metadata.tunnel_dmaci);
                }
            }
        }
        if (TX_REWRITE(rewrite_metadata.flags, SMAC, FROM_VRMAC)) {
            modify_field(ethernet_1.srcAddr, rewrite_metadata.vrmac);
        }
        if (TX_REWRITE(rewrite_metadata.flags, ENCAP, VXLAN)) {
            if (rewrite_metadata.ip_type == IPTYPE_IPV4) {
                ipv4_vxlan_encap(dmaco, smaco);
            } else {
                ipv6_vxlan_encap(dmaco, smaco);
            }
        } else {
            if (TX_REWRITE(rewrite_metadata.flags, ENCAP, VLAN)) {
                encap_vlan(vlan);
            }
        }
    } else {
        if (RX_REWRITE(rewrite_metadata.flags, DMAC, FROM_MAPPING)) {
            modify_field(ethernet_1.dstAddr, rewrite_metadata.dmaci);
        } else {
            if (RX_REWRITE(rewrite_metadata.flags, DMAC, FROM_NEXTHOP)) {
                modify_field(ethernet_1.dstAddr, dmaci);
            }
        }
        if (RX_REWRITE(rewrite_metadata.flags, SMAC, FROM_VRMAC)) {
            modify_field(ethernet_1.srcAddr, rewrite_metadata.vrmac);
        }
        if (RX_REWRITE(rewrite_metadata.flags, ENCAP, VLAN)) {
            encap_vlan(vlan);
        }
    }
    modify_field(capri_intrinsic.tm_oport, port);
    if (port == TM_PORT_DMA) {
        modify_field(capri_intrinsic.lif, lif);
        modify_field(capri_rxdma_intrinsic.qtype, qtype);
        modify_field(capri_rxdma_intrinsic.qid, qid);
    }
    modify_field(scratch_metadata.mac, smaco);
}

@pragma stage 4
@pragma index_table
@pragma hbm_table
table nexthop {
    reads {
        p4e_i2e.nexthop_id  : exact;
    }
    actions {
        nexthop_info;
    }
    size : NEXTHOP_TABLE_SIZE;
}

/******************************************************************************
 * Tunnel 2
 *****************************************************************************/
action tunnel2_ipv4_encap(vlan, dmac, smac, dipo) {
    // remove inner ethernet header
    remove_header(ethernet_0);

    // add tunnel headers
    add_header(ethernet_00);
    add_header(ipv4_00);
    add_header(udp_00);

    modify_field(scratch_metadata.ip_totallen, capri_p4_intrinsic.packet_len);

    // account for new headers that are added
    // 8 bytes of UDP header, 4 bytes of MPLS header, 20 bytes of IP header
    // -14 bytes of inner Ethernet header
    add_to_field(scratch_metadata.ip_totallen, 20 + 8 + 4 - 14);

    modify_field(ethernet_00.dstAddr, dmac);
    modify_field(ethernet_00.srcAddr, smac);

    modify_field(ipv4_00.version, 4);
    modify_field(ipv4_00.ihl, 5);
    modify_field(ipv4_00.totalLen, scratch_metadata.ip_totallen);
    modify_field(ipv4_00.ttl, 64);
    modify_field(ipv4_00.protocol, IP_PROTO_UDP);
    modify_field(ipv4_00.srcAddr, rewrite_metadata.device_ipv4_addr);
    modify_field(ipv4_00.dstAddr, dipo);

    modify_field(udp_00.srcPort, (0xC000 | p4e_i2e.entropy_hash));
    subtract(udp_00.len, scratch_metadata.ip_totallen, 20);

    modify_field(udp_00.dstPort, UDP_PORT_MPLS);
    add_header(mpls_00);
    modify_field(mpls_00.label, rewrite_metadata.tunnel2_vni);
    modify_field(mpls_00.exp, 0);
    modify_field(mpls_00.bos, 1);
    modify_field(mpls_00.ttl, 64);

    if (vlan == 0) {
        modify_field(ethernet_00.etherType, ETHERTYPE_IPV4);
        add(capri_p4_intrinsic.packet_len, scratch_metadata.ip_totallen, 14);
    } else {
        modify_field(ethernet_00.etherType, ETHERTYPE_VLAN);
        add_header(ctag_00);
        modify_field(ctag_00.pcp, 0);
        modify_field(ctag_00.dei, 0);
        modify_field(ctag_00.vid, vlan);
        modify_field(ctag_00.etherType, ETHERTYPE_IPV4);
        add(capri_p4_intrinsic.packet_len, scratch_metadata.ip_totallen, 18);
    }
}

action tunnel2_ipv6_encap(vlan, dmac, smac, dipo) {
    // remove inner ethernet header
    remove_header(ethernet_0);

    // add tunnel headers
    add_header(ethernet_00);
    add_header(ipv6_00);
    add_header(udp_00);

    modify_field(scratch_metadata.ip_totallen, capri_p4_intrinsic.packet_len);

    // account for new headers that are added
    // 8 bytes of UDP header, 4 bytes of MPLS header,
    // -14 bytes of inner Ethernet header
    add_to_field(scratch_metadata.ip_totallen, 8 + 4 - 14);

    modify_field(ethernet_00.dstAddr, dmac);
    modify_field(ethernet_00.srcAddr, smac);

    modify_field(ipv6_00.version, 6);
    modify_field(ipv6_00.payloadLen, scratch_metadata.ip_totallen);
    modify_field(ipv6_00.hopLimit, 64);
    modify_field(ipv6_00.nextHdr, IP_PROTO_UDP);
    modify_field(ipv6_00.srcAddr, rewrite_metadata.device_ipv6_addr);
    modify_field(ipv6_00.dstAddr, dipo);

    modify_field(udp_00.srcPort, (0xC000 | p4e_i2e.entropy_hash));
    modify_field(udp_00.len, scratch_metadata.ip_totallen);

    modify_field(udp_00.dstPort, UDP_PORT_MPLS);
    add_header(mpls_00);
    modify_field(mpls_00.label, rewrite_metadata.tunnel2_vni);
    modify_field(mpls_00.exp, 0);
    modify_field(mpls_00.bos, 1);
    modify_field(mpls_00.ttl, 64);

    if (vlan == 0) {
        modify_field(ethernet_00.etherType, ETHERTYPE_IPV6);
        add(capri_p4_intrinsic.packet_len,
            scratch_metadata.ip_totallen, (40 + 14));
    } else {
        modify_field(ethernet_00.etherType, ETHERTYPE_VLAN);
        add_header(ctag_00);
        modify_field(ctag_00.pcp, 0);
        modify_field(ctag_00.dei, 0);
        modify_field(ctag_00.vid, vlan);
        modify_field(ctag_00.etherType, ETHERTYPE_IPV6);
        add(capri_p4_intrinsic.packet_len,
            scratch_metadata.ip_totallen, (40 + 18));
    }
}

action tunnel2_info(vlan, dmaco, smaco, dipo, ip_type) {
    modify_field(scratch_metadata.flag, ip_type);
    if (ip_type == IPTYPE_IPV4) {
        tunnel2_ipv4_encap(vlan, dmaco, smaco, dipo);
    } else {
        tunnel2_ipv6_encap(vlan, dmaco, smaco, dipo);
    }
}

@pragma stage 5
table tunnel2 {
    reads {
        rewrite_metadata.tunnel2_id : exact;
    }
    actions {
        tunnel2_info;
    }
    size : TUNNEL2_TABLE_SIZE;
}

control nexthops {
    if (rewrite_metadata.nexthop_type == NEXTHOP_TYPE_ECMP) {
        apply(ecmp);
    }
    if (rewrite_metadata.nexthop_type == NEXTHOP_TYPE_TUNNEL) {
        apply(tunnel);
    }
    apply(nexthop);
    if (control_metadata.apply_tunnel2 == TRUE) {
        apply(tunnel2);
    }
}
