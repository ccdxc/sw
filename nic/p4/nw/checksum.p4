/*****************************************************************************/
/* Checksum verification and computation                                     */
/*****************************************************************************/
action compute_checksum1() {
    modify_field(scratch_metadata.packet_len, l4_metadata.tcp_data_len);
    modify_field(scratch_metadata.packet_len, udp.len);
    modify_field(scratch_metadata.packet_len, inner_udp.len);
    modify_field(scratch_metadata.size8, control_metadata.checksum_ctl);
    modify_field(scratch_metadata.packet_len, ipv4.totalLen);
    modify_field(scratch_metadata.packet_len, inner_ipv4.totalLen);
    modify_field(scratch_metadata.packet_len, ipv6.payloadLen);
    modify_field(scratch_metadata.packet_len, inner_ipv6.payloadLen);
}

action compute_checksum2()  {}
action compute_checksum3()  {}
action compute_checksum4()  {}
action compute_checksum5()  {}
action compute_checksum6()  {}
action compute_checksum7()  {}
action compute_checksum8()  {}
action compute_checksum9()  {}
action compute_checksum10() {}
action compute_checksum11() {}
action compute_checksum12() {}
action compute_checksum13() {}
action compute_checksum14() {}
action compute_checksum15() {}
action compute_checksum16() {}
action compute_checksum17() {}
action compute_checksum18() {}
action compute_checksum19() {}
action compute_checksum20() {}
action compute_checksum21() {}
action compute_checksum22() {}
action compute_checksum23() {}
action compute_checksum24() {}
action compute_checksum25() {}

@pragma stage 5
table compute_checksum {
    reads {
        entry_inactive.compute_checksum : ternary;
        control_metadata.p4plus_app_id  : ternary;
        ipv4.valid                      : ternary;
        ipv6.valid                      : ternary;
        inner_ipv4.valid                : ternary;
        inner_ipv6.valid                : ternary;
        tcp.valid                       : ternary;
        udp.valid                       : ternary;
        inner_udp.valid                 : ternary;
    }
    actions {
        compute_checksum1;
        compute_checksum2;
        compute_checksum3;
        compute_checksum4;
        compute_checksum5;
        compute_checksum6;
        compute_checksum7;
        compute_checksum8;
        compute_checksum9;
        compute_checksum10;
        compute_checksum11;
        compute_checksum12;
        compute_checksum13;
        compute_checksum14;
        compute_checksum15;
        compute_checksum16;
        compute_checksum17;
        compute_checksum18;
        compute_checksum19;
        compute_checksum20;
        compute_checksum21;
        compute_checksum22;
        compute_checksum23;
        compute_checksum24;
        compute_checksum25;
        nop;
    }
    size : CHECKSUM_COMPUTE_TABLE_SIZE;
}

control process_checksum_computation {
    apply(compute_checksum);
}

/******************************************************************************
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
ipv4 | ipv6 | inner ipv4 | inner ipv6 | tcp | udp | inner udp |  Action number and bits to set
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     0      |     0      |  1  |  0  |     0     |  1.  ipv4_csum, ipv4_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     0      |     X      |  0  |  1  |     0     |  2.  ipv4_csum, ipv4_udp_csum, udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     0      |     0      |  0  |  0  |     0     |  3.  ipv4_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     1      |     0      |  1  |  0  |     0     |  4.  ipv4_csum, inner_ipv4_csum, inner_ipv4_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     1      |     0      |  0  |  0  |     1     |  5.  ipv4_csum, inner_ipv4_csum, inner_ipv4_udp_csum, inner_udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     1      |     0      |  0  |  0  |     0     |  6.  ipv4_csum, inner_ipv4_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     1      |     0      |  1  |  1  |     0     |  7.  ipv4_csum, inner_ipv4_csum, ipv4_udp_csum, udp_csum, inner_ipv4_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     1      |     0      |  0  |  1  |     1     |  8.  ipv4_csum, inner_ipv4_csum, ipv4_udp_csum, udp_csm, inner_ipv4_udp_csum, inner_udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     1      |     0      |  0  |  1  |     0     |  9.  ipv4_csum, inner_ipv4_csum, ipv4_udp_csum, udp_csm
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     0      |     1      |  1  |  0  |     0     |  10. ipv4_csum, inner_ipv6_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     0      |     1      |  0  |  0  |     1     |  11. ipv4_csum, inner_ipv6_udp_csum, inner_udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     0      |     1      |  1  |  1  |     0     |  12. ipv4_csum, ipv4_udp_csum, udp_csum, inner_ipv6_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  1  |  0   |     0      |     1      |  0  |  1  |     1     |  13. ipv4_csum, ipv4_udp_csum, udp_csum, inner_ipv6_udp_csum, inner_udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     0      |     0      |  1  |  0  |     0     |  14. ipv6_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     0      |     X      |  0  |  1  |     0     |  15. ipv6_udp_csum, udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     1      |     0      |  1  |  0  |     0     |  16. inner_ipv4_csum, inner_ipv4_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     1      |     0      |  0  |  0  |     1     |  17. inner_ipv4_csum, inner_ipv4_udp_csum, inner_udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     1      |     0      |  0  |  0  |     0     |  18. inner_ipv4_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     1      |     0      |  1  |  1  |     0     |  19. ipv6_udp_csum, udp_csum, inner_ipv4_csum, inner_ipv4_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     1      |     0      |  0  |  1  |     1     |  20. ipv6_udp_csum, udp_csum, inner_ipv4_csum, inner_ipv4_udp_csum, inner_udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     1      |     0      |  0  |  1  |     0     |  21. ipv6_udp_csum, udp_csum, inner_ipv4_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     0      |     1      |  1  |  1  |     0     |  22. ipv6_udp_csum, udp_csum, inner_ipv6_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     0      |     1      |  0  |  1  |     1     |  23. ipv6_udp_csum, udp_csum, inner_ipv6_udp_csum, inner_udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     0      |     1      |  1  |  0  |     0     |  24. inner_ipv6_tcp_csum, tcp_csum
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
  0  |  1   |     0      |     1      |  0  |  0  |     1     |  25. inner_ipv6_udp_csum, inner_udp_csum, icrc
-----+------+------------+------------+-----+-----+-----------+-------------------------------------------
*****************************************************************************/
