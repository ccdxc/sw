/*****************************************************************************/
/* NAT                                                                       */
/*****************************************************************************/
action nat_rewrite(nat_ip) {
    if (nat_metadata.xlate_idx == 0) {
        // return;
    }

    if (control_metadata.direction == TX_FROM_HOST) {
        if (TX_REWRITE(rewrite_metadata.flags, SRC_IP, FROM_PUBLIC) or
            TX_REWRITE(rewrite_metadata.flags, SRC_IP, FROM_SERVICE)) {
            if (ipv4_1.valid == TRUE) {
                modify_field(ipv4_1.srcAddr, nat_ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.srcAddr, nat_ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (TX_REWRITE(rewrite_metadata.flags, SRC_IP, FROM_SERVICE)) {
            if (udp_1.valid == TRUE) {
                modify_field(udp_1.srcPort, p4e_i2e.service_xlate_port);
            }
            if (tcp.valid == TRUE) {
                modify_field(tcp.srcPort, p4e_i2e.service_xlate_port);
            }
        }
        if (TX_REWRITE(rewrite_metadata.flags, DST_IP, FROM_SESSION)) {
            if (ipv4_1.valid == TRUE) {
                modify_field(ipv4_1.dstAddr, rewrite_metadata.ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.dstAddr, rewrite_metadata.ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (TX_REWRITE(rewrite_metadata.flags, DPORT, FROM_SESSION)) {
            if (udp_1.valid == TRUE) {
                modify_field(udp_1.dstPort, rewrite_metadata.l4port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (tcp.valid == TRUE) {
                modify_field(tcp.dstPort, rewrite_metadata.l4port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (TX_REWRITE(rewrite_metadata.flags, ENCAP, VXLAN)) {
            modify_field(rewrite_metadata.encap_src_ip, nat_ip);
        }
    } else {
        if (RX_REWRITE(rewrite_metadata.flags, SRC_IP, FROM_SESSION)) {
            if (ipv4_1.valid == TRUE) {
                modify_field(ipv4_1.srcAddr, rewrite_metadata.ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.srcAddr, rewrite_metadata.ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (RX_REWRITE(rewrite_metadata.flags, SPORT, FROM_SESSION)) {
            if (udp_1.valid == TRUE) {
                modify_field(udp_1.srcPort, rewrite_metadata.l4port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (tcp.valid == TRUE) {
                modify_field(tcp.srcPort, rewrite_metadata.l4port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (RX_REWRITE(rewrite_metadata.flags, DST_IP, FROM_CA) or
            RX_REWRITE(rewrite_metadata.flags, DST_IP, FROM_SERVICE)) {
            if (ipv4_1.valid == TRUE) {
                modify_field(ipv4_1.dstAddr, nat_ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.dstAddr, nat_ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (RX_REWRITE(rewrite_metadata.flags, DST_IP, FROM_SERVICE)) {
            if (udp_1.valid == TRUE) {
                modify_field(udp_1.dstPort, p4e_i2e.service_xlate_port);
            }
            if (tcp.valid == TRUE) {
                modify_field(tcp.dstPort, p4e_i2e.service_xlate_port);
            }
        }
    }
}

@pragma stage 1
@pragma hbm_table
@pragma index_table
table nat {
    reads {
        nat_metadata.xlate_idx  : exact;
    }
    actions {
        nat_rewrite;
    }
    size : NAT_TABLE_SIZE;
}

/*****************************************************************************/
/* 4to6/6to4 translation                                                     */
/*****************************************************************************/
action local_46_info(prefix) {
    if (control_metadata.direction == TX_FROM_HOST) {
        if (TX_REWRITE(rewrite_metadata.flags, SRC_IP, FROM_46)) {
            if (ctag_1.valid == TRUE) {
                modify_field(ctag_1.etherType, ETHERTYPE_IPV6);
            } else {
                modify_field(ethernet_1.etherType, ETHERTYPE_IPV6);
            }
            remove_header(ipv4_1);
            add_header(ipv6_1);
            modify_field(ipv6_1.version, 6);
            modify_field(ipv6_1.trafficClass, ipv4_1.diffserv);
            modify_field(ipv6_1.payloadLen, ipv4_1.totalLen - 20);
            modify_field(ipv6_1.nextHdr, ipv4_1.protocol);
            modify_field(ipv6_1.hopLimit, ipv4_1.ttl);
            modify_field(ipv6_1.srcAddr, prefix | ipv4_1.srcAddr);
            modify_field(ipv6_1.dstAddr, rewrite_metadata.ip | ipv4_1.dstAddr);
            add(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len, 20);
            modify_field(control_metadata.update_checksum, TRUE);
        }
    } else {
        if (RX_REWRITE(rewrite_metadata.flags, SRC_IP, FROM_64)) {
            if (ctag_1.valid == TRUE) {
                modify_field(ctag_1.etherType, ETHERTYPE_IPV4);
            } else {
                modify_field(ethernet_1.etherType, ETHERTYPE_IPV4);
            }
            remove_header(ipv6_1);
            add_header(ipv4_1);
            modify_field(ipv4_1.version, 4);
            modify_field(ipv4_1.ihl, 5);
            modify_field(ipv4_1.diffserv, ipv6_1.trafficClass);
            modify_field(ipv4_1.totalLen, ipv6_1.payloadLen + 20);
            modify_field(ipv4_1.ttl, ipv6_1.hopLimit);
            modify_field(ipv4_1.protocol, ipv6_1.nextHdr);
            modify_field(ipv4_1.srcAddr, ipv6_1.srcAddr);
            modify_field(ipv4_1.dstAddr, ipv6_1.dstAddr);
            subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len, 20);
            modify_field(control_metadata.update_checksum, TRUE);
        }
    }
}

@pragma stage 2
@pragma_index_table
table local_46_mapping {
    reads {
        p4e_i2e.ca6_xlate_idx   : exact;
    }
    actions {
        local_46_info;
    }
    size : LOCAL_46_MAPPING_TABLE_SIZE;
}

control nat {
    apply(nat);
    apply(local_46_mapping);
}
