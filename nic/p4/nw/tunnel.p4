header_type tunnel_metadata_t {
    fields {
        tunnel_type             : 4;
        tunnel_vni              : 24;
        tunnel_originate        : 1;
        tunnel_originate_egress : 1;    // NCC work-around for predication
        tunnel_terminate        : 1;
        tunnel_terminate_egress : 1;    // NCC work-around for predication
        inner_ip_proto          : 8;
    }
}

header_type ipsec_metadata_t {
    fields {
        spi     : 32;
        seqNo   : 32;
        ipsec_type    : 2;
    }
}

metadata tunnel_metadata_t tunnel_metadata;
metadata ipsec_metadata_t ipsec_metadata;

/*****************************************************************************/
/* Tunnel decap - copy headers                                               */
/*****************************************************************************/
action copy_inner_ipv4_udp() {
    copy_header(ipv4, inner_ipv4);
    copy_header(udp, inner_udp);
    remove_header(inner_ipv4);
    remove_header(inner_udp);
}

action copy_inner_ipv4_other() {
    copy_header(ipv4, inner_ipv4);
    remove_header(inner_ipv4);
    remove_header(udp);
}

action copy_inner_ipv6_udp() {
    copy_header(ipv6, inner_ipv6);
    copy_header(udp, inner_udp);
    remove_header(inner_ipv6);
    remove_header(inner_udp);
}

action copy_inner_ipv6_other() {
    copy_header(ipv6, inner_ipv6);
    remove_header(inner_ipv6);
    remove_header(udp);
}

action copy_inner_eth_ipv4_udp() {
    copy_header(ethernet, inner_ethernet);
    copy_header(ipv4, inner_ipv4);
    copy_header(udp, inner_udp);
    remove_header(inner_ethernet);
    remove_header(inner_ipv4);
    remove_header(inner_udp);
    remove_header(vlan_tag);
}

action copy_inner_eth_ipv4_other() {
    copy_header(ethernet, inner_ethernet);
    copy_header(ipv4, inner_ipv4);
    remove_header(inner_ethernet);
    remove_header(inner_ipv4);
    remove_header(udp);
    remove_header(vlan_tag);
}

action copy_inner_eth_ipv6_udp() {
    copy_header(ethernet, inner_ethernet);
    copy_header(ipv6, inner_ipv6);
    copy_header(udp, inner_udp);
    remove_header(inner_ethernet);
    remove_header(inner_ipv6);
    remove_header(inner_udp);
    remove_header(vlan_tag);
}

action copy_inner_eth_ipv6_other() {
    copy_header(ethernet, inner_ethernet);
    copy_header(ipv6, inner_ipv6);
    remove_header(inner_ethernet);
    remove_header(inner_ipv6);
    remove_header(udp);
    remove_header(vlan_tag);
}

action copy_inner_eth_non_ip() {
    copy_header(ethernet, inner_ethernet);
    remove_header(inner_ethernet);
    remove_header(udp);
    remove_header(vlan_tag);
}

@pragma stage 0
table tunnel_decap_copy_inner {
    reads {
        inner_ethernet : valid;
        inner_ipv4     : valid;
        inner_ipv6     : valid;
        inner_udp      : valid;
    }
    actions {
        copy_inner_ipv4_udp;
        copy_inner_ipv4_other;
        copy_inner_ipv6_udp;
        copy_inner_ipv6_other;
        copy_inner_eth_ipv4_udp;
        copy_inner_eth_ipv4_other;
        copy_inner_eth_ipv6_udp;
        copy_inner_eth_ipv6_other;
        copy_inner_eth_non_ip;
        nop;
    }
    size : TUNNEL_DECAP_TABLE_SIZE;
}

