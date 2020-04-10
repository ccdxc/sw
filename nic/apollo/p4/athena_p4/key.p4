/******************************************************************************/
/* Key derivation tables                                                      */
/******************************************************************************/
action native_ipv4_packet() {
    modify_field(key_metadata.ktype, P4_KEY_TYPE_IPV4);

    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(key_metadata.src, ipv4_1.srcAddr);
        modify_field(key_metadata.dst, ipv4_1.dstAddr);
        if (udp_1.valid == TRUE) {
            modify_field(key_metadata.sport, udp_1.srcPort);
            modify_field(key_metadata.dport, udp_1.dstPort);
        }
        else {
            modify_field(key_metadata.sport, key_metadata.parsed_sport);
            modify_field(key_metadata.dport, key_metadata.parsed_dport);
        }
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        modify_field(key_metadata.dst, ipv4_1.srcAddr);
        modify_field(key_metadata.src, ipv4_1.dstAddr);
        if (udp_1.valid == TRUE) {
            modify_field(key_metadata.dport, udp_1.srcPort);
            modify_field(key_metadata.sport, udp_1.dstPort);
        }
        else {
            if (tcp.valid == TRUE) {
                modify_field(key_metadata.dport, key_metadata.parsed_sport);
                modify_field(key_metadata.sport, key_metadata.parsed_dport);
            }
            else {
                modify_field(key_metadata.sport, key_metadata.parsed_sport);
                modify_field(key_metadata.dport, key_metadata.parsed_dport);
            }
        }
    }
    modify_field(key_metadata.proto, ipv4_1.protocol);
    //modify_field(p4_to_p4plus_tcp_proxy.payload_len,
    //             (ipv4_1.totalLen - ((ipv4_1.ihl + tcp.dataOffset) * 4)));
}

action native_ipv6_packet() {
    modify_field(key_metadata.ktype, P4_KEY_TYPE_IPV6);
    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(key_metadata.src, ipv6_1.srcAddr);
        modify_field(key_metadata.dst, ipv6_1.dstAddr);
        if (udp_1.valid == TRUE) {
            modify_field(key_metadata.sport, udp_1.srcPort);
            modify_field(key_metadata.dport, udp_1.dstPort);
        }
        else {
            modify_field(key_metadata.sport, key_metadata.parsed_sport);
            modify_field(key_metadata.dport, key_metadata.parsed_dport);
        }
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        modify_field(key_metadata.dst, ipv6_1.srcAddr);
        modify_field(key_metadata.src, ipv6_1.dstAddr);
        if (udp_1.valid == TRUE) {
            modify_field(key_metadata.dport, udp_1.srcPort);
            modify_field(key_metadata.sport, udp_1.dstPort);
        }
        else {
            if (tcp.valid == TRUE) {
                modify_field(key_metadata.dport, key_metadata.parsed_sport);
                modify_field(key_metadata.sport, key_metadata.parsed_dport);
            }
            else {
                modify_field(key_metadata.sport, key_metadata.parsed_sport);
                modify_field(key_metadata.dport, key_metadata.parsed_dport);
            }
        }
    }
    modify_field(key_metadata.proto, ipv6_1.nextHdr);
}

action native_nonip_packet() {
        /* Skip DNAT and Flow lookup */
        modify_field(control_metadata.skip_dnat_lkp, TRUE);
        modify_field(control_metadata.skip_flow_lkp, TRUE);
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(ingress_recirc_header.dnat_done, TRUE);
}

action tunneled_ipv4_packet() {
    modify_field(key_metadata.ktype, P4_KEY_TYPE_IPV4);
    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(key_metadata.src, ipv4_2.srcAddr);
        modify_field(key_metadata.dst, ipv4_2.dstAddr);
        if (udp_2.valid == TRUE) {
            modify_field(key_metadata.sport, udp_2.srcPort);
            modify_field(key_metadata.dport, udp_2.dstPort);
        }
        else {
            modify_field(key_metadata.sport, key_metadata.parsed_sport);
            modify_field(key_metadata.dport, key_metadata.parsed_dport);
        }
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        modify_field(key_metadata.dst, ipv4_2.srcAddr);
        modify_field(key_metadata.src, ipv4_2.dstAddr);
        if (udp_2.valid == TRUE) {
            modify_field(key_metadata.dport, udp_2.srcPort);
            modify_field(key_metadata.sport, udp_2.dstPort);
        }
        else {
            if (tcp.valid == TRUE) {
                modify_field(key_metadata.dport, key_metadata.parsed_sport);
                modify_field(key_metadata.sport, key_metadata.parsed_dport);
            }
            else {
                modify_field(key_metadata.sport, key_metadata.parsed_sport);
                modify_field(key_metadata.dport, key_metadata.parsed_dport);
            }
        }
    }
    modify_field(key_metadata.proto, ipv4_2.protocol);
}

action tunneled_ipv6_packet() {
    modify_field(key_metadata.ktype, P4_KEY_TYPE_IPV6);
    if (control_metadata.direction == TX_FROM_HOST) {
        modify_field(key_metadata.src, ipv6_2.srcAddr);
        modify_field(key_metadata.dst, ipv6_2.dstAddr);
        if (udp_2.valid == TRUE) {
            modify_field(key_metadata.sport, udp_2.srcPort);
            modify_field(key_metadata.dport, udp_2.dstPort);
        }
        else {
            modify_field(key_metadata.sport, key_metadata.parsed_sport);
            modify_field(key_metadata.dport, key_metadata.parsed_dport);
        }
    }
    if (control_metadata.direction == RX_FROM_SWITCH) {
        modify_field(key_metadata.dst, ipv6_2.srcAddr);
        modify_field(key_metadata.src, ipv6_2.dstAddr);
        if (udp_2.valid == TRUE) {
            modify_field(key_metadata.dport, udp_2.srcPort);
            modify_field(key_metadata.sport, udp_2.dstPort);
        }
        else {
            if (tcp.valid == TRUE) {
                modify_field(key_metadata.dport, key_metadata.parsed_sport);
                modify_field(key_metadata.sport, key_metadata.parsed_dport);
            }
            else {
                modify_field(key_metadata.sport, key_metadata.parsed_sport);
                modify_field(key_metadata.dport, key_metadata.parsed_dport);
            }
        }
    }
    modify_field(key_metadata.proto, ipv6_2.nextHdr);
}

action tunneled_nonip_packet()  {
    /* Skip DNAT and Flow lookup */
    modify_field(control_metadata.skip_dnat_lkp, TRUE);
    modify_field(control_metadata.skip_flow_lkp, TRUE);
    modify_field(ingress_recirc_header.flow_done, TRUE);
    modify_field(ingress_recirc_header.dnat_done, TRUE);
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

        modify_field(control_metadata.flow_ohash_lkp, TRUE);
        modify_field(control_metadata.dnat_ohash_lkp, TRUE);
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
    } else {
        modify_field(capri_intrinsic.tm_oq, capri_intrinsic.tm_iq);
    }

    if (control_metadata.skip_flow_lkp == TRUE) {
        modify_field(control_metadata.flow_miss, TRUE);
    }

    /* Drop packet on PB errors */
    modify_field(capri_intrinsic.drop, capri_intrinsic.hw_error);
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
