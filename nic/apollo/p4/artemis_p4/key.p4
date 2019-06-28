/******************************************************************************/
/* Key derivation tables                                                      */
/******************************************************************************/
action native_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_1.srcAddr);
    modify_field(key_metadata.dst, ipv4_1.dstAddr);
    modify_field(key_metadata.ipv4_src, ipv4_1.srcAddr);
    modify_field(key_metadata.ipv4_dst, ipv4_1.dstAddr);
    if (udp_1.valid == TRUE) {
        modify_field(key_metadata.sport, udp_1.srcPort);
        modify_field(key_metadata.dport, udp_1.dstPort);
    }
    modify_field(key_metadata.proto, ipv4_1.protocol);
    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(key_metadata.mapping_ip, ipv4_1.srcAddr);
        modify_field(key_metadata.mapping_port, key_metadata.sport);
    } else {
        modify_field(key_metadata.mapping_ip, ipv4_1.dstAddr);
        modify_field(key_metadata.mapping_port, key_metadata.dport);
    }
}

action native_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_1.srcAddr);
    modify_field(key_metadata.dst, ipv6_1.dstAddr);
    if (udp_1.valid == TRUE) {
        modify_field(key_metadata.sport, udp_1.srcPort);
        modify_field(key_metadata.dport, udp_1.dstPort);
    }
    modify_field(key_metadata.proto, ipv6_1.nextHdr);
    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(key_metadata.mapping_ip, ipv6_1.srcAddr);
        modify_field(key_metadata.mapping_port, key_metadata.sport);
    } else {
        modify_field(key_metadata.mapping_ip, ipv6_1.dstAddr);
        modify_field(key_metadata.mapping_port, key_metadata.dport);
    }
}

action native_nonip_packet() {
    if (ctag_1.valid == TRUE) {
        modify_field(key_metadata.dport, ctag_1.etherType);
    } else {
        modify_field(key_metadata.dport, ethernet_1.etherType);
    }
    modify_field(key_metadata.ktype, KEY_TYPE_MAC);
    modify_field(key_metadata.src, ethernet_1.srcAddr);
    modify_field(key_metadata.dst, ethernet_1.dstAddr);
}

action set_tep1_dst() {
    if (ipv4_1.valid == TRUE) {
        modify_field(key_metadata.mapping_ip, ipv4_1.dstAddr);
    } else {
        if (ipv6_1.valid == TRUE) {
            modify_field(key_metadata.mapping_ip, ipv6_1.dstAddr);
        }
    }
}

action tunneled_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_2.srcAddr);
    modify_field(key_metadata.dst, ipv4_2.dstAddr);
    modify_field(key_metadata.ipv4_src, ipv4_2.srcAddr);
    modify_field(key_metadata.ipv4_dst, ipv4_2.dstAddr);
    if (udp_2.valid == TRUE) {
        modify_field(key_metadata.sport, udp_2.srcPort);
        modify_field(key_metadata.dport, udp_2.dstPort);
    }
    modify_field(key_metadata.proto, ipv4_2.protocol);
    modify_field(key_metadata.mapping_ip2, ipv4_2.dstAddr);
    modify_field(key_metadata.mapping_port, key_metadata.dport);
}

action tunneled_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_2.srcAddr);
    modify_field(key_metadata.dst, ipv6_2.dstAddr);
    if (udp_2.valid == TRUE) {
        modify_field(key_metadata.sport, udp_2.srcPort);
        modify_field(key_metadata.dport, udp_2.dstPort);
    }
    modify_field(key_metadata.proto, ipv6_2.nextHdr);
    modify_field(key_metadata.mapping_ip2, ipv6_2.dstAddr);
    modify_field(key_metadata.mapping_port, key_metadata.dport);
}

action tunneled_nonip_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_MAC);
    modify_field(key_metadata.src, ethernet_2.srcAddr);
    modify_field(key_metadata.dst, ethernet_2.dstAddr);
    modify_field(key_metadata.dport, ethernet_2.etherType);
}

