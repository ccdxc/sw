/*****************************************************************************/
/* P4+ APP related processing                                                */
/*****************************************************************************/
action p4plus_app_tcp_proxy() {
    if ((tcp.flags & TCP_FLAG_SYN) == TCP_FLAG_SYN) {
        f_p4plus_cpu_pkt(0);
    } else {
        remove_header(ethernet);
        remove_header(vlan_tag);
        remove_header(ipv4);
        remove_header(ipv6);
        remove_header(tcp);
    }

    add_header(p4_to_p4plus_tcp_proxy);
    add_header(p4_to_p4plus_tcp_proxy_sack);
    modify_field(p4_to_p4plus_tcp_proxy.payload_len, l4_metadata.tcp_data_len);
    modify_field(p4_to_p4plus_tcp_proxy.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(p4_to_p4plus_tcp_proxy.table0_valid, TRUE);

    if (tcp_option_one_sack.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.num_sack_blocks, 1);
    }
    if (tcp_option_two_sack.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.num_sack_blocks, 2);
    }
    if (tcp_option_three_sack.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.num_sack_blocks, 3);
    }
    if (tcp_option_four_sack.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.num_sack_blocks, 4);
    }

    add_header(capri_rxdma_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_TCP_PROXY_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_classic_nic() {
    add_header(p4_to_p4plus_classic_nic);
    modify_field(p4_to_p4plus_classic_nic.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(scratch_metadata.classic_nic_flags, CLASSIC_NIC_FLAGS_FCS_OK);

    if ((control_metadata.vlan_strip == TRUE) and (vlan_tag.valid == TRUE)) {
        modify_field(ethernet.etherType, vlan_tag.etherType);
        modify_field(p4_to_p4plus_classic_nic.vlan_pcp, vlan_tag.pcp);
        modify_field(p4_to_p4plus_classic_nic.vlan_dei, vlan_tag.dei);
        modify_field(p4_to_p4plus_classic_nic.vlan_vid, vlan_tag.vid);
        bit_or(p4_to_p4plus_classic_nic.flags, p4_to_p4plus_classic_nic.flags,
               CLASSIC_NIC_FLAGS_VLAN_VALID);
        remove_header(vlan_tag);
        subtract(control_metadata.packet_len, control_metadata.packet_len, 4);
    }

    if ((inner_ipv4.valid == TRUE) or (inner_ipv6.valid == TRUE)) {
        add_header(p4_to_p4plus_classic_nic_inner_ip);
        if (inner_ipv4.valid == TRUE) {
            modify_field(p4_to_p4plus_classic_nic.ip_proto,
                         inner_ipv4.protocol);
            bit_or(scratch_metadata.classic_nic_flags,
                   scratch_metadata.classic_nic_flags,
                   CLASSIC_NIC_FLAGS_IPV4_VALID);
        }
        if (inner_ipv6.valid == TRUE) {
            modify_field(p4_to_p4plus_classic_nic.ip_proto,
                         inner_ipv6.nextHdr);
            bit_or(scratch_metadata.classic_nic_flags,
                   scratch_metadata.classic_nic_flags,
                   CLASSIC_NIC_FLAGS_IPV6_VALID);
        }
        bit_or(scratch_metadata.classic_nic_flags,
               scratch_metadata.classic_nic_flags, CLASSIC_NIC_FLAGS_TUNNELED);
    } else {
        add_header(p4_to_p4plus_classic_nic_ip);
        if ((ipv4.valid == TRUE) or (ipv6.valid == TRUE)) {
            if (ipv4.valid == TRUE) {
                modify_field(p4_to_p4plus_classic_nic.ip_proto, ipv4.protocol);
                bit_or(scratch_metadata.classic_nic_flags,
                       scratch_metadata.classic_nic_flags,
                       CLASSIC_NIC_FLAGS_IPV4_VALID);
            }
            if (ipv6.valid == TRUE) {
                modify_field(p4_to_p4plus_classic_nic.ip_proto, ipv6.nextHdr);
                bit_or(scratch_metadata.classic_nic_flags,
                       scratch_metadata.classic_nic_flags,
                       CLASSIC_NIC_FLAGS_IPV6_VALID);
            }
        }
    }
    if (inner_udp.valid == TRUE) {
        modify_field(p4_to_p4plus_classic_nic.l4_sport, inner_udp.srcPort);
        modify_field(p4_to_p4plus_classic_nic.l4_dport, inner_udp.dstPort);
        modify_field(p4_to_p4plus_classic_nic.l4_checksum, inner_udp.checksum);
    } else {
        if (udp.valid == TRUE) {
            modify_field(p4_to_p4plus_classic_nic.l4_sport, udp.srcPort);
            modify_field(p4_to_p4plus_classic_nic.l4_dport, udp.dstPort);
            modify_field(p4_to_p4plus_classic_nic.l4_checksum, udp.checksum);
        }
        if (tcp.valid == TRUE) {
            modify_field(p4_to_p4plus_classic_nic.l4_sport, tcp.srcPort);
            modify_field(p4_to_p4plus_classic_nic.l4_dport, tcp.dstPort);
            modify_field(p4_to_p4plus_classic_nic.l4_checksum, tcp.checksum);
        }
    }
    bit_or(p4_to_p4plus_classic_nic.flags, p4_to_p4plus_classic_nic.flags,
           scratch_metadata.classic_nic_flags);
    modify_field(p4_to_p4plus_classic_nic.packet_len, control_metadata.packet_len);

    add_header(capri_rxdma_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CLASSIC_NIC_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_ipsec() {
    add_header(p4_to_p4plus_ipsec);
    modify_field(p4_to_p4plus_ipsec.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(p4_to_p4plus_ipsec.seq_no, ipsec_metadata.seq_no);

    add_header(capri_rxdma_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_IPSEC_HDR_SZ));
    if (vlan_tag.valid == TRUE) {
        modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 18);
        modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv4.totalLen+18);
    } else {
        modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 14);
        modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv4.totalLen+18);
    }
    modify_field(p4_to_p4plus_ipsec.l4_protocol, ipv4.protocol);
    modify_field(p4_to_p4plus_ipsec.ip_hdr_size, ipv4.ihl << 2);

    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_rdma() {
    modify_field(p4_to_p4plus_roce.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    remove_header(ethernet);
    remove_header(vlan_tag);
    remove_header(ipv4);
    remove_header(ipv6);
    remove_header(udp);
}

action f_p4plus_cpu_pkt(offset) {
    add_header(p4_to_p4plus_cpu_pkt);

    modify_field(p4_to_p4plus_cpu_pkt.src_lif, control_metadata.src_lif);
    modify_field(p4_to_p4plus_cpu_pkt.lif, capri_intrinsic.lif);
    modify_field(p4_to_p4plus_cpu_pkt.qid, control_metadata.qid);
    modify_field(p4_to_p4plus_cpu_pkt.qtype, control_metadata.qtype);
    modify_field(p4_to_p4plus_cpu_pkt.lkp_vrf, flow_lkp_metadata.lkp_vrf);
    modify_field(p4_to_p4plus_cpu_pkt.lkp_dir,
                 control_metadata.lkp_flags_egress);
    modify_field(p4_to_p4plus_cpu_pkt.lkp_inst,
                 control_metadata.lkp_flags_egress);
    modify_field(p4_to_p4plus_cpu_pkt.lkp_type,
                 control_metadata.lkp_flags_egress);

    modify_field(p4_to_p4plus_cpu_pkt.l2_offset, 0xFFFF);
    modify_field(p4_to_p4plus_cpu_pkt.l3_offset, 0xFFFF);
    modify_field(p4_to_p4plus_cpu_pkt.l4_offset, 0xFFFF);
    modify_field(p4_to_p4plus_cpu_pkt.payload_offset, 0xFFFF);

    modify_field(scratch_metadata.offset, offset);
    modify_field(scratch_metadata.cpu_flags, 0);

    if (ethernet.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu_pkt.l2_offset, scratch_metadata.offset);
        if (vlan_tag.valid == TRUE) {
            add_to_field(scratch_metadata.offset, 18);
        } else {
            add_to_field(scratch_metadata.offset, 14);
        }
    }
    if (ipv4.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               CPU_FLAGS_IPV4_VALID);
        modify_field(p4_to_p4plus_cpu_pkt.l3_offset, scratch_metadata.offset);
        add_to_field(scratch_metadata.offset, ipv4.ihl << 2);
    }
    if (ipv6.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               CPU_FLAGS_IPV6_VALID);
        modify_field(p4_to_p4plus_cpu_pkt.l3_offset, scratch_metadata.offset);
        add_to_field(scratch_metadata.offset, 40);
    }
    if ((udp.valid == TRUE) or (esp.valid == TRUE)) {
        modify_field(p4_to_p4plus_cpu_pkt.l4_offset, scratch_metadata.offset);
        add_to_field(scratch_metadata.offset, 8);
    }
    if (tcp.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu_pkt.l4_offset, scratch_metadata.offset);
        add_to_field(scratch_metadata.offset, tcp.dataOffset << 2);
    }
    if ((icmp.valid == TRUE) or (icmpv6.valid == TRUE)) {
        modify_field(p4_to_p4plus_cpu_pkt.l4_offset, scratch_metadata.offset);
        add_to_field(scratch_metadata.offset, 4);
    }
    if ((ah.valid == TRUE) or (v6_ah_esp.valid == TRUE)) {
        modify_field(p4_to_p4plus_cpu_pkt.l4_offset, scratch_metadata.offset);
        add_to_field(scratch_metadata.offset, 12);
    }

    modify_field(p4_to_p4plus_cpu_pkt.payload_offset, scratch_metadata.offset);
    modify_field(p4_to_p4plus_cpu_pkt.flags, scratch_metadata.cpu_flags);
}

