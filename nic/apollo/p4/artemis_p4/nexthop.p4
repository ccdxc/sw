/******************************************************************************
 * Nexthop and tunnel rewrite                                                 *
 *****************************************************************************/
action ipv4_vxlan_encap(vni, dipo, dmac) {
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
    modify_field(ethernet_0.srcAddr, rewrite_metadata.device_mac);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);

    modify_field(ipv4_0.version, 4);
    modify_field(ipv4_0.ihl, 5);
    modify_field(ipv4_0.totalLen, scratch_metadata.ip_totallen);
    modify_field(ipv4_0.ttl, 64);
    modify_field(ipv4_0.protocol, IP_PROTO_UDP);
    if (TX_REWRITE(rewrite_metadata.flags, SRC_IP_OUTER, FROM_XLATE)) {
        modify_field(ipv4_0.dstAddr, dipo);
        modify_field(ipv4_0.srcAddr, rewrite_metadata.encap_src_ip);
    } else {
        modify_field(ipv4_0.dstAddr, ipv4_1.dstAddr);
        modify_field(ipv4_0.srcAddr, dipo);
    }

    modify_field(udp_0.srcPort, (0xC000 | p4e_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_VXLAN);
    subtract(udp_0.len, scratch_metadata.ip_totallen, 20);

    modify_field(vxlan_0.flags, 0x8);
    modify_field(vxlan_0.vni, vni);
    modify_field(vxlan_0.reserved, 0);
    modify_field(vxlan_0.reserved2, 0);

    add(capri_p4_intrinsic.packet_len, scratch_metadata.ip_totallen, 14);
}

action ipv6_vxlan_encap(vni, dipo, dmac) {
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
    modify_field(ethernet_0.srcAddr, rewrite_metadata.device_mac);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV6);

    modify_field(ipv6_0.version, 6);
    modify_field(ipv6_0.payloadLen, scratch_metadata.ip_totallen);
    modify_field(ipv6_0.hopLimit, 64);
    modify_field(ipv6_0.nextHdr, IP_PROTO_UDP);
    modify_field(ipv6_0.dstAddr, dipo);
    modify_field(ipv6_0.srcAddr, rewrite_metadata.encap_src_ip);

    modify_field(udp_0.srcPort, (0xC000 | p4e_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_VXLAN);
    modify_field(udp_0.len, scratch_metadata.ip_totallen);

    modify_field(vxlan_0.flags, 0x8);
    modify_field(vxlan_0.vni, vni);
    modify_field(vxlan_0.reserved, 0);
    modify_field(vxlan_0.reserved2, 0);

    add(capri_p4_intrinsic.packet_len, scratch_metadata.ip_totallen, (40 + 14));
}

action nexthop_info(port, vni, ip_type, dipo, dmaco, dmaci) {
    if (rewrite_metadata.nexthop_idx == 0) {
        egress_drop(P4E_DROP_INVALID_NEXTHOP);
    }
    modify_field(capri_intrinsic.tm_oport, port);
    if (TX_REWRITE(rewrite_metadata.flags, DMAC, FROM_NEXTHOP)) {
        modify_field(ethernet_1.dstAddr, dmaci);
    }
    if (TX_REWRITE(rewrite_metadata.flags, ENCAP, NONE)) {
        if (vni == 0) {
            if (ctag_1.valid == TRUE) {
                modify_field(ethernet_1.etherType, ctag_1.etherType);
                remove_header(ctag_1);
                subtract(capri_p4_intrinsic.packet_len,
                         capri_p4_intrinsic.packet_len, 4);
            }
        } else {
            if (ctag_1.valid == FALSE) {
                add_header(ctag_1);
                modify_field(ctag_1.etherType, ethernet_1.etherType);
                modify_field(ethernet_1.etherType, ETHERTYPE_VLAN);
                add(capri_p4_intrinsic.packet_len,
                    capri_p4_intrinsic.packet_len, 4);
            }
            modify_field(ctag_1.pcp, 0);
            modify_field(ctag_1.dei, 0);
            modify_field(ctag_1.vid, vni);
        }
    }
    if (TX_REWRITE(rewrite_metadata.flags, ENCAP, VXLAN)) {
        modify_field(scratch_metadata.flag, ip_type);
        if (ip_type == IPTYPE_IPV4) {
            ipv4_vxlan_encap(vni, dipo, dmaco);
        } else {
            ipv6_vxlan_encap(vni, dipo, dmaco);
        }
    }
}

@pragma stage 3
@pragma hbm_table
@pragma index_table
table nexthop {
    reads {
        rewrite_metadata.nexthop_idx    : exact;
    }
    actions {
        nexthop_info;
    }
    size : NEXTHOP_TABLE_SIZE;
}

control nexthop {
    if (control_metadata.direction == TX_FROM_HOST) {
        apply(nexthop);
    }
}
