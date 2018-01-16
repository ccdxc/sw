/*****************************************************************************/
/* P4+ APP related processing                                                */
/*****************************************************************************/

action p4plus_app_default () {
    f_egress_tcp_options_fixup();
    if (control_metadata.checksum_results != 0) {
        drop_packet();
    }
}

action p4plus_app_tcp_proxy() {
    // drop packet on checksum error
    if (control_metadata.checksum_results != 0) {
        drop_packet();
    }

    if ((tcp.flags & TCP_FLAG_SYN) == TCP_FLAG_SYN) {
        f_p4plus_cpu_pkt(0);
        f_egress_tcp_options_fixup();
        modify_field(p4_to_p4plus_tcp_proxy.payload_len,
                     (capri_p4_intrinsic.packet_len + P4PLUS_CPU_PKT_SZ));
    } else {
        remove_header(ethernet);
        remove_header(vlan_tag);
        remove_header(ipv4);
        remove_header(ipv6);
        remove_header(tcp);
        //remove_header(tcp_options_blob);
        remove_header(tcp_option_eol);
        remove_header(tcp_option_nop);
        remove_header(tcp_option_mss);
        remove_header(tcp_option_ws);
        remove_header(tcp_option_sack_perm);
        remove_header(tcp_option_timestamp);
        remove_header(tcp_option_one_sack);
        remove_header(tcp_option_two_sack);
        remove_header(tcp_option_three_sack);
        remove_header(tcp_option_four_sack);
        modify_field(p4_to_p4plus_tcp_proxy.payload_len,
                     l4_metadata.tcp_data_len);
    }

    add_header(p4_to_p4plus_tcp_proxy);
    add_header(p4_to_p4plus_tcp_proxy_sack);
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

    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_TCP_PROXY_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action f_p4plus_app_classic_nic_prep() {
    if ((inner_ethernet.valid == TRUE) or (inner_ipv4.valid == TRUE) or
        (inner_ipv6.valid == TRUE)) {
        add_header(p4_to_p4plus_classic_nic_inner_ip);
        if (inner_ipv4.valid == TRUE) {
            if (inner_udp.valid == TRUE) {
                modify_field(p4_to_p4plus_classic_nic.header_flags,
                             CLASSIC_NIC_HEADER_FLAGS_IPV4_UDP);
            } else {
                if (tcp.valid == TRUE) {
                    modify_field(p4_to_p4plus_classic_nic.header_flags,
                                 CLASSIC_NIC_HEADER_FLAGS_IPV4_TCP);
                }
                else {
                    modify_field(p4_to_p4plus_classic_nic.header_flags,
                                 CLASSIC_NIC_HEADER_FLAGS_IPV4);
                }
            }
        }
        if (inner_ipv6.valid == TRUE) {
            if (inner_udp.valid == TRUE) {
                modify_field(p4_to_p4plus_classic_nic.header_flags,
                             CLASSIC_NIC_HEADER_FLAGS_IPV6_UDP);
            } else {
                if (tcp.valid == TRUE) {
                    modify_field(p4_to_p4plus_classic_nic.header_flags,
                                 CLASSIC_NIC_HEADER_FLAGS_IPV6_TCP);
                }
                else {
                    modify_field(p4_to_p4plus_classic_nic.header_flags,
                                 CLASSIC_NIC_HEADER_FLAGS_IPV6);
                }
            }
        }
    } else {
        add_header(p4_to_p4plus_classic_nic_ip);
        if (ipv4.valid == TRUE) {
            if (udp.valid == TRUE) {
                modify_field(p4_to_p4plus_classic_nic.header_flags,
                             CLASSIC_NIC_HEADER_FLAGS_IPV4_UDP);
            } else {
                if (tcp.valid == TRUE) {
                    modify_field(p4_to_p4plus_classic_nic.header_flags,
                                 CLASSIC_NIC_HEADER_FLAGS_IPV4_TCP);
                }
                else {
                    modify_field(p4_to_p4plus_classic_nic.header_flags,
                                 CLASSIC_NIC_HEADER_FLAGS_IPV4);
                }
            }
        }
        if (ipv6.valid == TRUE) {
            if (udp.valid == TRUE) {
                modify_field(p4_to_p4plus_classic_nic.header_flags,
                             CLASSIC_NIC_HEADER_FLAGS_IPV6_UDP);
            } else {
                if (tcp.valid == TRUE) {
                    modify_field(p4_to_p4plus_classic_nic.header_flags,
                                 CLASSIC_NIC_HEADER_FLAGS_IPV6_TCP);
                }
                else {
                    modify_field(p4_to_p4plus_classic_nic.header_flags,
                                 CLASSIC_NIC_HEADER_FLAGS_IPV6);
                }
            }
        }
    }
    if (inner_udp.valid == TRUE) {
        modify_field(p4_to_p4plus_classic_nic.l4_sport, inner_udp.srcPort);
        modify_field(p4_to_p4plus_classic_nic.l4_dport, inner_udp.dstPort);
    } else {
        if (udp.valid == TRUE) {
            modify_field(p4_to_p4plus_classic_nic.l4_sport, udp.srcPort);
            modify_field(p4_to_p4plus_classic_nic.l4_dport, udp.dstPort);
        }
        if (tcp.valid == TRUE) {
            modify_field(p4_to_p4plus_classic_nic.l4_sport, tcp.srcPort);
            modify_field(p4_to_p4plus_classic_nic.l4_dport, tcp.dstPort);
        }
    }

    // checksum level (use generated value in ASM, not the ones in defines.h)
    if (((control_metadata.checksum_results & (1 << CSUM_HDR_UDP)) != 0) or
        (((control_metadata.checksum_results & (1 << CSUM_HDR_TCP)) != 0) and
        ((inner_ipv4.valid == FALSE) and (inner_ipv6.valid == FALSE)))) {
        if (((control_metadata.checksum_results & (1 << CSUM_HDR_TCP)) != 0) and
            ((inner_ipv4.valid == FALSE) and (inner_ipv6.valid == FALSE))) {
            modify_field(p4_to_p4plus_classic_nic.csum_level, 2);
        } else {
            modify_field(p4_to_p4plus_classic_nic.csum_level, 1);
        }
    }

    // l2 checksum (CHECKSUM_COMPLETE) : code in ASM only
}

action p4plus_app_classic_nic() {
    if ((control_metadata.vlan_strip == TRUE) and (vlan_tag.valid == TRUE)) {
        modify_field(ethernet.etherType, vlan_tag.etherType);
        modify_field(p4_to_p4plus_classic_nic.vlan_pcp, vlan_tag.pcp);
        modify_field(p4_to_p4plus_classic_nic.vlan_dei, vlan_tag.dei);
        modify_field(p4_to_p4plus_classic_nic.vlan_vid, vlan_tag.vid);
        modify_field(p4_to_p4plus_classic_nic.vlan_valid, TRUE);
        remove_header(vlan_tag);
        subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len, 4);
    }
    if (control_metadata.checksum_results == 0) {
        modify_field(p4_to_p4plus_classic_nic.csum_ok, TRUE);
    }
    modify_field(p4_to_p4plus_classic_nic.packet_len,
                 capri_p4_intrinsic.packet_len);
    add_header(p4_to_p4plus_classic_nic);
    add_header(capri_rxdma_intrinsic);
    modify_field(p4_to_p4plus_classic_nic.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CLASSIC_NIC_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);

    f_egress_tcp_options_fixup();
}

