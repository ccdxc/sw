/*****************************************************************************/
/* Ingress pipeline to RxDMA pipeline                                        */
/*****************************************************************************/
action ingress_to_rxdma() {
    add_header(capri_p4_intrinsic);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
    if ((service_header.local_mapping_done == FALSE) or
        (service_header.flow_done == FALSE) or
        (service_header.remote_vnic_mapping_rx_done == FALSE)) {
        add_header(predicate_header);
        modify_field(predicate_header.direction, control_metadata.direction);
        add_header(service_header);
        modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
    } else {
        if (control_metadata.fastpath == TRUE) {
            // TODO : SPAN
            modify_field(capri_intrinsic.tm_span_session,
                         control_metadata.mirror_session);
            modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
            add_header(capri_txdma_intrinsic);
            add_header(predicate_header);
            add_header(txdma_to_p4e_header);
            modify_field(predicate_header.direction, control_metadata.direction);
            add_header(p4i_apollo_i2e);
            remove_header(service_header);
        } else {
            modify_field(capri_intrinsic.tm_span_session,
                         control_metadata.mirror_session);
            modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
            modify_field(capri_intrinsic.lif, APOLLO_SERVICE_LIF);
            add_header(capri_rxdma_intrinsic);
            add_header(p4_to_rxdma_header);
            add_header(predicate_header);
            // Splitter offset should point to here
            modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                         (CAPRI_GLOBAL_INTRINSIC_HDR_SZ +
                          CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                          APOLLO_P4_TO_RXDMA_HDR_SZ + APOLLO_PREDICATE_HDR_SZ));
            add_header(predicate_header2);
            add_header(p4_to_txdma_header);
            add_header(p4i_apollo_i2e);
            remove_header(service_header);

            modify_field(p4_to_rxdma_header.table3_valid, TRUE);
            modify_field(p4_to_rxdma_header.direction, control_metadata.direction);
            modify_field(p4_to_txdma_header.payload_len,
                         capri_p4_intrinsic.packet_len + APOLLO_I2E_HDR_SZ);
            modify_field(predicate_header.direction, control_metadata.direction);
            if (p4_to_txdma_header.lpm_addr == 0) {
                modify_field(predicate_header.lpm_bypass, TRUE);
            }
        }
    }
}

action classic_nic_app() {
    if (capri_intrinsic.tm_oport == TM_PORT_DMA) {
        classic_nic_to_rxdma();
    } else {
        classic_nic_to_uplink();
    }
}

action classic_nic_to_rxdma() {
    add_header(capri_p4_intrinsic);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
    add_header(p4_to_p4plus_classic_nic);
    add_header(p4_to_p4plus_classic_nic_ip);
    add_header(capri_rxdma_intrinsic);

    if ((control_metadata.vlan_strip == TRUE) and (ctag_1.valid == TRUE)) {
        modify_field(ethernet_1.etherType, ctag_1.etherType);
        modify_field(p4_to_p4plus_classic_nic.vlan_pcp, ctag_1.pcp);
        modify_field(p4_to_p4plus_classic_nic.vlan_dei, ctag_1.dei);
        modify_field(p4_to_p4plus_classic_nic.vlan_vid, ctag_1.vid);
        modify_field(p4_to_p4plus_classic_nic.vlan_valid, TRUE);
        remove_header(ctag_1);
        subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len, 4);
    }

    if (key_metadata.ktype == KEY_TYPE_IPV4) {
        if (key_metadata.proto == IP_PROTO_TCP) {
            modify_field(p4_to_p4plus_classic_nic.pkt_type,
                         CLASSIC_NIC_PKT_TYPE_IPV4_TCP);
        } else {
            if (key_metadata.proto == IP_PROTO_UDP) {
                modify_field(p4_to_p4plus_classic_nic.pkt_type,
                             CLASSIC_NIC_PKT_TYPE_IPV4_UDP);
            } else {
                modify_field(p4_to_p4plus_classic_nic.pkt_type,
                             CLASSIC_NIC_PKT_TYPE_IPV4);
            }
        }
    }
    if (key_metadata.ktype == KEY_TYPE_IPV6) {
        if (key_metadata.proto == IP_PROTO_TCP) {
            modify_field(p4_to_p4plus_classic_nic.pkt_type,
                         CLASSIC_NIC_PKT_TYPE_IPV6_TCP);
        } else {
            if (key_metadata.proto == IP_PROTO_UDP) {
                modify_field(p4_to_p4plus_classic_nic.pkt_type,
                             CLASSIC_NIC_PKT_TYPE_IPV6_UDP);
            } else {
                modify_field(p4_to_p4plus_classic_nic.pkt_type,
                             CLASSIC_NIC_PKT_TYPE_IPV6);
            }
        }
    }

    modify_field(p4_to_p4plus_classic_nic.packet_len,
                 capri_p4_intrinsic.packet_len);
    modify_field(p4_to_p4plus_classic_nic.p4plus_app_id,
                 control_metadata.p4plus_app_id);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CLASSIC_NIC_HDR_SZ));
}