action p4plus_app_cpu() {
    add_header(p4_to_p4plus_cpu);
    add_header(p4_to_p4plus_cpu_ip);
    modify_field(p4_to_p4plus_cpu.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(p4_to_p4plus_cpu.table0_valid, TRUE);

    if (ipv4.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.ip_proto, ipv4.protocol);
    }
    if (ipv6.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.ip_proto, ipv6.nextHdr);
    }
    if (udp.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.l4_sport, udp.srcPort);
        modify_field(p4_to_p4plus_cpu.l4_dport, udp.dstPort);
    }
    if (tcp.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.l4_sport, tcp.srcPort);
        modify_field(p4_to_p4plus_cpu.l4_dport, tcp.dstPort);
    }
    if (icmp.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.l4_sport, icmp.typeCode);
    }
    if (icmpv6.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.l4_sport, icmpv6.typeCode);
    }

    f_p4plus_cpu_pkt(0);

    add(p4_to_p4plus_cpu.packet_len, control_metadata.packet_len,
        P4PLUS_CPU_PKT_SZ);

    add_header(capri_rxdma_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CPU_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_raw_redir() {
    add_header(p4_to_p4plus_cpu);
    add_header(p4_to_p4plus_cpu_ip);
    modify_field(p4_to_p4plus_cpu.p4plus_app_id,
                 control_metadata.p4plus_app_id);

    if (ipv4.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.ip_proto, ipv4.protocol);
    }
    if (ipv6.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.ip_proto, ipv6.nextHdr);
    }
    if (udp.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.l4_sport, udp.srcPort);
        modify_field(p4_to_p4plus_cpu.l4_dport, udp.dstPort);
    }
    if (tcp.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.l4_sport, tcp.srcPort);
        modify_field(p4_to_p4plus_cpu.l4_dport, tcp.dstPort);
    }
    if (icmp.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.l4_sport, icmp.typeCode);
    }
    if (icmpv6.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.l4_sport, icmpv6.typeCode);
    }

    f_p4plus_cpu_pkt(P4PLUS_RAW_REDIR_HDR_SZ);

    add(p4_to_p4plus_cpu.packet_len, control_metadata.packet_len,
        P4PLUS_CPU_PKT_SZ);

    add_header(capri_rxdma_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CPU_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

@pragma stage 5
table p4plus_app {
    reads {
        control_metadata.p4plus_app_id : exact;
    }
    actions {
        p4plus_app_classic_nic;
        p4plus_app_tcp_proxy;
        p4plus_app_ipsec;
        p4plus_app_rdma;
        p4plus_app_cpu;
        p4plus_app_raw_redir;
        nop;
    }
    size : P4PLUS_APP_TABLE_SIZE;
}

/*****************************************************************************/
/* P4+ to P4 app processing                                                  */
/*****************************************************************************/
action f_p4plus_to_p4() {
    // update IP id
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID) != 0) {
        add(ipv4.identification, ipv4.identification, p4plus_to_p4.ip_id_delta);
    }

    // update IP length
    if (vlan_tag.valid == TRUE) {
        subtract(scratch_metadata.packet_len, control_metadata.packet_len, 18);
    } else {
        subtract(scratch_metadata.packet_len, control_metadata.packet_len, 14);
    }
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN) != 0) {
        if (ipv4.valid == TRUE) {
            modify_field(ipv4.totalLen, scratch_metadata.packet_len);
            subtract_from_field(scratch_metadata.packet_len, ipv4.ihl << 2);
        } else {
            if (ipv6.valid == TRUE) {
                subtract_from_field(scratch_metadata.packet_len, 40);
                modify_field(ipv6.payloadLen, scratch_metadata.packet_len);
            }
        }
    }

    // update UDP length
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN) != 0) {
        modify_field(udp.len, scratch_metadata.packet_len);
    }

    // update TCP sequence number
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO) != 0) {
        add(tcp.seqNo, tcp.seqNo, p4plus_to_p4.tcp_seq_delta);
    }

    // insert vlan tag
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_INSERT_VLAN_TAG) != 0) {
        add_header(vlan_tag);
        modify_field(vlan_tag.pcp, p4plus_to_p4.vlan_tag >> 13);
        modify_field(vlan_tag.dei, p4plus_to_p4.vlan_tag >> 12);
        modify_field(vlan_tag.vid, p4plus_to_p4.vlan_tag);
        modify_field(vlan_tag.etherType, ethernet.etherType);
        modify_field(ethernet.etherType, ETHERTYPE_VLAN);
        add_to_field(control_metadata.packet_len, 4);
        modify_field(capri_p4_intrinsic.packet_len,
                     control_metadata.packet_len);
    }
    remove_header(p4plus_to_p4);
    remove_header(capri_txdma_intrinsic);
}

action p4plus_to_p4_apps() {
    if (p4plus_to_p4.valid == TRUE) {
        f_p4plus_to_p4();
    }
}
