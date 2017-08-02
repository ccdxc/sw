/*****************************************************************************/
/* Key validation                                                            */
/*****************************************************************************/

/******************************************************************************
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  type | src   | dst   | ttl | ver | flags | frag   | flags | action
       |       |       |     |     |       | offset |       |
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  MAC  | 0     | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  MAC  | mcast | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  MAC  | bcast | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  MAC  | x     | 0     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  MAC  | x     | x     |  x  | x   |   x   | x      |   x   | check
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv4 | 127/8 | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv4 | 224/4 | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv4 | bcast | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv4 | x     | 0     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv4 | x     | 127/8 |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv4 | x     | x     |  0  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv4 | x     | x     |  x  | 4   |   x   | x      |   x   | check
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv4 | x     | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv6 | ::1   | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv6 | mcast | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv6 |       | ::    |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv6 |       | ::1   |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv6 | x     | x     |  0  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv6 | x     | x     |  x  | 6   |   x   | x      |   x   | check
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
  IPv6 | x     | x     |  x  | x   |   x   | x      |   x   | malformed
  -----+-------+-------+-----+-----+-------+--------+-------+------------------
******************************************************************************/

action malformed_packet() {
    modify_field(control_metadata.drop_reason, DROP_MALFORMED_PKT);
    drop_packet();
}

action validate_native_packet() {
    if ((ethernet.srcAddr == 0) or
        (ethernet.dstAddr == 0) or
        ((ethernet.srcAddr & 0x010000000000) == 0x010000000000) or
        (ethernet.srcAddr == 0xFFFFFFFFFFFF)) {
        malformed_packet();
    }

    if ((ipv4.valid == TRUE) and
        ((ipv4.version != 4) or
         (ipv4.ttl == 0) or
         ((ipv4.srcAddr & 0xff000000) == 0x7f000000) or
         ((ipv4.srcAddr & 0xf0000000) == 0xe0000000) or
         (ipv4.srcAddr == 0xffffffff) or
         (ipv4.dstAddr == 0) or
         ((ipv4.dstAddr & 0xf0000000) == 0x7f000000) or
         (ipv4.srcAddr == ipv4.dstAddr))) {
        malformed_packet();
    }

    if ((ipv6.valid == TRUE) and
        ((ipv6.version != 6) or
         (ipv6.hopLimit == 0) or
         (ipv6.srcAddr == 0x00000000000000000000000000000001) or
         ((ipv6.srcAddr & 0xff000000000000000000000000000000) == 0xff000000000000000000000000000000) or
         (ipv6.dstAddr == 0x00000000000000000000000000000000) or
         (ipv6.dstAddr == 0x00000000000000000000000000000001) or
         (ipv6.srcAddr == ipv6.dstAddr))) {
        malformed_packet();
    }
}

action validate_tunneled_packet() {
    if ((inner_ethernet.srcAddr == 0) or
        (inner_ethernet.dstAddr == 0) or
        ((inner_ethernet.srcAddr & 0x010000000000) == 0x010000000000) or
        (inner_ethernet.srcAddr == 0xFFFFFFFFFFFF)) {
        malformed_packet();
    }

    if ((inner_ipv4.valid == TRUE) and
        ((inner_ipv4.version != 4) or
         (inner_ipv4.ttl == 0))) {
        malformed_packet();
    }

    if ((inner_ipv6.valid == TRUE) and
        ((inner_ipv6.version != 6) or
         (inner_ipv6.hopLimit == 0))) {
        malformed_packet();
    }
}

action validate_tunneled_packet2() {
    if (((tunnel_metadata.tunnel_terminate == TRUE) and
         (inner_ipv4.valid == TRUE)) and
        (((inner_ipv4.srcAddr & 0xff000000) == 0x7f000000) or
         ((inner_ipv4.srcAddr & 0xf0000000) == 0xe0000000) or
         (inner_ipv4.srcAddr == 0xffffffff) or
         (inner_ipv4.dstAddr == 0) or
         ((inner_ipv4.dstAddr & 0xf0000000) == 0x7f000000) or
         (inner_ipv4.srcAddr == inner_ipv4.dstAddr))) {
        malformed_packet();
    }

    if (((tunnel_metadata.tunnel_terminate == TRUE) and
         (inner_ipv6.valid == TRUE)) and
        ((inner_ipv6.srcAddr == 0x00000000000000000000000000000001) or
         ((inner_ipv6.srcAddr & 0xff000000000000000000000000000000) == 0xff000000000000000000000000000000) or
         (inner_ipv6.dstAddr == 0x00000000000000000000000000000000) or
         (inner_ipv6.dstAddr == 0x00000000000000000000000000000001) or
         (inner_ipv6.srcAddr == inner_ipv6.dstAddr))) {
        malformed_packet();
    }
}