action p4plus_app_ipsec() {
    // drop packet on checksum error
    if (control_metadata.checksum_results != 0) {
        drop_packet();
    }

    add_header(p4_to_p4plus_ipsec);
    modify_field(p4_to_p4plus_ipsec.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(p4_to_p4plus_ipsec.seq_no, ipsec_metadata.seq_no);
    modify_field(p4_to_p4plus_ipsec.spi, (flow_lkp_metadata.lkp_sport << 16) | flow_lkp_metadata.lkp_dport);

    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_IPSEC_HDR_SZ));

    if (ipv4.valid == TRUE) {
        if (udp.valid == TRUE) {
            modify_field(p4_to_p4plus_ipsec.ip_hdr_size, ipv4.ihl << 2 + UDP_HDR_SIZE);
        } else {
            modify_field(p4_to_p4plus_ipsec.ip_hdr_size, ipv4.ihl << 2);
        }
        modify_field(p4_to_p4plus_ipsec.l4_protocol, ipv4.protocol);
        if (vlan_tag.valid == TRUE) {
            modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 18);
            modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv4.totalLen+18);
        } else {
            modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 14);
            modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv4.totalLen+14);
        }
    }

    if (ipv6.valid == TRUE) {
        // Remove all extension headers before sending it to p4+
        if (udp.valid == TRUE) {
            modify_field(p4_to_p4plus_ipsec.ip_hdr_size, IPV6_BASE_HDR_SIZE+UDP_HDR_SIZE);
        } else {
            modify_field(p4_to_p4plus_ipsec.ip_hdr_size, IPV6_BASE_HDR_SIZE);
        }
        modify_field(p4_to_p4plus_ipsec.l4_protocol, ipv6.nextHdr);
        modify_field(v6_generic.valid, 0);
        if (vlan_tag.valid == TRUE) {
            modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 18);
            modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv6.payloadLen+18);
        } else {
            modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 14);
            modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv6.payloadLen+14);
        }
    }

    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_rdma() {
    // drop packet on checksum error
    if (control_metadata.checksum_results != 0) {
        drop_packet();
    }

    modify_field(p4_to_p4plus_roce.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    if (ipv4.valid == TRUE) {
        modify_field(p4_to_p4plus_roce.ecn, ipv4.diffserv, 0xC0);
    } else {
        if (ipv6.valid == TRUE) {
            modify_field(p4_to_p4plus_roce.ecn, ipv6.trafficClass, 0xC0);
        }
    }

    if ((udp_opt_ocs.valid == FALSE) or
        ((udp_opt_ocs.valid == TRUE) and ((capri_intrinsic.csum_err &
                                           (1 << CSUM_HDR_UDP_OPT_OCS)) == 0))) {
        if (udp_opt_timestamp.valid == TRUE) {
            modify_field(p4_to_p4plus_roce.roce_opt_ts_valid, TRUE);
        }
        if (udp_opt_mss.valid == TRUE) {
            modify_field(p4_to_p4plus_roce.roce_opt_mss_valid, TRUE);
        }
    }

    remove_header(ethernet);
    remove_header(vlan_tag);
    remove_header(ipv4);
    remove_header(ipv6);
    remove_header(udp);
    remove_header(icrc);
    remove_header(udp_opt_eol);
    remove_header(udp_opt_nop);
    remove_header(udp_opt_ocs);
    remove_header(udp_opt_mss); 
    // packet payload removed in ASM (decode_roce_opcode.asm)
}


action f_egress_tcp_options_fixup () {
    // if(tcp_options_blob.valid == TRUE) {
    if (tcp.valid == TRUE and tcp.dataOffset > 5) {
        remove_header(tcp_option_eol);
        remove_header(tcp_option_nop);
        remove_header(tcp_option_mss);
        remove_header(tcp_option_ws);
        remove_header(tcp_option_sack_perm);
        remove_header(tcp_option_timestamp);
        remove_header(tcp_option_one_sack);
        remove_header(tcp_option_two_sack);
        remove_header(tcp_option_three_sack);
        remove_header(tcp_option_four_sack);
    }
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
    // Copy the src_tm_iq only for pkts from TxDMA
    if ((control_metadata.lkp_flags_egress & (1 << CPU_LKP_FLAGS_LKP_DIR)) == 
            (FLOW_DIR_FROM_DMA << CPU_LKP_FLAGS_LKP_DIR)) {
        modify_field(p4_to_p4plus_cpu_pkt.src_tm_iq, control_metadata.src_tm_iq);
    }

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
        if (tcp.dataOffset > 5) {
            bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
                   CPU_FLAGS_TCP_OPTIONS_PRESENT);
            if (tcp_option_ws.valid == TRUE) {
                bit_or(scratch_metadata.cpu_tcp_options, scratch_metadata.cpu_tcp_options,
                       CPU_TCP_OPTIONS_WINDOW_SCALE);
            }
            if (tcp_option_mss.valid == TRUE) {
                bit_or(scratch_metadata.cpu_tcp_options, scratch_metadata.cpu_tcp_options,
                       CPU_TCP_OPTIONS_MSS);
            }
            if (tcp_option_timestamp.valid == TRUE) {
                bit_or(scratch_metadata.cpu_tcp_options, scratch_metadata.cpu_tcp_options,
                       CPU_TCP_OPTIONS_TIMESTAMP);
            }
            if (tcp_option_sack_perm.valid == TRUE) {
                bit_or(scratch_metadata.cpu_tcp_options, scratch_metadata.cpu_tcp_options,
                       CPU_TCP_OPTIONS_SACK_PERMITTED);
            }
        }
    }
    if ((icmp.valid == TRUE) or (icmpv6.valid == TRUE)) {
        modify_field(p4_to_p4plus_cpu_pkt.l4_offset, scratch_metadata.offset);
        add_to_field(scratch_metadata.offset, 4);
    }
    if (ah.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu_pkt.l4_offset, scratch_metadata.offset);
        add_to_field(scratch_metadata.offset, 12);
    }

    modify_field(p4_to_p4plus_cpu_pkt.payload_offset, scratch_metadata.offset);
    modify_field(p4_to_p4plus_cpu_pkt.flags, scratch_metadata.cpu_flags);
    modify_field(p4_to_p4plus_cpu_pkt.tcp_options, scratch_metadata.cpu_tcp_options);
}

