/******************************************************************************
 * Nexthop Group
 *****************************************************************************/
action nexthop_group_info(nexthop_index, num_nexthops) {
    if (nexthop_index == 0) {
        modify_field(rewrite_metadata.nexthop_index,
                     txdma_to_p4e_header.nexthop_group_index);
        // return
    }

    modify_field(scratch_metadata.num_nexthops, num_nexthops);
    if (num_nexthops == 0) {
        modify_field(scratch_metadata.nexthop_index, nexthop_index);
    } else {
        modify_field(scratch_metadata.nexthop_index, nexthop_index +
            (p4e_apollo_i2e.entropy_hash % scratch_metadata.num_nexthops));
    }
    modify_field(rewrite_metadata.nexthop_index, scratch_metadata.nexthop_index);
}

@pragma stage 2
table nexthop_group {
    reads {
        txdma_to_p4e_header.nexthop_group_index : exact;
    }
    actions {
        nexthop_group_info;
    }
    size : NEXTHOP_GROUP_TABLE_SIZE;
}

/******************************************************************************
 * Nexthop
 *****************************************************************************/
action nexthop_info(tep_index, snat_required, encap_type,
                    dst_slot_id, traffic_class) {
    if (rewrite_metadata.nexthop_index == 0) {
        modify_field(control_metadata.p4e_drop_reason,
                     1 << P4E_DROP_INVALID_NEXTHOP);
        drop_packet();
    }

    modify_field(rewrite_metadata.tep_index, tep_index);
    modify_field(nat_metadata.snat_required, snat_required);
    modify_field(rewrite_metadata.encap_type, encap_type);
    if (rewrite_metadata.dst_slot_id_valid == 0) {
        modify_field(rewrite_metadata.dst_slot_id, dst_slot_id);
    }
    modify_field(policer_metadata.traffic_class, traffic_class);
    // mytep_ip is a table constant
    modify_field(rewrite_metadata.mytep_ip, scratch_metadata.mytep_ip);
}

@pragma stage 3
table nexthop {
    reads {
        rewrite_metadata.nexthop_index  : exact;
    }
    actions {
        nexthop_info;
    }
    size : NEXTHOP_TABLE_SIZE;
}

/******************************************************************************
 * Tunnel rewrite
 *****************************************************************************/
action gre_tep(dipo, dmac) {
    // remove headers
    remove_header(ethernet_1);
    remove_header(ctag_1);

    // add tunnel headers
    add_header(ethernet_0);
    add_header(ipv4_0);
    add_header(gre_0);
    add_header(mpls_dst_0);
    if (rewrite_metadata.encap_type == VNIC_ENCAP) {
        add_header(mpls_src_0);
    }

    // subtract ethernet header size
    subtract(scratch_metadata.ip_totallen, capri_p4_intrinsic.packet_len, 14);
    if (ctag_1.valid == TRUE) {
        subtract_from_field(scratch_metadata.ip_totallen, 4);
    }
    // account for new headers that are added
    // 4 bytes of GRE header, 4 bytes of mpls header, 20 bytes of IP header
    add_to_field(scratch_metadata.ip_totallen, 20 + 4 + 4);

    modify_field(ethernet_0.dstAddr, dmac);
    // mytep_macsa is a table constant
    modify_field(ethernet_0.srcAddr, scratch_metadata.mytep_macsa);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);

    modify_field(ipv4_0.version, 4);
    modify_field(ipv4_0.ihl, 5);
    modify_field(ipv4_0.totalLen, scratch_metadata.ip_totallen);
    modify_field(ipv4_0.ttl, 64);
    modify_field(ipv4_0.protocol, IP_PROTO_GRE);
    modify_field(ipv4_0.dstAddr, dipo);
    modify_field(ipv4_0.srcAddr, rewrite_metadata.mytep_ip);

    modify_field(gre_0.proto, ETHERTYPE_MPLS_UNICAST);

    modify_field(mpls_dst_0.label, rewrite_metadata.dst_slot_id);
    if (rewrite_metadata.encap_type == VNIC_ENCAP) {
        modify_field(mpls_src_0.label, rewrite_metadata.src_slot_id);
        modify_field(mpls_src_0.bos, 1);
        add_to_field(scratch_metadata.ip_totallen, 4);
    } else {
        modify_field(mpls_dst_0.bos, 1);
    }

    if (rewrite_metadata.mytep_ip == dipo) {
        modify_field(control_metadata.local_switching, 1);
    }
}