action set_tep2_dst() {
    if (ipv4_2.valid == TRUE) {
        modify_field(tunnel_metadata.tep2_dst, ipv4_2.dstAddr);
    } else {
        if (ipv6_2.valid == TRUE) {
            modify_field(tunnel_metadata.tep2_dst, ipv6_2.dstAddr);
        }
    }
}

action tunneled2_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_3.srcAddr);
    modify_field(key_metadata.dst, ipv4_3.dstAddr);
    modify_field(key_metadata.ipv4_src, ipv4_3.srcAddr);
    modify_field(key_metadata.ipv4_dst, ipv4_3.dstAddr);
    modify_field(key_metadata.proto, ipv4_3.protocol);
    modify_field(key_metadata.mapping_ip2, ipv4_3.dstAddr);
    modify_field(key_metadata.mapping_port, key_metadata.dport);
}

action tunneled2_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_3.srcAddr);
    modify_field(key_metadata.dst, ipv6_3.dstAddr);
    modify_field(key_metadata.proto, ipv6_3.nextHdr);
    modify_field(key_metadata.mapping_ip2, ipv6_3.dstAddr);
    modify_field(key_metadata.mapping_port, key_metadata.dport);
}

action tunneled2_nonip_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_MAC);
    modify_field(key_metadata.src, ethernet_3.srcAddr);
    modify_field(key_metadata.dst, ethernet_3.dstAddr);
    modify_field(key_metadata.dport, ethernet_3.etherType);
}

@pragma stage 0
table key_native {
    reads {
        ipv4_1.valid        : ternary;
        ipv6_1.valid        : ternary;
        ethernet_2.valid    : ternary;
        ipv4_2.valid        : ternary;
        ipv6_2.valid        : ternary;
        ethernet_3.valid    : ternary;
        ipv4_3.valid        : ternary;
        ipv6_3.valid        : ternary;
    }
    actions {
        nop;
        set_tep1_dst;
        native_ipv4_packet;
        native_ipv6_packet;
        native_nonip_packet;
    }
    size : KEY_MAPPING_TABLE_SIZE;
}

@pragma stage 0
table key_tunneled {
    reads {
        ipv4_1.valid        : ternary;
        ipv6_1.valid        : ternary;
        ethernet_2.valid    : ternary;
        ipv4_2.valid        : ternary;
        ipv6_2.valid        : ternary;
        ethernet_3.valid    : ternary;
        ipv4_3.valid        : ternary;
        ipv6_3.valid        : ternary;
    }
    actions {
        nop;
        set_tep2_dst;
        tunneled_ipv4_packet;
        tunneled_ipv6_packet;
        tunneled_nonip_packet;
    }
    size : KEY_MAPPING_TABLE_SIZE;
}

@pragma stage 1
table key_tunneled2 {
    reads {
        ipv4_1.valid        : ternary;
        ipv6_1.valid        : ternary;
        ethernet_2.valid    : ternary;
        ipv4_2.valid        : ternary;
        ipv6_2.valid        : ternary;
        ethernet_3.valid    : ternary;
        ipv4_3.valid        : ternary;
        ipv6_3.valid        : ternary;
    }
    actions {
        nop;
        tunneled2_ipv4_packet;
        tunneled2_ipv6_packet;
        tunneled2_nonip_packet;
    }
    size : KEY_MAPPING_TABLE_SIZE;
}

action process_ingress_recirc() {
    if (ingress_recirc.valid == TRUE) {
        modify_field(control_metadata.flow_ohash_lkp, ~ingress_recirc.flow_done);
        modify_field(capri_p4_intrinsic.recirc, FALSE);
    }
}

action init_config() {
    process_ingress_recirc();
    if (cps_blob.valid == TRUE) {
        subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.frame_size,
                 (offset_metadata.l2_1 + 0x100));
    } else {
        subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.frame_size,
                 offset_metadata.l2_1);
    }
    if (capri_intrinsic.tm_oq != TM_P4_RECIRC_QUEUE) {
        modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    } else {
        modify_field(capri_intrinsic.tm_oq, capri_intrinsic.tm_iq);
    }
}

@pragma stage 1
table init_config {
    actions {
        init_config;
    }
}

control key_init {
    apply(key_native);
    apply(key_tunneled);
    apply(key_tunneled2);
    apply(init_config);
}
