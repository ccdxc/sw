/*****************************************************************************/
/* P4+ APP related processing                                                */
/*****************************************************************************/
action p4plus_app_tcp_proxy() {
    remove_header(ethernet);
    remove_header(vlan_tag);
    remove_header(ipv4);
    remove_header(ipv6);
    remove_header(tcp);

    add_header(p4_to_p4plus_tcp_proxy);
    add_header(p4_to_p4plus_tcp_proxy_sack);
    modify_field(p4_to_p4plus_tcp_proxy.payload_len, l4_metadata.tcp_data_len);
    modify_field(p4_to_p4plus_tcp_proxy.p4plus_app_id,
                 control_metadata.p4plus_app_id);
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
        if ((ipv4.valid == TRUE) or (ipv6.valid == TRUE)) {
            add_header(p4_to_p4plus_classic_nic_ip);
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

    add_header(capri_rxdma_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CLASSIC_NIC_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_ipsec() {
}

action p4plus_app_rdma() {
}

action p4plus_app_cpu() {
    add_header(p4_to_p4plus_cpu);
    modify_field(p4_to_p4plus_cpu.p4plus_app_id,
                 control_metadata.p4plus_app_id);

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
        nop;
    }
    size : P4PLUS_APP_TABLE_SIZE;
}