action p4plus_app_cpu() {
    // drop packet on checksum error
    if (control_metadata.checksum_results != 0) {
        drop_packet();
    }

    add_header(p4_to_p4plus_cpu);
    add_header(p4_to_p4plus_cpu_ip);
    modify_field(p4_to_p4plus_cpu.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(p4_to_p4plus_cpu.table0_valid, TRUE);

    if (ipv4.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.ip_proto, ipv4.protocol);
        modify_field(p4_to_p4plus_cpu.packet_type, CPU_PACKET_TYPE_IPV4);
    }
    if (ipv6.valid == TRUE) {
        modify_field(p4_to_p4plus_cpu.ip_proto, ipv6.nextHdr);
        modify_field(p4_to_p4plus_cpu.packet_type, CPU_PACKET_TYPE_IPV6);
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

    add(p4_to_p4plus_cpu.packet_len, capri_p4_intrinsic.packet_len,
        P4PLUS_CPU_PKT_SZ);

    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CPU_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);

    f_egress_tcp_options_fixup();
}

action p4plus_app_raw_redir() {
    // drop packet on checksum error
    if (control_metadata.checksum_results != 0) {
        drop_packet();
    }

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

    add(p4_to_p4plus_cpu.packet_len, capri_p4_intrinsic.packet_len,
        P4PLUS_CPU_PKT_SZ);

    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CPU_HDR_SZ));
    modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
    modify_field(capri_rxdma_intrinsic.qtype, control_metadata.qtype);
}