/*****************************************************************************/
/* Tunnel decap - remove tunnel headers                                      */
/*****************************************************************************/
action remove_tunnel_hdrs() {
    remove_header(vxlan);
    remove_header(genv);
    remove_header(nvgre);
    remove_header(gre);
    remove_header(mpls[0]);
    remove_header(mpls[1]);
    remove_header(mpls[2]);
}

@pragma stage 0
table tunnel_decap {
    actions {
        remove_tunnel_hdrs;
    }
    size : TUNNEL_DECAP_TABLE_SIZE;
}

/*****************************************************************************/
/* VLAN encap                                                                */
/*****************************************************************************/
action f_encap_vlan(vlan_id, ethertype) {
    add_header(vlan_tag);
    modify_field(vlan_tag.etherType, ethertype);
    modify_field(vlan_tag.vid, vlan_id);
    modify_field(ethernet.etherType, ETHERTYPE_VLAN);
    if (qos_metadata.cos_en == TRUE) {
        modify_field(vlan_tag.pcp, qos_metadata.cos);
    }
}

action encap_vlan() {
    f_encap_vlan(rewrite_metadata.tunnel_vnid, ethernet.etherType);
}

/*****************************************************************************/
/* VXLAN tunnel encap actions                                                */
/*****************************************************************************/
action f_insert_vxlan_header(mac_sa, mac_da) {
    copy_header(inner_ethernet, ethernet);
    add_header(udp);
    add_header(vxlan);

    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    modify_field(udp.srcPort, rewrite_metadata.entropy_hash);
    modify_field(udp.dstPort, UDP_PORT_VXLAN);
    modify_field(udp.checksum, 0);
    add(udp.len, l3_metadata.payload_length, 30);

    modify_field(vxlan.flags, 0x8);
    modify_field(vxlan.reserved, 0);
    modify_field(vxlan.vni, rewrite_metadata.tunnel_vnid);
    modify_field(vxlan.reserved2, 0);
}

action encap_vxlan(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    f_insert_vxlan_header(mac_sa, mac_da);
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv4.totalLen, l3_metadata.payload_length, 50);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
    } else {
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv6.payloadLen, l3_metadata.payload_length, 30);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
    }
    if (vlan_valid == TRUE) {
        f_encap_vlan(vlan_id, scratch_metadata.ethtype);
    } else {
        modify_field(ethernet.etherType, scratch_metadata.ethtype);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
    modify_field(scratch_metadata.flag, ip_type);
}

/*****************************************************************************/
/* VXLAN GPE tunnel encap actions                                            */
/*****************************************************************************/
action f_insert_vxlan_gpe_header(mac_sa, mac_da) {
    copy_header(inner_ethernet, ethernet);
    add_header(udp);
    add_header(vxlan_gpe);

    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    modify_field(udp.srcPort, rewrite_metadata.entropy_hash);
    modify_field(udp.dstPort, UDP_PORT_VXLAN_GPE);
    modify_field(udp.checksum, 0);
    add(udp.len, l3_metadata.payload_length, 30);

    modify_field(vxlan_gpe.flags, 0x9);
    modify_field(vxlan_gpe.reserved, 0);
    modify_field(vxlan_gpe.next_proto, 0);
    modify_field(vxlan_gpe.vni, rewrite_metadata.tunnel_vnid);
    modify_field(vxlan_gpe.reserved2, 0);
}

action encap_vxlan_gpe(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    f_insert_vxlan_gpe_header(mac_sa, mac_da);
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv4.totalLen, l3_metadata.payload_length, 50);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
    } else {
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv6.payloadLen, l3_metadata.payload_length, 30);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
    }
    if (vlan_valid == TRUE) {
        f_encap_vlan(vlan_id, scratch_metadata.ethtype);
    } else {
        modify_field(ethernet.etherType, scratch_metadata.ethtype);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
    modify_field(scratch_metadata.flag, ip_type);
}

