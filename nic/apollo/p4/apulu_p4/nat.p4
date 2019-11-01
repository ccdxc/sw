/*****************************************************************************/
/* NAT                                                                       */
/*****************************************************************************/
action nat_rewrite(ip, port) {
    if (control_metadata.rx_packet == FALSE) {
        if (TX_REWRITE(rewrite_metadata.flags, SIP, FROM_NAT)) {
            if (ipv4_1.valid == TRUE) {
                modify_field(ipv4_1.srcAddr, ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.srcAddr, ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (TX_REWRITE(rewrite_metadata.flags, SPORT, FROM_NAT)) {
            if (udp_1.valid == TRUE) {
                modify_field(udp_1.srcPort, port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (tcp.valid == TRUE) {
                modify_field(tcp.srcPort, port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
    } else {
        if (RX_REWRITE(rewrite_metadata.flags, DIP, FROM_NAT)) {
            if (ipv4_1.valid == TRUE) {
                modify_field(ipv4_1.dstAddr, ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.dstAddr, ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (RX_REWRITE(rewrite_metadata.flags, DPORT, FROM_NAT)) {
            if (udp_1.valid == TRUE) {
                modify_field(udp_1.dstPort, port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (tcp.valid == TRUE) {
                modify_field(tcp.dstPort, port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
    }
}

action nat2_rewrite(ip, port) {
    if (control_metadata.rx_packet == FALSE) {
        if (TX_REWRITE(rewrite_metadata.flags, DIP, FROM_NAT)) {
            if (ipv4_1.valid == TRUE) {
                modify_field(ipv4_1.dstAddr, ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.dstAddr, ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (TX_REWRITE(rewrite_metadata.flags, DPORT, FROM_NAT)) {
            if (udp_1.valid == TRUE) {
                modify_field(udp_1.dstPort, port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (tcp.valid == TRUE) {
                modify_field(tcp.dstPort, port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
    } else {
        if (RX_REWRITE(rewrite_metadata.flags, SIP, FROM_NAT)) {
            if (ipv4_1.valid == TRUE) {
                modify_field(ipv4_1.srcAddr, ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (ipv6_1.valid == TRUE) {
                modify_field(ipv6_1.srcAddr, ip);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
        if (RX_REWRITE(rewrite_metadata.flags, SPORT, FROM_NAT)) {
            if (udp_1.valid == TRUE) {
                modify_field(udp_1.srcPort, port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
            if (tcp.valid == TRUE) {
                modify_field(tcp.srcPort, port);
                modify_field(control_metadata.update_checksum, TRUE);
            }
        }
    }
}

@pragma stage 1
@pragma hbm_table
@pragma index_table
table nat {
    reads {
        rewrite_metadata.xlate_id   : exact;
    }
    actions {
        nat_rewrite;
    }
    size : NAT_TABLE_SIZE;
}

@pragma stage 1
@pragma index_table
table nat2 {
    reads {
        rewrite_metadata.xlate_id2  : exact;
    }
    actions {
        nat2_rewrite;
    }
    size : NAT2_TABLE_SIZE;
}

control nat {
    if (control_metadata.apply_nat == TRUE) {
        apply(nat);
    }
    if (control_metadata.apply_nat2 == TRUE) {
        apply(nat2);
    }
}
