/******************************************************************************
 * Overlay Nexthop Group
 *****************************************************************************/
action overlay_nexthop_group_info(nexthop_id, nexthop_type, num_nexthops) {
    modify_field(scratch_metadata.num_nexthops, num_nexthops);
    if (num_nexthops == 0) {
        modify_field(scratch_metadata.nexthop_id, nexthop_id);
    } else {
        modify_field(scratch_metadata.nexthop_id, nexthop_id +
            (p4e_i2e.entropy_hash % scratch_metadata.num_nexthops));
    }
    modify_field(rewrite_metadata.nexthop_type, nexthop_type);
    modify_field(txdma_to_p4e.nexthop_id, scratch_metadata.nexthop_id);
}

@pragma stage 2
table overlay_nexthop_group {
    reads {
        txdma_to_p4e.nexthop_id : exact;
    }
    actions {
        overlay_nexthop_group_info;
    }
    size : NEXTHOP_GROUP_TABLE_SIZE;
}

/******************************************************************************
 * Underlay Nexthop Group
 *****************************************************************************/
action underlay_nexthop_group_info(nexthop_id, num_nexthops) {
    modify_field(scratch_metadata.num_nexthops, num_nexthops);
    if (num_nexthops == 0) {
        modify_field(scratch_metadata.nexthop_id, nexthop_id);
    } else {
        modify_field(scratch_metadata.nexthop_id, nexthop_id +
            (p4e_i2e.entropy_hash % scratch_metadata.num_nexthops));
    }
    modify_field(txdma_to_p4e.nexthop_id, scratch_metadata.nexthop_id);
}

@pragma stage 3
table underlay_nexthop_group {
    reads {
        txdma_to_p4e.nexthop_id : exact;
    }
    actions {
        underlay_nexthop_group_info;
    }
    size : NEXTHOP_GROUP_TABLE_SIZE;
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

action ipv4_vxlan_encap(vni, dipo, dmac, smac) {
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
    modify_field(ipv4_0.dstAddr, dipo);
    modify_field(ipv4_0.srcAddr, rewrite_metadata.device_ipv4_addr);

    modify_field(udp_0.srcPort, (0xC000 | p4e_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_VXLAN);
    subtract(udp_0.len, scratch_metadata.ip_totallen, 20);

    modify_field(vxlan_0.flags, 0x8);
    modify_field(vxlan_0.vni, rewrite_metadata.vni);
    modify_field(vxlan_0.reserved, 0);
    modify_field(vxlan_0.reserved2, 0);

    add(capri_p4_intrinsic.packet_len, scratch_metadata.ip_totallen, 14);
}

action ipv6_vxlan_encap(vni, dipo, dmac, smac) {
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
    modify_field(ipv6_0.dstAddr, dipo);
    modify_field(ipv6_0.srcAddr, rewrite_metadata.device_ipv6_addr);

    modify_field(udp_0.srcPort, (0xC000 | p4e_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_VXLAN);
    modify_field(udp_0.len, scratch_metadata.ip_totallen);

    modify_field(vxlan_0.flags, 0x8);
    modify_field(vxlan_0.vni, rewrite_metadata.vni);
    modify_field(vxlan_0.reserved, 0);
    modify_field(vxlan_0.reserved2, 0);

    add(capri_p4_intrinsic.packet_len, scratch_metadata.ip_totallen, (40 + 14));
}

action nexthop_info(lif, qtype, qid, port, vni, ip_type, dipo, dmaco, smaco,
                    dmaci) {
    if (capri_intrinsic.tm_oq != TM_P4_RECIRC_QUEUE) {
        modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    } else {
        modify_field(capri_intrinsic.tm_oq, capri_intrinsic.tm_iq);
    }
    if (txdma_to_p4e.nexthop_id == 0) {
        egress_drop(P4E_DROP_NEXTHOP_INVALID);
    }
    if (control_metadata.rx_packet == FALSE) {
        if (TX_REWRITE(rewrite_metadata.flags, DMAC, FROM_MAPPING)) {
            modify_field(ethernet_1.dstAddr, rewrite_metadata.dmaci);
        } else {
            if (TX_REWRITE(rewrite_metadata.flags, DMAC, FROM_NEXTHOP)) {
                modify_field(ethernet_1.dstAddr, dmaci);
            }
        }
        if (TX_REWRITE(rewrite_metadata.flags, SMAC, FROM_VRMAC)) {
            modify_field(ethernet_1.srcAddr, rewrite_metadata.vrmac);
        }
        if (TX_REWRITE(rewrite_metadata.flags, ENCAP, VLAN)) {
            encap_vlan(vni);
        } else {
            if (TX_REWRITE(rewrite_metadata.flags, ENCAP, VXLAN)) {
                modify_field(scratch_metadata.flag, ip_type);
                if (ip_type == IPTYPE_IPV4) {
                    ipv4_vxlan_encap(vni, dipo, dmaco, smaco);
                } else {
                    ipv6_vxlan_encap(vni, dipo, dmaco, smaco);
                }
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
            encap_vlan(vni);
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
        txdma_to_p4e.nexthop_id : exact;
    }
    actions {
        nexthop_info;
    }
    size : NEXTHOP_TABLE_SIZE;
}

control nexthops {
    if (rewrite_metadata.nexthop_type == NEXTHOP_TYPE_OVERLAY) {
        apply(overlay_nexthop_group);
    }
    if (rewrite_metadata.nexthop_type == NEXTHOP_TYPE_UNDERLAY) {
        apply(underlay_nexthop_group);
    }
    apply(nexthop);
}