/*****************************************************************************/
/* GENEVE tunnel encap actions                                               */
/*****************************************************************************/
action f_insert_genv_header(mac_sa, mac_da) {
    copy_header(inner_ethernet, ethernet);
    add_header(udp);
    add_header(genv);

    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);

    modify_field(udp.srcPort, rewrite_metadata.entropy_hash);
    modify_field(udp.dstPort, UDP_PORT_GENV);
    modify_field(udp.checksum, 0);
    add(udp.len, l3_metadata.payload_length, 30);

    modify_field(genv.ver, 0);
    modify_field(genv.oam, 0);
    modify_field(genv.critical, 0);
    modify_field(genv.optLen, 0);
    modify_field(genv.protoType, ETHERTYPE_ETHERNET);
    modify_field(genv.vni, rewrite_metadata.tunnel_vnid);
    modify_field(genv.reserved, 0);
    modify_field(genv.reserved2, 0);
}

action encap_genv(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    f_insert_genv_header(mac_sa, mac_da);
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv4.totalLen, l3_metadata.payload_length, 50);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
    } else {
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv6.payloadLen, l3_metadata.payload_length, 30);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
    }

    if (vlan_valid == TRUE) {
        f_encap_vlan(vlan_id, scratch_metadata.ethtype);
    } else {
        modify_field(ethernet.etherType, scratch_metadata.ethtype);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
    modify_field(scratch_metadata.flag, ip_type);
}

/*****************************************************************************/
/* NVGRE tunnel encap actions                                                */
/*****************************************************************************/
action f_insert_nvgre_header(mac_sa, mac_da) {
    copy_header(inner_ethernet, ethernet);
    add_header(gre);
    add_header(nvgre);

    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);

    modify_field(gre.proto, ETHERTYPE_ETHERNET);
    modify_field(gre.recurse, 0);
    modify_field(gre.flags, 0);
    modify_field(gre.ver, 0);
    modify_field(gre.R, 0);
    modify_field(gre.K, 1);
    modify_field(gre.C, 0);
    modify_field(gre.S, 0);
    modify_field(gre.s, 0);

    modify_field(nvgre.tni, rewrite_metadata.tunnel_vnid);
    modify_field(nvgre.flow_id, rewrite_metadata.entropy_hash);
}

action encap_nvgre(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    f_insert_nvgre_header(mac_sa, mac_da);
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_GRE);
        add(ipv4.totalLen, l3_metadata.payload_length, 42);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
    } else {
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_GRE);
        add(ipv6.payloadLen, l3_metadata.payload_length, 22);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
    }
    if (vlan_valid == TRUE) {
        f_encap_vlan(vlan_id, scratch_metadata.ethtype);
    } else {
        modify_field(ethernet.etherType, scratch_metadata.ethtype);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
    modify_field(scratch_metadata.flag, ip_type);
}

/*****************************************************************************/
/* GRE tunnel encap actions                                                  */
/*****************************************************************************/
action f_insert_gre_header(mac_sa, mac_da) {
    add_header(gre);

    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
}

action encap_gre(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    f_insert_gre_header(mac_sa, mac_da);
    modify_field(gre.proto, ethernet.etherType);
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_GRE);
        add(ipv4.totalLen, l3_metadata.payload_length, 24);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
    } else {
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_GRE);
        add(ipv6.payloadLen, l3_metadata.payload_length, 4);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
    }
    if (vlan_valid == TRUE) {
        f_encap_vlan(vlan_id, scratch_metadata.ethtype);
    } else {
        modify_field(ethernet.etherType, scratch_metadata.ethtype);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
    modify_field(scratch_metadata.flag, ip_type);
}

