/*****************************************************************************/
/* Ingress pipeline to egress pipeline                                       */
/*****************************************************************************/
action ingress_to_egress() {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
    add_header(capri_p4_intrinsic);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
    modify_field(capri_intrinsic.tm_span_session,
                 control_metadata.mirror_session);
    add_header(predicate_header);
    modify_field(predicate_header.direction, control_metadata.direction);
    add_header(txdma_to_p4e);
    add_header(p4i_i2e);
    remove_header(service_header);
    if (vxlan_1.valid == TRUE) {
        remove_header(ethernet_1);
        remove_header(ctag_1);
        remove_header(ipv4_1);
        remove_header(ipv6_1);
        remove_header(udp_1);
        remove_header(vxlan_1);
    }
    if (vxlan_2.valid == TRUE) {
        remove_header(ethernet_2);
        remove_header(ctag_2);
        remove_header(ipv4_2);
        remove_header(ipv6_2);
        remove_header(udp_2);
        remove_header(vxlan_2);
    }
}

action ingress_to_artemis() {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(capri_intrinsic.lif, ARTEMIS_SERVICE_LIF);
    add_header(capri_p4_intrinsic);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
    modify_field(capri_intrinsic.tm_span_session,
                 control_metadata.mirror_session);
    add_header(capri_rxdma_intrinsic);
    add_header(p4_to_rxdma);
    add_header(predicate_header);
            // Splitter offset should point to here
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ +
                  CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  ARTEMIS_P4_TO_RXDMA_HDR_SZ + ARTEMIS_PREDICATE_HDR_SZ));
    add_header(p4i_i2e);
    remove_header(service_header);

    modify_field(p4_to_rxdma.table3_valid, TRUE);
    modify_field(p4_to_rxdma.direction, control_metadata.direction);
    modify_field(predicate_header.direction, control_metadata.direction);
}

action ingress_to_classic_nic() {
    add_header(capri_p4_intrinsic);
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
                 P4PLUS_APPTYPE_CLASSIC_NIC);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_CLASSIC_NIC_HDR_SZ));
}

action ingress_to_uplink() {
    add_header(capri_p4_intrinsic);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
}

action add_p4_to_arm_header() {
    add_header(p4_to_arm);
    modify_field(p4_to_arm.packet_len, capri_p4_intrinsic.packet_len);
    modify_field(p4_to_arm.flow_hash, p4i_i2e.entropy_hash);
    modify_field(offset_metadata.l2_1, offset_metadata.l2_1);
    modify_field(offset_metadata.l2_2, offset_metadata.l2_2);
    modify_field(offset_metadata.l3_1, offset_metadata.l3_1);
    modify_field(offset_metadata.l3_2, offset_metadata.l3_2);
    modify_field(offset_metadata.l4_1, offset_metadata.l4_1);
    modify_field(offset_metadata.l4_2, offset_metadata.l4_2);
    modify_field(p4_to_arm.payload_offset, offset_metadata.payload_offset);

    modify_field(scratch_metadata.cpu_flags, 0);
    bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
           (control_metadata.direction << ARTEMIS_CPU_FLAGS_DIRECTION_BIT_POS));
    if (ctag_1.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               ARTEMIS_CPU_FLAGS_VLAN_VALID);
    }
    if (ipv4_1.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               ARTEMIS_CPU_FLAGS_IPV4_1_VALID);
    } else {
        if (ipv6_1.valid == TRUE) {
            bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
                   ARTEMIS_CPU_FLAGS_IPV6_1_VALID);
        }
    }
    if (ethernet_2.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               ARTEMIS_CPU_FLAGS_ETH_2_VALID);
    }
    if (ipv4_2.valid == TRUE) {
        bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
               ARTEMIS_CPU_FLAGS_IPV4_2_VALID);
    } else {
        if (ipv6_2.valid == TRUE) {
            bit_or(scratch_metadata.cpu_flags, scratch_metadata.cpu_flags,
                   ARTEMIS_CPU_FLAGS_IPV6_2_VALID);
        }
    }
    modify_field(p4_to_arm.flags, scratch_metadata.cpu_flags);
}

action ingress_to_arm() {
    remove_header(capri_txdma_intrinsic);
    remove_header(predicate_header);
    add_p4_to_arm_header();
    ingress_to_classic_nic();
    modify_field(p4_to_p4plus_classic_nic.packet_len,
                 capri_p4_intrinsic.packet_len + ARTEMIS_P4_TO_ARM_HDR_SZ);
}

@pragma stage 5
@pragma index_table
table inter_pipe_ingress {
    reads {
        control_metadata.pipe_id    : exact;
    }
    actions {
        ingress_to_egress;
        ingress_to_artemis;
        ingress_to_classic_nic;
        ingress_to_arm;
    }
    size : INTER_PIPE_TABLE_SIZE;
}

control inter_pipe_ingress {
    apply(inter_pipe_ingress);
}
