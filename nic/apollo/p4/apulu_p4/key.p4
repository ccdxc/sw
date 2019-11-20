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
    } else {
        modify_field(key_metadata.sport, key_metadata.parsed_sport);
        modify_field(key_metadata.dport, key_metadata.parsed_dport);
    }
    modify_field(key_metadata.proto, ipv4_1.protocol);
}

action native_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_1.srcAddr);
    modify_field(key_metadata.dst, ipv6_1.dstAddr);
    if (udp_1.valid == TRUE) {
        modify_field(key_metadata.sport, udp_1.srcPort);
        modify_field(key_metadata.dport, udp_1.dstPort);
    } else {
        modify_field(key_metadata.sport, key_metadata.parsed_sport);
        modify_field(key_metadata.dport, key_metadata.parsed_dport);
    }
    modify_field(key_metadata.proto, ipv6_1.nextHdr);
}

action native_nonip_packet() {
    if (ctag_1.valid == TRUE) {
        modify_field(key_metadata.dport, ctag_1.etherType);
    } else {
        modify_field(key_metadata.dport, ethernet_1.etherType);
    }
    if (arp.valid == TRUE) {
        modify_field(key_metadata.sport, arp.opcode);
    }
    modify_field(key_metadata.ktype, KEY_TYPE_MAC);
    modify_field(key_metadata.src, ethernet_1.srcAddr);
    modify_field(key_metadata.dst, ethernet_1.dstAddr);
}

action tunneled_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_2.srcAddr);
    modify_field(key_metadata.dst, ipv4_2.dstAddr);
    modify_field(key_metadata.ipv4_src, ipv4_2.srcAddr);
    modify_field(key_metadata.ipv4_dst, ipv4_2.dstAddr);
    modify_field(key_metadata.proto, ipv4_2.protocol);
    modify_field(key_metadata.sport, key_metadata.parsed_sport);
    modify_field(key_metadata.dport, key_metadata.parsed_dport);
}

action tunneled_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_2.srcAddr);
    modify_field(key_metadata.dst, ipv6_2.dstAddr);
    modify_field(key_metadata.proto, ipv6_2.nextHdr);
    modify_field(key_metadata.sport, key_metadata.parsed_sport);
    modify_field(key_metadata.dport, key_metadata.parsed_dport);
}

action tunneled_nonip_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_MAC);
    modify_field(key_metadata.src, ethernet_2.srcAddr);
    modify_field(key_metadata.dst, ethernet_2.dstAddr);
    modify_field(key_metadata.dport, ethernet_2.etherType);
}

action native_packet() {
    if (ipv4_1.valid == TRUE) {
        native_ipv4_packet();
    } else {
        if (ipv6_1.valid == TRUE) {
            native_ipv6_packet();
        } else {
            native_nonip_packet();
        }
    }
}

@pragma stage 0
table key_native {
    actions {
        native_packet;
    }
}

action tunneled_packet() {
    if (ipv4_2.valid == TRUE) {
        tunneled_ipv4_packet();
    } else {
        if (ipv6_2.valid == TRUE) {
            tunneled_ipv6_packet();
        } else {
            tunneled_nonip_packet();
        }
    }
}

@pragma stage 1
table key_tunneled {
    actions {
        tunneled_packet;
    }
}

control key_init {
    apply(key_native);
}
