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

action validate_ipv4_flow_key() {
    if (((flow_lkp_metadata.lkp_src & 0xff000000) == 0x7f000000) or
        ((flow_lkp_metadata.lkp_src & 0xf0000000) == 0xe0000000) or
        (flow_lkp_metadata.lkp_src == 0xffffffff) or
        (flow_lkp_metadata.lkp_dst == 0) or
        ((flow_lkp_metadata.lkp_dst & 0xff000000) == 0x7f000000) or
        (flow_lkp_metadata.lkp_src == flow_lkp_metadata.lkp_dst)) {
        malformed_packet();
    }
}

action validate_ipv6_flow_key() {
    if ((flow_lkp_metadata.lkp_src == 0x00000000000000000000000000000001) or
        ((flow_lkp_metadata.lkp_src & 0xff000000000000000000000000000000) ==
         0xff000000000000000000000000000000) or
        (flow_lkp_metadata.lkp_dst == 0x00000000000000000000000000000000) or
        (flow_lkp_metadata.lkp_dst == 0x00000000000000000000000000000001) or
        (flow_lkp_metadata.lkp_src == flow_lkp_metadata.lkp_dst)) {
        malformed_packet();
    }
}

action validate_flow_key() {
    if (flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4) {
        validate_ipv4_flow_key();
    } else {
        if (flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV6) {
            validate_ipv6_flow_key();
        }
    }
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
         (ipv4.ttl == 0))) {
        malformed_packet();
    }

    if ((ipv6.valid == TRUE) and
        ((ipv6.version != 6) or
         (ipv6.hopLimit == 0))) {
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

action validate_packet() {
    if (tunnel_metadata.tunnel_terminate == TRUE) {
        validate_tunneled_packet();
    } else {
        validate_native_packet();
    }
}

@pragma stage 3
table validate_packet {
    actions {
        validate_packet;
    }
    default_action : validate_packet;
}
