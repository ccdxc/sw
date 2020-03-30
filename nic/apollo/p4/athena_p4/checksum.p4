/*****************************************************************************/
/* Checksums                                                                 */
/*****************************************************************************/
action update_ipv4_checksum() {
    modify_field(capri_deparser_len.ipv4_1_hdr_len, ipv4_1.ihl << 2);
}

action update_ipv4_udp_checksum() {
    modify_field(capri_deparser_len.ipv4_1_hdr_len, ipv4_1.ihl << 2);
    modify_field(capri_deparser_len.l4_payload_len, udp_1.len);
}

action update_ipv4_tcp_checksum() {
    modify_field(capri_deparser_len.ipv4_1_hdr_len, ipv4_1.ihl << 2);
    modify_field(capri_deparser_len.l4_payload_len,
                 ipv4_1.totalLen - (ipv4_1.ihl << 2));
}

action update_ipv4_icmp_checksum() {
    modify_field(capri_deparser_len.ipv4_1_hdr_len, ipv4_1.ihl << 2);
    modify_field(capri_deparser_len.l4_payload_len,
                 ipv4_1.totalLen - (ipv4_1.ihl << 2));
}

action update_ipv6_udp_checksum() {
    modify_field(capri_deparser_len.l4_payload_len, udp_1.len);
}

action update_ipv6_tcp_checksum() {
    modify_field(capri_deparser_len.l4_payload_len, ipv6_1.payloadLen);
}

action update_ipv6_icmp_checksum() {
    modify_field(capri_deparser_len.l4_payload_len, ipv6_1.payloadLen);
}

action update_l2_ipv4_checksum() {
    modify_field(capri_deparser_len.ipv4_2_hdr_len, ipv4_2.ihl << 2);
}

action update_l2_ipv4_udp_checksum() {
    modify_field(capri_deparser_len.ipv4_2_hdr_len, ipv4_2.ihl << 2);
    modify_field(capri_deparser_len.l4_payload_len, udp_2.len);
}

action update_l2_ipv4_tcp_checksum() {
    modify_field(capri_deparser_len.ipv4_2_hdr_len, ipv4_2.ihl << 2);
    modify_field(capri_deparser_len.l4_payload_len,
                 ipv4_2.totalLen - (ipv4_2.ihl << 2));
}

action update_l2_ipv4_icmp_checksum() {
    modify_field(capri_deparser_len.ipv4_2_hdr_len, ipv4_2.ihl << 2);
    modify_field(capri_deparser_len.l4_payload_len,
                 ipv4_2.totalLen - (ipv4_2.ihl << 2));
}

action update_l2_ipv6_udp_checksum() {
    modify_field(capri_deparser_len.l4_payload_len, udp_2.len);
}

action update_l2_ipv6_tcp_checksum() {
    modify_field(capri_deparser_len.l4_payload_len, ipv6_2.payloadLen);
}

action update_l2_ipv6_icmp_checksum() {
    modify_field(capri_deparser_len.l4_payload_len, ipv6_2.payloadLen);
}

@pragma stage 5
table checksum {
    reads {
        ipv4_1.valid  : ternary;
        ipv6_1.valid  : ternary;
        udp_1.valid   : ternary;
        ipv4_2.valid  : ternary;
        ipv6_2.valid  : ternary;
        udp_2.valid   : ternary;
        tcp.valid     : ternary;
        icmp.valid    : ternary;
    }
    actions {
        update_ipv4_checksum;
        update_ipv4_udp_checksum;
        update_ipv4_tcp_checksum;
        update_ipv4_icmp_checksum;
        update_ipv6_udp_checksum;
        update_ipv6_tcp_checksum;
        update_ipv6_icmp_checksum;
        update_l2_ipv4_checksum;
        update_l2_ipv4_udp_checksum;
        update_l2_ipv4_tcp_checksum;
        update_l2_ipv4_icmp_checksum;
        update_l2_ipv6_udp_checksum;
        update_l2_ipv6_tcp_checksum;
        update_l2_ipv6_icmp_checksum;
    }
    size : CHECKSUM_TABLE_SIZE;
}

control update_checksums {
    if (control_metadata.update_checksum == TRUE) {
        apply(checksum);
    }
}
