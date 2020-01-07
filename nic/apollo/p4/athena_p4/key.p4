/******************************************************************************/
/* Key derivation tables                                                      */
/******************************************************************************/
action native_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_1.srcAddr);
    modify_field(key_metadata.dst, ipv4_1.dstAddr);
    if (ctag_1.valid == TRUE) {
        modify_field(key_metadata.vlan, ctag_1.vid);
    }
    if (udp_1.valid == TRUE) {
        modify_field(key_metadata.sport, udp_1.srcPort);
        modify_field(key_metadata.dport, udp_1.dstPort);
    }
    if (tcp.valid == TRUE) {
        modify_field(key_metadata.sport, tcp.srcPort);
        modify_field(key_metadata.dport, tcp.dstPort);
    }
    //modify_field(p4_to_p4plus_tcp_proxy.payload_len,
    //             (ipv4_1.totalLen - ((ipv4_1.ihl + tcp.dataOffset) * 4)));
    modify_field(key_metadata.proto, ipv4_1.protocol);
    // FIXME: The follwing exceeds K+I beyond 512b
    modify_field(key_metadata.ingress_port, control_metadata.direction);
    modify_field(key_metadata.tcp_flags, tcp.flags);
}

action native_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_1.srcAddr);
    modify_field(key_metadata.dst, ipv6_1.dstAddr);
    if (ctag_1.valid == TRUE) {
        modify_field(key_metadata.vlan, ctag_1.vid);
    }
    if (udp_1.valid == TRUE) {
        modify_field(key_metadata.sport, udp_1.srcPort);
        modify_field(key_metadata.dport, udp_1.dstPort);
    }
    if (tcp.valid == TRUE) {
        modify_field(key_metadata.sport, tcp.srcPort);
        modify_field(key_metadata.dport, tcp.dstPort);
    }
    modify_field(key_metadata.proto, ipv6_1.nextHdr);
    // FIXME: The follwing exceeds K+I beyond 512b
    modify_field(key_metadata.ingress_port, control_metadata.direction);
    modify_field(key_metadata.tcp_flags, tcp.flags);
}

action native_nonip_packet() {
    modify_field(control_metadata.skip_flow_lkp, TRUE);
}

action tunneled_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_2.srcAddr);
    modify_field(key_metadata.dst, ipv4_2.dstAddr);
    modify_field(key_metadata.proto, ipv4_2.protocol);
    if (udp_2.valid == TRUE) {
        modify_field(key_metadata.sport, udp_2.srcPort);
        modify_field(key_metadata.dport, udp_2.dstPort);
    }
    if (tcp.valid == TRUE) {
        modify_field(key_metadata.sport, tcp.srcPort);
        modify_field(key_metadata.dport, tcp.dstPort);
    }
    if (mpls_label1_1.valid == TRUE) {
        modify_field(key_metadata.tenant_id, mpls_label1_1.label);
    }
    if (mpls_label2_1.valid == TRUE) {
        modify_field(key_metadata.tenant_id, mpls_label2_1.label);
    }
    modify_field(key_metadata.ingress_port, control_metadata.direction);
    modify_field(key_metadata.tcp_flags, tcp.flags);
}

action tunneled_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_2.srcAddr);
    modify_field(key_metadata.dst, ipv6_2.dstAddr);
    modify_field(key_metadata.proto, ipv6_2.nextHdr);
    if (udp_2.valid == TRUE) {
        modify_field(key_metadata.sport, udp_2.srcPort);
        modify_field(key_metadata.dport, udp_2.dstPort);
    }
    if (tcp.valid == TRUE) {
        modify_field(key_metadata.sport, tcp.srcPort);
        modify_field(key_metadata.dport, tcp.dstPort);
    }
    modify_field(key_metadata.ingress_port, control_metadata.direction);
    modify_field(key_metadata.tcp_flags, tcp.flags);
}

action tunneled_nonip_packet()  {
    modify_field(control_metadata.skip_flow_lkp, TRUE);
}

@pragma stage 0
table key_native {
    reads {
        ipv4_1.valid                : ternary;
        ipv6_1.valid                : ternary;
        ethernet_2.valid            : ternary;
        ipv4_2.valid                : ternary;
        ipv6_2.valid                : ternary;
    }
    actions {
        nop;
        native_ipv4_packet;
        native_ipv6_packet;
        native_nonip_packet;
    }
    size : KEY_MAPPING_TABLE_SIZE;
}

@pragma stage 0
table key_tunneled {
    reads {
        ipv4_1.valid                : ternary;
        ipv6_1.valid                : ternary;
        ethernet_2.valid            : ternary;
        ipv4_2.valid                : ternary;
        ipv6_2.valid                : ternary;
    }
    actions {
        nop;
        tunneled_ipv4_packet;
        tunneled_ipv6_packet;
        tunneled_nonip_packet;
    }
    size : KEY_MAPPING_TABLE_SIZE;
}

action ingress_recirc_header_info() {
    if (ingress_recirc_header.valid == TRUE) {

        modify_field(control_metadata.flow_ohash_lkp,
                     ~ingress_recirc_header.flow_done);
        modify_field(capri_p4_intrinsic.recirc, FALSE);
    }
}

action init_config() {
    ingress_recirc_header_info();
    /* TODO: Account only the user packet for throttling */
    subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.frame_size,
             offset_metadata.l2_1);
    subtract(p4i_to_p4e_header.packet_len, capri_p4_intrinsic.frame_size,
             offset_metadata.l2_1);
    if (capri_intrinsic.tm_oq != TM_P4_RECIRC_QUEUE) {
        modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    }

    if (control_metadata.skip_flow_lkp == TRUE) {
        modify_field(control_metadata.flow_miss, TRUE);
    }
}

@pragma stage 0
table init_config {
    actions {
        init_config;
    }
}

control key_init {
    apply(key_native);
    apply(key_tunneled);
    apply(init_config);
}