/*****************************************************************************/
/* ERSPAN tunnel encap actions                                               */
/*****************************************************************************/
action f_insert_erspan_t3_header(mac_sa, mac_da) {
    copy_header(inner_ethernet, ethernet);
    add_header(gre);
    add_header(erspan_t3_header);
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    modify_field(gre.C, 0);
    modify_field(gre.R, 0);
    modify_field(gre.K, 0);
    modify_field(gre.S, 0);
    modify_field(gre.s, 0);
    modify_field(gre.recurse, 0);
    modify_field(gre.flags, 0);
    modify_field(gre.ver, 0);
    modify_field(gre.proto, GRE_PROTO_ERSPAN_T3);
    modify_field(erspan_t3_header.timestamp, capri_intrinsic.timestamp);
    modify_field(erspan_t3_header.span_id, capri_intrinsic.tm_span_session);
    modify_field(erspan_t3_header.version, 2);
    modify_field(erspan_t3_header.sgt, 0);
}

action encap_erspan(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    f_insert_erspan_t3_header(mac_sa, mac_da);
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_GRE);
        add(ipv4.totalLen, l3_metadata.payload_length, 50);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
    } else {
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_GRE);
        add(ipv6.payloadLen, l3_metadata.payload_length, 30);
        modify_field(ethernet.etherType, ETHERTYPE_IPV4);
    }
    if (vlan_valid == TRUE) {
        f_encap_vlan(vlan_id, scratch_metadata.ethtype);
    } else {
        modify_field(ethernet.etherType, scratch_metadata.ethtype);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
    modify_field(scratch_metadata.flag, ip_type);
}

/*****************************************************************************/
/* IPinIP tunnel encap actions                                               */
/*****************************************************************************/
action encap_ip(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, tunnel_metadata.inner_ip_proto);
        add(ipv4.totalLen, l3_metadata.payload_length, 20);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
    } else {
        f_insert_ipv6_header(ip_sa, ip_da, tunnel_metadata.inner_ip_proto);
        modify_field(ipv6.payloadLen, l3_metadata.payload_length);
        modify_field(ethernet.etherType, ETHERTYPE_IPV4);
    }
    if (vlan_valid == TRUE) {
        f_encap_vlan(vlan_id, scratch_metadata.ethtype);
    } else {
        modify_field(ethernet.etherType, scratch_metadata.ethtype);
    }

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
    modify_field(scratch_metadata.flag, ip_type);
    modify_field(scratch_metadata.mac, mac_sa);
    modify_field(scratch_metadata.mac, mac_da);
    modify_field(scratch_metadata.lkp_proto, tunnel_metadata.inner_ip_proto);
}

/*****************************************************************************/
/* IP header encap                                                           */
/*****************************************************************************/
action f_insert_ipv4_header(ip_sa, ip_da, proto) {
    add_header(ipv4);
    modify_field(ipv4.srcAddr, ip_sa);
    modify_field(ipv4.dstAddr, ip_da);
    modify_field(ipv4.protocol, proto);
    modify_field(ipv4.ttl, 64);
    modify_field(ipv4.version, 0x4);
    modify_field(ipv4.ihl, 0x5);
    modify_field(ipv4.identification, 0);
}

action f_insert_ipv6_header(ip_sa, ip_da, proto) {
    add_header(ipv6);
    modify_field(ipv6.srcAddr, ip_sa);
    modify_field(ipv6.dstAddr, ip_da);
    modify_field(ipv6.version, 0x6);
    modify_field(ipv6.nextHdr, proto);
    modify_field(ipv6.hopLimit, 64);
    modify_field(ipv6.trafficClass, 0);
    modify_field(ipv6.flowLabel, 0);
}