action p4plus_app_p4pt() {
    // drop packet on checksum error
    if (control_metadata.checksum_results != 0) {
        drop_packet();
    }

    add_header(p4_to_p4plus_p4pt);
    modify_field(p4_to_p4plus_p4pt.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(p4_to_p4plus_p4pt.p4pt_idx, flow_info_metadata.flow_index);
    modify_field(p4_to_p4plus_p4pt.flow_dir, control_metadata.lkp_flags_egress);

    remove_header(ethernet);
    remove_header(vlan_tag);
    remove_header(ipv4);
    remove_header(ipv6);
    remove_header(udp);
    remove_header(tcp);
    remove_header(tcp_option_eol);
    remove_header(tcp_option_nop);
    remove_header(tcp_option_mss);
    remove_header(tcp_option_ws);
    remove_header(tcp_option_sack_perm);
    remove_header(tcp_option_timestamp);
    remove_header(tcp_option_one_sack);
    remove_header(tcp_option_two_sack);
    remove_header(tcp_option_three_sack);
    remove_header(tcp_option_four_sack);

    if (tcp.valid == TRUE) {
        if (l4_metadata.tcp_data_len < 64) {
            modify_field(scratch_metadata.packet_len, l4_metadata.tcp_data_len);
        } else {
            modify_field(scratch_metadata.packet_len, 64);
        }
    }

    if (udp.valid == TRUE) {
        if (udp.len < 64) {
            modify_field(scratch_metadata.packet_len, udp.len);
        } else {
            modify_field(scratch_metadata.packet_len, 64);
        }
    }

    modify_field(p4_to_p4plus_p4pt.payload_len, scratch_metadata.packet_len);
    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_P4PT_HDR_SZ));
    add_to_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 scratch_metadata.packet_len);
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
        p4plus_app_p4pt;
        p4plus_app_default;
    }
    default_action : p4plus_app_default;
    size : P4PLUS_APP_TABLE_SIZE;
}

action p4plus_app_prep() {
    if (control_metadata.p4plus_app_id == P4PLUS_APPTYPE_CLASSIC_NIC) {
        f_p4plus_app_classic_nic_prep();
    }
}