action classic_nic_to_uplink() {
    add_header(capri_p4_intrinsic);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
}

action add_p4_to_arm_header() {
    add_header(p4_to_arm);
    modify_field(p4_to_arm.packet_len, capri_p4_intrinsic.packet_len);
    modify_field(p4_to_arm.flow_hash, p4i_apollo_i2e.entropy_hash);
    modify_field(offset_metadata.l2_1, offset_metadata.l2_1);
    modify_field(offset_metadata.l2_2, offset_metadata.l2_2);
    modify_field(offset_metadata.l3_1, offset_metadata.l3_1);
    modify_field(offset_metadata.l3_2, offset_metadata.l3_2);
    modify_field(offset_metadata.l4_1, offset_metadata.l4_1);
    modify_field(offset_metadata.l4_2, offset_metadata.l4_2);
    modify_field(p4_to_arm.payload_offset, offset_metadata.payload_offset);

    modify_field(scratch_metadata.cpu_flags, 0);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           (control_metadata.direction << APOLLO_CPU_FLAGS_DIRECTION_BIT_POS));
    if (ctag_1.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               APOLLO_CPU_FLAGS_VLAN_VALID);
    }
    if (ipv4_1.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               APOLLO_CPU_FLAGS_IPV4_1_VALID);
    } else {
        if (ipv6_1.valid == TRUE) {
            bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
                   APOLLO_CPU_FLAGS_IPV6_1_VALID);
        }
    }
    if (ethernet_2.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               APOLLO_CPU_FLAGS_ETH_2_VALID);
    }
    if (ipv4_2.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               APOLLO_CPU_FLAGS_IPV4_2_VALID);
    } else {
        if (ipv6_2.valid == TRUE) {
            bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
                   APOLLO_CPU_FLAGS_IPV6_2_VALID);
        }
    }
    modify_field(p4_to_arm.flags, scratch_metadata.cpu_flags);
}

action redirect_to_arm() {
    remove_header(capri_txdma_intrinsic);
    remove_header(predicate_header);
    add_p4_to_arm_header();
    classic_nic_to_rxdma();
    modify_field(p4_to_p4plus_classic_nic.packet_len,
                 capri_p4_intrinsic.packet_len + APOLLO_P4_TO_ARM_HDR_SZ);
}

action p4plus_app_tcp_proxy() {
    remove_header(ethernet_1);
    remove_header(ctag_1);
    remove_header(ipv4_1);
    remove_header(ipv6_1);
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
    if (tcp_option_timestamp.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.timestamp_valid, TRUE);
    }
    if (ipv4_1.valid == TRUE) {
        modify_field(p4_to_p4plus_tcp_proxy.ecn, ipv4_1.diffserv, 0x3);
    } else {
        if (ipv6_1.valid == TRUE) {
            modify_field(p4_to_p4plus_tcp_proxy.ecn, ipv6_1.trafficClass, 0x3);
        }
    }

    add_header(capri_rxdma_intrinsic);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_TCP_PROXY_HDR_SZ));
}

@pragma stage 5
@pragma index_table
table ingress_to_rxdma {
    reads {
        control_metadata.p4plus_app_id  : exact;
    }
    actions {
        ingress_to_rxdma;
        classic_nic_app;
        redirect_to_arm;
        p4plus_app_tcp_proxy;
    }
    size : APP_TABLE_SIZE;
}

control ingress_to_rxdma {
    apply(ingress_to_rxdma);
}

/*****************************************************************************/
/* Egress pipeline to uplink/ingress                                         */
/*****************************************************************************/
action egress_to_uplink() {
    modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    if (control_metadata.local_switching == TRUE) {
        modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
        remove_header(predicate_header);
        add_header(predicate_header);
        bit_xor(predicate_header.direction, control_metadata.direction, 1);
    } else {
        modify_field(capri_intrinsic.tm_span_session,
                     control_metadata.mirror_session);
        if (capri_intrinsic.tm_oport == TM_PORT_DMA) {
            add_header(capri_p4_intrinsic);
            add_header(capri_rxdma_intrinsic);
            add_header(p4_to_p4plus_classic_nic);
            add_header(p4_to_p4plus_classic_nic_ip);

            modify_field(p4_to_p4plus_classic_nic.packet_len,
                         capri_p4_intrinsic.packet_len);
            modify_field(p4_to_p4plus_classic_nic.p4plus_app_id,
                         P4PLUS_APPTYPE_CLASSIC_NIC);
            modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                         (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                          P4PLUS_CLASSIC_NIC_HDR_SZ));
        }
        remove_header(predicate_header);
    }
    remove_header(capri_txdma_intrinsic);
    remove_header(txdma_to_p4e_header);
    remove_header(p4e_apollo_i2e);
}

@pragma stage 5
table egress_to_uplink {
    actions {
        egress_to_uplink;
    }
}

control egress_to_uplink {
    apply(egress_to_uplink);
}