action encap_mpls(mac_sa, mac_da, eompls, num_labels, label0, exp0, bos0, ttl0,
                  label1, exp1, bos1, ttl1, label2, exp2, bos2, ttl2) {
    if (eompls == TRUE) {
        copy_header(inner_ethernet, ethernet);
    }
    if (num_labels == 1) {
        push(mpls, 1);
        modify_field(mpls[0].label, label0);
        modify_field(mpls[0].exp, exp0);
        modify_field(mpls[0].bos, bos0);
        modify_field(mpls[0].ttl, ttl0);
    }
    if (num_labels == 2) {
        modify_field(mpls[0].label, label0);
        modify_field(mpls[0].exp, exp0);
        modify_field(mpls[0].bos, bos0);
        modify_field(mpls[0].ttl, ttl0);
        modify_field(mpls[1].label, label1);
        modify_field(mpls[1].exp, exp1);
        modify_field(mpls[1].bos, bos1);
        modify_field(mpls[1].ttl, ttl1);
    }
    if (num_labels == 3) {
        push(mpls, 3);
        modify_field(mpls[0].label, label0);
        modify_field(mpls[0].exp, exp0);
        modify_field(mpls[0].bos, bos0);
        modify_field(mpls[0].ttl, ttl0);
        modify_field(mpls[1].label, label1);
        modify_field(mpls[1].exp, exp1);
        modify_field(mpls[1].bos, bos1);
        modify_field(mpls[1].ttl, ttl1);
        modify_field(mpls[2].label, label2);
        modify_field(mpls[2].exp, exp2);
        modify_field(mpls[2].bos, bos2);
        modify_field(mpls[2].ttl, ttl2);
    }
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    modify_field(ethernet.etherType, ETHERTYPE_MPLS);

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.flag, eompls);
    modify_field(scratch_metadata.num_labels, num_labels);
}

action encap_ipv4_ipsec_tunnel_esp(mac_sa, mac_da, ip_sa, ip_da)
{
    f_insert_ipv4_header(ip_sa,ip_da,IP_PROTO_IPSEC_ESP);
    add(ipv4.totalLen, l3_metadata.payload_length,20); // 20 IP. Need to still add padding and esp trailer.
    // 42 = Eth(14)+ip(20)
    modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 34);
    // add outer-mac header length
    add_header(p4_to_p4plus_ipsec);
    modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv4.totalLen+14);

    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
   //update-splitter-offset here

}

action encap_vlan_ipv4_ipsec_tunnel_esp(mac_sa, mac_da, ip_sa, ip_da, vlan_id)
{
    f_insert_ipv4_header(ip_sa,ip_da,IP_PROTO_IPSEC_ESP);
    add(ipv4.totalLen, l3_metadata.payload_length,20); // 20 IP. Need to still add padding and esp trailer.
    f_encap_vlan(vlan_id, ETHERTYPE_IPV4);
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);

    // 46 = Eth(14)+vlan(4)+ip(20)
    add_header(p4_to_p4plus_ipsec);
    modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 38);
    modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv4.totalLen+18);
   //update-splitter-offset here
}

action encap_ipv6_ipsec_tunnel_esp(mac_sa, mac_da, ip_sa, ip_da)
{
    f_insert_ipv6_header(ip_sa,ip_da,IP_PROTO_IPSEC_ESP);
    add(ipv6.payloadLen, l3_metadata.payload_length, 40);
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    add_header(p4_to_p4plus_ipsec);
    // 42 = Eth(14)+ipv6(40)
    modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 54);
    // add outer-mac header length
    modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv6.payloadLen+14);
   //update-splitter-offset here

}

action encap_vlan_ipv6_ipsec_tunnel_esp(mac_sa, mac_da, ip_sa, ip_da, vlan_id)
{
    f_insert_ipv6_header(ip_sa,ip_da,IP_PROTO_IPSEC_ESP);
    add(ipv6.payloadLen, l3_metadata.payload_length, 40);
    f_encap_vlan(vlan_id, ETHERTYPE_IPV6);
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    add_header(p4_to_p4plus_ipsec);
    // 66 = Eth(14)+vlan(4)+ipv6(40)
    modify_field(p4_to_p4plus_ipsec.ipsec_payload_start, 58);
    modify_field(p4_to_p4plus_ipsec.ipsec_payload_end, ipv6.payloadLen+18);
   //update-splitter-offset here
}

