/*****************************************************************************/
/* NAT Rewrite                                                               */
/*****************************************************************************/
header_type nat_metadata_t {
    fields {
        twice_nat_ip          : 128;
        twice_nat_l4_port     : 16;
        nat_ip                : 128;
        nat_l4_port           : 16;
        twice_nat_idx         : 13;

        update_checksum       : 1;
        update_inner_checksum : 1;
        tcp_checksum_len      : 16;
    }
}

metadata nat_metadata_t nat_metadata;

action ipv4_nat_update_l4_checksum() {
    subtract(nat_metadata.tcp_checksum_len, ipv4.totalLen, ipv4.ihl);
    modify_field(nat_metadata.update_checksum, TRUE);
}

action ipv4_nat_src_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.srcAddr, nat_metadata.nat_ip);
}

action ipv4_nat_dst_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.dstAddr, nat_metadata.nat_ip);
}

action ipv4_nat_src_udp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.srcAddr, nat_metadata.nat_ip);
    modify_field(udp.srcPort, nat_metadata.nat_l4_port);
}

action ipv4_nat_dst_udp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.dstAddr, nat_metadata.nat_ip);
    modify_field(udp.dstPort, nat_metadata.nat_l4_port);
}

action ipv4_nat_src_tcp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.srcAddr, nat_metadata.nat_ip);
    modify_field(tcp.srcPort, nat_metadata.nat_l4_port);
}

action ipv4_nat_dst_tcp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.dstAddr, nat_metadata.nat_ip);
    modify_field(tcp.dstPort, nat_metadata.nat_l4_port);
}

action ipv4_twice_nat_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.srcAddr, nat_metadata.nat_ip);
    modify_field(ipv4.dstAddr, nat_metadata.twice_nat_ip);
}

action ipv4_twice_nat_udp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.srcAddr, nat_metadata.nat_ip);
    modify_field(udp.srcPort, nat_metadata.nat_l4_port);
    modify_field(ipv4.dstAddr, nat_metadata.twice_nat_ip);
    modify_field(udp.dstPort, nat_metadata.twice_nat_l4_port);
}

action ipv4_twice_nat_tcp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv4.srcAddr, nat_metadata.nat_ip);
    modify_field(tcp.srcPort, nat_metadata.nat_l4_port);
    modify_field(ipv4.dstAddr, nat_metadata.nat_ip);
    modify_field(tcp.dstPort, nat_metadata.twice_nat_l4_port);
}

action ipv6_nat_src_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv6.srcAddr, nat_metadata.nat_ip);
}

action ipv6_nat_dst_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv6.dstAddr, nat_metadata.nat_ip);
}

action ipv6_nat_src_udp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv6.srcAddr, nat_metadata.nat_ip);
    modify_field(udp.srcPort, nat_metadata.nat_l4_port);
}

action ipv6_nat_dst_udp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv6.dstAddr, nat_metadata.nat_ip);
    modify_field(udp.dstPort, nat_metadata.nat_l4_port);
}

action ipv6_nat_src_tcp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(tcp.srcPort, nat_metadata.nat_l4_port);
}

action ipv6_nat_dst_tcp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv6.dstAddr, nat_metadata.nat_ip);
    modify_field(tcp.dstPort, nat_metadata.nat_l4_port);
}

action ipv6_twice_nat_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv6.srcAddr, nat_metadata.nat_ip);
    modify_field(ipv6.dstAddr, nat_metadata.twice_nat_ip);
}

action ipv6_twice_nat_udp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv6.srcAddr, nat_metadata.nat_ip);
    modify_field(udp.srcPort, nat_metadata.nat_l4_port);
    modify_field(ipv6.dstAddr, nat_metadata.twice_nat_ip);
    modify_field(udp.dstPort, nat_metadata.twice_nat_l4_port);
}

action ipv6_twice_nat_tcp_rewrite(mac_sa, mac_da) {
    rewrite(mac_sa, mac_da);
    modify_field(ipv6.srcAddr, nat_metadata.nat_ip);
    modify_field(tcp.srcPort, nat_metadata.nat_l4_port);
    modify_field(ipv6.dstAddr, nat_metadata.twice_nat_ip);
    modify_field(tcp.dstPort, nat_metadata.twice_nat_l4_port);
}

action twice_nat_rewrite_info(ip, l4_port) {
    modify_field(nat_metadata.twice_nat_ip, ip);
    modify_field(nat_metadata.twice_nat_l4_port, l4_port);
}

@pragma stage 0
table twice_nat {
    reads {
        nat_metadata.twice_nat_idx : exact;
    }
    actions {
        nop;
        twice_nat_rewrite_info;
    }
    size : TWICE_NAT_TABLE_SIZE;
}
