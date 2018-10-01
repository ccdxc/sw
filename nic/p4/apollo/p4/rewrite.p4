action nexthop_info(tep_index, snat_required, encap_type,
                    dst_slot_id, traffic_class)
{
    modify_field(rewrite_metadata.tep_index, tep_index);
    modify_field(nat_metadata.snat_required, snat_required);
    modify_field(rewrite_metadata.encap_type, encap_type);
    modify_field(rewrite_metadata.dst_slot_id, dst_slot_id);
    modify_field(policer_metadata.traffic_class, traffic_class);
    // mytep_ip is a table constant
    modify_field(rewrite_metadata.mytep_ip, scratch_metadata.mytep_ip);
}

@pragma stage 2
table nexthop_tx {
    reads {
        txdma_to_p4e_header.nexthop_index      : exact;
    }
    actions {
        nexthop_info;
    }
    size : NEXTHOP_TX_TABLE_SIZE;
}

action tep_tx(dipo, dmac)
{
    // Remove headers
    remove_header(ethernet_1);
    remove_header(ctag_1);

    // Add the tunnel header
    add_header(ethernet_0);
    add_header(ipv4_0);
    add_header(gre_0);
    add_header(mpls_dst_0);
    if (rewrite_metadata.encap_type == VNIC_ENCAP) {
        add_header(mpls_src_0);
    }

    modify_field(ethernet_0.dstAddr, dmac);
    // mytep_macsa is a table constant
    modify_field(ethernet_0.srcAddr, scratch_metadata.mytep_macsa);
    modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);
    modify_field(ipv4_0.version, 4);
    modify_field(ipv4_0.ihl, 5);
    // Subtract the ethernet header size
    modify_field(scratch_metadata.ip_totallen, capri_p4_intrinsic.packet_len - 14);
    if (ctag_1.valid == TRUE) {
        subtract_from_field(scratch_metadata.ip_totallen, 4);
    }
    // Account for the new headers we are adding
    // 4 bytes of GRE header, 4 bytes of mpls header, 20 bytes of IP header
    add_to_field(scratch_metadata.ip_totallen, 20 + 4 + 4);
    modify_field(ipv4_0.ttl, 64);
    modify_field(ipv4_0.protocol, IP_PROTO_GRE);
    modify_field(ipv4_0.dstAddr, dipo);
    modify_field(ipv4_0.srcAddr, rewrite_metadata.mytep_ip);
    // TODO setup the ip, gre and mpls headers correctly
    modify_field(gre_0.proto, ETHERTYPE_MPLS_UNICAST);
    modify_field(mpls_dst_0.label, rewrite_metadata.dst_slot_id);
    if (rewrite_metadata.encap_type == VNIC_ENCAP) {
        modify_field(mpls_src_0.label, p4e_apollo_i2e.src_slot_id);
        modify_field(mpls_src_0.bos, 1);
        add_to_field(scratch_metadata.ip_totallen, 4);
    } else {
        modify_field(mpls_dst_0.bos, 1);
    }

    modify_field(ipv4_0.totalLen, scratch_metadata.ip_totallen);

    // scratch metadata
    modify_field(scratch_metadata.encap_type, rewrite_metadata.encap_type);
}

@pragma stage 3
table tep_tx {
    reads {
        rewrite_metadata.tep_index  : exact;
    }
    actions {
        tep_tx;
    }
    size : TEP_TABLE_SIZE;
}

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

@pragma stage 3
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
        apply(nexthop_tx);
        apply(tep_tx);
    }
    apply(nat);
}
