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

// Same IP check for src and dst only when IP Normalizaiton is enabled.
action validate_ipv4_flow_key() {
    if (((flow_lkp_metadata.lkp_src & 0xff000000) == 0x7f000000) or
        ((flow_lkp_metadata.lkp_src & 0xf0000000) == 0xe0000000) or
        (flow_lkp_metadata.lkp_src == 0xffffffff) or
        (flow_lkp_metadata.lkp_dst == 0) or
        ((flow_lkp_metadata.lkp_dst & 0xff000000) == 0x7f000000) or
        ((l4_metadata.ip_normalization_en == TRUE) and
         (flow_lkp_metadata.lkp_src == flow_lkp_metadata.lkp_dst))) {
        malformed_packet();
    }
}

// Same IP check for src and dst only when IP Normalizaiton is enabled.
action validate_ipv6_flow_key() {
    if ((flow_lkp_metadata.lkp_src == 0x00000000000000000000000000000001) or
        ((flow_lkp_metadata.lkp_src & 0xff000000000000000000000000000000) ==
         0xff000000000000000000000000000000) or
        (flow_lkp_metadata.lkp_dst == 0x00000000000000000000000000000000) or
        (flow_lkp_metadata.lkp_dst == 0x00000000000000000000000000000001) or
        ((l4_metadata.ip_normalization_en == TRUE) and
         (flow_lkp_metadata.lkp_src == flow_lkp_metadata.lkp_dst))) {
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
    if ((capri_intrinsic.tm_iport != TM_PORT_NCSI and ethernet.srcAddr == 0) or
        (ethernet.dstAddr == 0) or
        (ethernet.srcAddr == ethernet.dstAddr) or
        ((ethernet.srcAddr & 0x010000000000) == 0x010000000000) or
        (ethernet.srcAddr == 0xFFFFFFFFFFFF)) {
        malformed_packet();
    }
}

action validate_tunneled_packet() {
    // For tunneled packet we will also validate the outer-L2 Header
    // Not validating outer-L3 header because if we decided to terminate
    // then the outer L3 header is mostly good (Like DIP should be valid
    // as we do a lookup of it in input_mapping_tunnel table, where as
    // outer SIP might still be invalid).
    if ((ethernet.srcAddr == 0) or
        (ethernet.dstAddr == 0) or
        (ethernet.srcAddr == ethernet.dstAddr) or
        ((ethernet.srcAddr & 0x010000000000) == 0x010000000000) or
        (ethernet.srcAddr == 0xFFFFFFFFFFFF)) {
        malformed_packet();
    }
    if ((inner_ethernet.valid == TRUE) and
        ((inner_ethernet.srcAddr == 0) or
         (inner_ethernet.dstAddr == 0) or
         (inner_ethernet.srcAddr == inner_ethernet.dstAddr) or
         ((inner_ethernet.srcAddr & 0x010000000000) == 0x010000000000) or
         (inner_ethernet.srcAddr == 0xFFFFFFFFFFFF))) {
        malformed_packet();
    }
}

action check_parser_errors() {
    if (capri_p4_intrinsic.parser_err == TRUE) {
        if (control_metadata.uplink == TRUE) {
            if (capri_p4_intrinsic.crc_err != 0) {
                modify_field(control_metadata.drop_reason,
                             (1 << DROP_PARSER_ICRC_ERR),
                             (1 << DROP_PARSER_ICRC_ERR));
                drop_packet();
            }
            if (capri_p4_intrinsic.len_err != 0) {
                modify_field(control_metadata.drop_reason,
                             (1 << DROP_PARSER_LEN_ERR),
                             (1 << DROP_PARSER_LEN_ERR));
                drop_packet();
            }
            modify_field(control_metadata.checksum_results,
                         capri_intrinsic.csum_err);
        } else {
            if ((capri_p4_intrinsic.len_err != 0) and
                ((p4plus_to_p4.update_ip_len == 0) and
                 (p4plus_to_p4.update_udp_len == 0))) {
                modify_field(control_metadata.drop_reason,
                             (1 << DROP_PARSER_LEN_ERR),
                             (1 << DROP_PARSER_LEN_ERR));
                drop_packet();
            }
        }
    }
}

action validate_packet() {
    if (tunnel_metadata.tunnel_terminate == TRUE) {
        validate_tunneled_packet();
    } else {
        validate_native_packet();
    }
    check_parser_errors();
}

@pragma stage 1
table validate_packet {
    actions {
        validate_packet;
    }
    default_action : validate_packet;
}

control process_validation {
    apply(validate_packet);
}