@pragma stage 3
table p4plus_app_prep {
    actions {
        p4plus_app_prep;
    }
    default_action : p4plus_app_prep;
}

/*****************************************************************************/
/* P4+ to P4 app processing                                                  */
/*****************************************************************************/
action f_p4plus_to_p4_1() {
    // update IP id
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID) != 0) {
        add(ipv4.identification, ipv4.identification, p4plus_to_p4.ip_id_delta);
    }

    // update IP length
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN) != 0) {
        if (vlan_tag.valid == TRUE) {
            subtract(scratch_metadata.packet_len,
                     capri_p4_intrinsic.packet_len, 18);
        } else {
            subtract(scratch_metadata.packet_len,
                     capri_p4_intrinsic.packet_len, 14);
        }
        if (ipv4.valid == TRUE) {
            modify_field(ipv4.totalLen, scratch_metadata.packet_len);
            subtract_from_field(scratch_metadata.packet_len, ipv4.ihl << 2);
        } else {
            if (ipv6.valid == TRUE) {
                subtract_from_field(scratch_metadata.packet_len, 40);
                modify_field(ipv6.payloadLen, scratch_metadata.packet_len);
            }
        }

        // update tcp_data_len
        modify_field(l4_metadata.tcp_data_len,
                     (scratch_metadata.packet_len - (tcp.dataOffset) * 4));
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
        add(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len, 4);
    }

    // update from cpu flag
    if (p4plus_to_p4.p4plus_app_id == P4PLUS_APPTYPE_CPU)  {
        modify_field(control_metadata.from_cpu, TRUE);
    }
}

action f_p4plus_to_p4_2() {
    // update UDP length
    if (ipv4.valid == TRUE) {
        subtract(scratch_metadata.packet_len, ipv4.totalLen, ipv4.ihl << 2);
    } else {
        if (ipv6.valid == TRUE) {
            modify_field(scratch_metadata.packet_len, ipv6.payloadLen);
        }
    }
    modify_field(control_metadata.udp_opt_bytes, p4plus_to_p4.udp_opt_bytes);
    if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN) != 0) {
        subtract(udp.len, scratch_metadata.packet_len,
                 p4plus_to_p4.udp_opt_bytes);
    }

    // update checksum/icrc compute flags
    modify_field(scratch_metadata.size8, 0);
    if (p4plus_to_p4.p4plus_app_id == P4PLUS_APPTYPE_CLASSIC_NIC) {
        if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_COMPUTE_L4_CSUM) != 0) {
            bit_or(scratch_metadata.size8, scratch_metadata.size8,
                   (1 << CHECKSUM_CTL_L4_CHECKSUM));
        }
        if ((p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_COMPUTE_INNER_L4_CSUM) != 0) {
            bit_or(scratch_metadata.size8, scratch_metadata.size8,
                   (1 << CHECKSUM_CTL_INNER_L4_CHECKSUM));
        }
    } else {
        if (ipv4.valid == TRUE) {
            bit_or(scratch_metadata.size8, scratch_metadata.size8,
                   (1 << CHECKSUM_CTL_IP_CHECKSUM));
        }
        if (p4plus_to_p4.p4plus_app_id == P4PLUS_APPTYPE_RDMA) {
            bit_or(scratch_metadata.size8, scratch_metadata.size8,
                   (1 << CHECKSUM_CTL_ICRC));
        } else {
            if ((udp.valid == TRUE) or (tcp.valid == TRUE)) {
                bit_or(scratch_metadata.size8, scratch_metadata.size8,
                        (1 << CHECKSUM_CTL_L4_CHECKSUM));
            }
        }
    }
    modify_field(control_metadata.checksum_ctl, scratch_metadata.size8);

    remove_header(p4plus_to_p4);
    remove_header(capri_txdma_intrinsic);
}

@pragma stage 1
table p4plus_to_p4_1 {
    actions {
        f_p4plus_to_p4_1;
    }
    default_action : f_p4plus_to_p4_1;
}

@pragma stage 3
table p4plus_to_p4_2 {
    actions {
        f_p4plus_to_p4_2;
    }
    default_action : f_p4plus_to_p4_2;
}

control process_p4plus_to_p4 {
    if (p4plus_to_p4.valid == TRUE) {
        apply(p4plus_to_p4_2);
    }
}