action mpls_udp_tep(dipo, dmac) {
    // remove headers
    remove_header(ethernet_1);
    remove_header(ctag_1);

    // add tunnel headers
    add_header(ethernet_0);
    add_header(ipv4_0);
    add_header(udp_0);
    add_header(mpls_dst_0);
    if (rewrite_metadata.encap_type == VNIC_ENCAP) {
        add_header(mpls_src_0);
    }

    // subtract ethernet header size
    subtract(scratch_metadata.ip_totallen, capri_p4_intrinsic.packet_len, 14);
    if (ctag_1.valid == TRUE) {
        subtract_from_field(scratch_metadata.ip_totallen, 4);
    }
    // account for new headers that are added
    // 8 bytes of UDP header, 4 bytes of mpls header, 20 bytes of IP header
    add_to_field(scratch_metadata.ip_totallen, 20 + 8 + 4);
    if (rewrite_metadata.encap_type == VNIC_ENCAP) {
        add_to_field(scratch_metadata.ip_totallen, 4);
    }

    modify_field(ethernet_0.dstAddr, dmac);
    // mytep_macsa is a table constant
    modify_field(ethernet_0.srcAddr, scratch_metadata.mytep_macsa);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);

    modify_field(ipv4_0.version, 4);
    modify_field(ipv4_0.ihl, 5);
    modify_field(ipv4_0.totalLen, scratch_metadata.ip_totallen);
    modify_field(ipv4_0.ttl, 64);
    modify_field(ipv4_0.protocol, IP_PROTO_UDP);
    modify_field(ipv4_0.dstAddr, dipo);
    modify_field(ipv4_0.srcAddr, rewrite_metadata.mytep_ip);

    modify_field(udp_0.srcPort, (0xC000 | p4e_apollo_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_MPLS);
    subtract(udp_0.len, scratch_metadata.ip_totallen, 20);

    modify_field(mpls_dst_0.label, rewrite_metadata.dst_slot_id);
    if (rewrite_metadata.encap_type == VNIC_ENCAP) {
        modify_field(mpls_src_0.label, rewrite_metadata.src_slot_id);
        modify_field(mpls_src_0.bos, 1);
    } else {
        modify_field(mpls_dst_0.bos, 1);
    }

    if (rewrite_metadata.mytep_ip == dipo) {
        modify_field(control_metadata.local_switching, 1);
    }
}

action ipv4_vxlan_tep(dipo, dmac) {
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
    // mytep_macsa is a table constant
    modify_field(ethernet_0.srcAddr, scratch_metadata.mytep_macsa);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);

    modify_field(ipv4_0.version, 4);
    modify_field(ipv4_0.ihl, 5);
    modify_field(ipv4_0.totalLen, scratch_metadata.ip_totallen);
    modify_field(ipv4_0.ttl, 64);
    modify_field(ipv4_0.protocol, IP_PROTO_UDP);
    modify_field(ipv4_0.dstAddr, dipo);
    modify_field(ipv4_0.srcAddr, rewrite_metadata.mytep_ip);

    modify_field(udp_0.srcPort, (0xC000 | p4e_apollo_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_VXLAN);
    subtract(udp_0.len, scratch_metadata.ip_totallen, 20);

    modify_field(vxlan_0.flags, 0x8);
    modify_field(vxlan_0.vni, rewrite_metadata.dst_slot_id);
    modify_field(vxlan_0.reserved, 0);
    modify_field(vxlan_0.reserved2, 0);

    if (rewrite_metadata.mytep_ip == dipo) {
        modify_field(control_metadata.local_switching, 1);
    }
}

action ipv6_vxlan_tep(dipo, dmac, sipo) {
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
    // mytep_macsa is a table constant
    modify_field(ethernet_0.srcAddr, scratch_metadata.mytep_macsa);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV6);

    modify_field(ipv6_0.version, 6);
    modify_field(ipv6_0.payloadLen, scratch_metadata.ip_totallen);
    modify_field(ipv6_0.hopLimit, 64);
    modify_field(ipv6_0.nextHdr, IP_PROTO_UDP);
    modify_field(ipv6_0.dstAddr, dipo);
    modify_field(ipv6_0.srcAddr, sipo);

    modify_field(udp_0.srcPort, (0xC000 | p4e_apollo_i2e.entropy_hash));
    modify_field(udp_0.dstPort, UDP_PORT_VXLAN);
    modify_field(udp_0.len, scratch_metadata.ip_totallen);

    modify_field(vxlan_0.flags, 0x8);
    modify_field(vxlan_0.vni, rewrite_metadata.dst_slot_id);
    modify_field(vxlan_0.reserved, 0);
    modify_field(vxlan_0.reserved2, 0);

    if (sipo == dipo) {
        modify_field(control_metadata.local_switching, 1);
    }
}

@pragma stage 4
table tep {
    reads {
        rewrite_metadata.tep_index  : exact;
    }
    actions {
        gre_tep;
        mpls_udp_tep;
        ipv4_vxlan_tep;
        ipv6_vxlan_tep;
    }
    size : TEP_TABLE_SIZE;
}

/******************************************************************************
 * NAT rewrite
 *****************************************************************************/
action nat(nat_ip) {
    if (nat_metadata.snat_required == TRUE) {
        // SNAT only in Tx direction
        if (ipv4_1.valid == TRUE) {
            modify_field(ipv4_1.srcAddr, nat_ip);
        } else {
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.srcAddr, nat_ip);
            }
        }
    } else {
        if (p4e_apollo_i2e.dnat_required == TRUE) {
            if (ipv4_2.valid == TRUE) {
                modify_field(ipv4_2.dstAddr, nat_ip);
            } else {
                if (ipv6_2.valid == TRUE) {
                    modify_field(ipv6_2.dstAddr, nat_ip);
                }
            }
        }
    }
}

@pragma stage 4
@pragma hbm_table
table nat {
    reads {
        p4e_apollo_i2e.xlate_index  : exact;
    }
    actions {
        nat;
    }
    size : NAT_TABLE_SIZE;
}

control rewrite {
    if (control_metadata.direction == TX_FROM_HOST) {
        apply(nexthop_group);
        apply(nexthop);
        apply(tep);
    }
    apply(nat);
}
