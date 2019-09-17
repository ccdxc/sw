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

action tunneled_ipv4_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV4);
    modify_field(key_metadata.src, ipv4_2.srcAddr);
    modify_field(key_metadata.dst, ipv4_2.dstAddr);
    modify_field(key_metadata.ipv4_src, ipv4_2.srcAddr);
    modify_field(key_metadata.ipv4_dst, ipv4_2.dstAddr);
    modify_field(key_metadata.proto, ipv4_2.protocol);
}

action tunneled_ipv6_packet() {
    modify_field(key_metadata.ktype, KEY_TYPE_IPV6);
    modify_field(key_metadata.src, ipv6_2.srcAddr);
    modify_field(key_metadata.dst, ipv6_2.dstAddr);
    modify_field(key_metadata.proto, ipv6_2.nextHdr);
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

/******************************************************************************/
/* Ingress init                                                               */
/******************************************************************************/
action process_ingress_recirc() {
    if (ingress_recirc.valid == TRUE) {
        modify_field(control_metadata.local_mapping_ohash_lkp,
                     ~ingress_recirc.local_mapping_done);
        modify_field(control_metadata.flow_ohash_lkp,
                     ~ingress_recirc.flow_done);
        modify_field(capri_p4_intrinsic.recirc, FALSE);
    }
}

action init_config() {
    process_ingress_recirc();
    subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.frame_size,
             offset_metadata.l2_1);
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
    if ((control_metadata.rx_packet == TRUE) and (vxlan_1.valid == TRUE)) {
        apply(key_tunneled);
    }
    apply(init_config);
}