@pragma stage 4
table tunnel_rewrite {
    reads {
        rewrite_metadata.tunnel_rewrite_index : exact;
    }
    actions {
        nop;
        encap_vxlan;
        encap_genv;
        encap_nvgre;
        encap_gre;
        encap_ip;
        encap_erspan;
        encap_mpls;
        encap_vlan;
        encap_vxlan_gpe;
        encap_ipv4_ipsec_tunnel_esp;
        encap_vlan_ipv4_ipsec_tunnel_esp;
        encap_ipv6_ipsec_tunnel_esp;
        encap_vlan_ipv6_ipsec_tunnel_esp;
    }
    size : TUNNEL_REWRITE_TABLE_SIZE;
}

/*****************************************************************************/
/* Update L4 headers on tunnel encap                                         */
/*****************************************************************************/
action encap_inner_ipv4_udp_rewrite() {
    copy_header(inner_ipv4, ipv4);
    copy_header(inner_udp, udp);
    modify_field(l3_metadata.payload_length, ipv4.totalLen);
    remove_header(udp);
    remove_header(ipv4);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV4);

    /* move checksum update flag */
    modify_field(nat_metadata.update_inner_checksum,
                 nat_metadata.update_checksum);
    modify_field(nat_metadata.update_checksum, FALSE);
}

action encap_inner_ipv4_tcp_rewrite() {
    copy_header(inner_ipv4, ipv4);
    modify_field(l3_metadata.payload_length, ipv4.totalLen);
    remove_header(ipv4);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV4);

    /* move checksum update flag */
    modify_field(nat_metadata.update_inner_checksum,
                 nat_metadata.update_checksum);
    modify_field(nat_metadata.update_checksum, FALSE);
}

action encap_inner_ipv4_icmp_rewrite() {
    copy_header(inner_ipv4, ipv4);
    modify_field(l3_metadata.payload_length, ipv4.totalLen);
    remove_header(ipv4);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV4);
}

action encap_inner_ipv4_unknown_rewrite() {
    copy_header(inner_ipv4, ipv4);
    modify_field(l3_metadata.payload_length, ipv4.totalLen);
    remove_header(ipv4);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV4);
}

action encap_inner_ipv6_udp_rewrite() {
    copy_header(inner_ipv6, ipv6);
    copy_header(inner_udp, udp);
    add(l3_metadata.payload_length, ipv6.payloadLen, 40);
    remove_header(udp);
    remove_header(ipv6);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV6);
}

action encap_inner_ipv6_tcp_rewrite() {
    copy_header(inner_ipv6, ipv6);
    add(l3_metadata.payload_length, ipv6.payloadLen, 40);
    remove_header(ipv6);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV6);
}

action encap_inner_ipv6_icmp_rewrite() {
    copy_header(inner_ipv6, ipv6);
    add(l3_metadata.payload_length, ipv6.payloadLen, 40);
    remove_header(ipv6);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV6);
}

action encap_inner_ipv6_unknown_rewrite() {
    copy_header(inner_ipv6, ipv6);
    add(l3_metadata.payload_length, ipv6.payloadLen, 40);
    remove_header(ipv6);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV6);
}

@pragma stage 3
table tunnel_encap_update_inner {
    reads {
        ipv4 : valid;
        ipv6 : valid;
        tcp  : valid;
        udp  : valid;
        icmp : valid;
    }
    actions {
        encap_inner_ipv4_udp_rewrite;
        encap_inner_ipv4_tcp_rewrite;
        encap_inner_ipv4_icmp_rewrite;
        encap_inner_ipv4_unknown_rewrite;
        encap_inner_ipv6_udp_rewrite;
        encap_inner_ipv6_tcp_rewrite;
        encap_inner_ipv6_icmp_rewrite;
        encap_inner_ipv6_unknown_rewrite;
    }
    size : TUNNEL_ENCAP_UPDATE_INNER_TABLE_SIZE;
}
