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
        spi_hi     : 16;
        spi_lo     : 16;
        seq_no     : 32;
        ipsec_type : 2;
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
    if (inner_ipv4_options_blob.valid == TRUE) {
        // copy_header(ipv4_options_blob2, inner_ipv4_options_blob);
    }
    remove_header(inner_ipv4);
    remove_header(inner_ipv4_options_blob);
    remove_header(inner_udp);
}

action copy_inner_ipv4_other() {
    copy_header(ipv4, inner_ipv4);
    if (inner_ipv4_options_blob.valid == TRUE) {
        // copy_header(ipv4_options_blob2, inner_ipv4_options_blob);
    }
    remove_header(inner_ipv4);
    remove_header(inner_ipv4_options_blob);
    remove_header(udp);
}

action copy_inner_ipv6_udp() {
    copy_header(ipv6, inner_ipv6);
    copy_header(udp, inner_udp);
    remove_header(ipv4);
    remove_header(inner_ipv6);
    remove_header(inner_udp);
}

action copy_inner_ipv6_other() {
    copy_header(ipv6, inner_ipv6);
    remove_header(ipv4);
    remove_header(inner_ipv6);
    remove_header(udp);
}

action copy_inner_eth_ipv4_udp() {
    copy_header(ethernet, inner_ethernet);
    copy_header(ipv4, inner_ipv4);
    copy_header(udp, inner_udp);
    if (inner_ipv4_options_blob.valid == TRUE) {
        // copy_header(ipv4_options_blob2, inner_ipv4_options_blob);
    }
    remove_header(inner_ethernet);
    remove_header(inner_ipv4);
    remove_header(inner_ipv4_options_blob);
    remove_header(inner_udp);
    remove_header(vlan_tag);
}

action copy_inner_eth_ipv4_other() {
    copy_header(ethernet, inner_ethernet);
    copy_header(ipv4, inner_ipv4);
    if (inner_ipv4_options_blob.valid == TRUE) {
        // copy_header(ipv4_options_blob2, inner_ipv4_options_blob);
    }
    remove_header(inner_ethernet);
    remove_header(inner_ipv4);
    remove_header(inner_ipv4_options_blob);
    remove_header(udp);
    remove_header(vlan_tag);
}

action copy_inner_eth_ipv6_udp() {
    copy_header(ethernet, inner_ethernet);
    copy_header(ipv6, inner_ipv6);
    copy_header(udp, inner_udp);
    remove_header(ipv4);
    remove_header(inner_ethernet);
    remove_header(inner_ipv6);
    remove_header(inner_udp);
    remove_header(vlan_tag);
}

action copy_inner_eth_ipv6_other() {
    copy_header(ethernet, inner_ethernet);
    copy_header(ipv6, inner_ipv6);
    remove_header(ipv4);
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
    remove_header(ipv4);
    remove_header(ipv6);
}

@pragma stage 1
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
    if (ipv4.valid == TRUE) {
        subtract(capri_p4_intrinsic.packet_len, ipv4.totalLen, 20);
    } else {
        modify_field(capri_p4_intrinsic.packet_len, ipv6.payloadLen);
    }
    if ((vxlan.valid == TRUE) or (genv.valid == TRUE)) {
        add_to_field(capri_p4_intrinsic.packet_len, -16);
    }
    if (gre.valid == TRUE) {
        add_to_field(capri_p4_intrinsic.packet_len, -4);
    }
    if (nvgre.valid == TRUE) {
        add_to_field(capri_p4_intrinsic.packet_len, -4);
    }
    if (mpls[0].valid == TRUE) {
        add_to_field(capri_p4_intrinsic.packet_len, -4);
    }
    remove_header(vxlan);
    remove_header(genv);
    remove_header(nvgre);
    remove_header(gre);
    remove_header(mpls[0]);
#ifdef PHASE2
    remove_header(mpls[1]);
    remove_header(mpls[2]);
#endif
}

@pragma stage 1
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
    if (vlan_tag.valid == FALSE) {
        add_header(vlan_tag);
        modify_field(vlan_tag.etherType, ethertype);
        modify_field(ethernet.etherType, ETHERTYPE_VLAN);
        add_to_field(capri_p4_intrinsic.packet_len, 4);
    }
    modify_field(vlan_tag.vid, vlan_id);
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
    modify_field(udp.srcPort, (0xC000 | rewrite_metadata.entropy_hash));
    modify_field(udp.dstPort, UDP_PORT_VXLAN);
    modify_field(udp.checksum, 0);
    add(udp.len, capri_p4_intrinsic.packet_len, 16);

    modify_field(vxlan.flags, 0x8);
    modify_field(vxlan.reserved, 0);
    modify_field(vxlan.vni, rewrite_metadata.tunnel_vnid);
    modify_field(vxlan.reserved2, 0);
}

action encap_vxlan(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    f_insert_vxlan_header(mac_sa, mac_da);
    if (ip_type == IP_HEADER_TYPE_IPV4) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len, 36);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
        add_to_field(capri_p4_intrinsic.packet_len, 50);
    } else {
#ifdef PHASE2
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv6.payloadLen, capri_p4_intrinsic.packet_len, 16);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
        add_to_field(capri_p4_intrinsic.packet_len, 70);
#endif /* PHASE2 */
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
/* MPLSoUDP tunnel encap actions                                             */
/*****************************************************************************/
action encap_mpls_udp(mac_sa, mac_da, ip_sa, ip_da, ip_type,
                      vlan_valid, vlan_id) {
    add_header(udp);
    add_header(mpls[0]);
    modify_field(mpls[0].label, rewrite_metadata.tunnel_vnid);
    modify_field(mpls[0].exp, 0);
    modify_field(mpls[0].bos, 1);
    modify_field(mpls[0].ttl, 64);

    if (mac_sa != 0) {
        modify_field(ethernet.srcAddr, mac_sa);
    }
    modify_field(ethernet.dstAddr, mac_da);
    modify_field(udp.srcPort, (0xC000 | rewrite_metadata.entropy_hash));
    modify_field(udp.dstPort, UDP_PORT_MPLS);
    modify_field(udp.checksum, 0);
    add(udp.len, capri_p4_intrinsic.packet_len, -2);

    if (ip_type == IP_HEADER_TYPE_IPV4) {
        if (rewrite_metadata.tunnel_ip == 0) {
            f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_UDP);
        } else {
            f_insert_ipv4_header(ip_sa, rewrite_metadata.tunnel_ip,
                                 IP_PROTO_UDP);
        }
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len, 18);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
        add_to_field(capri_p4_intrinsic.packet_len, 32);
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
    modify_field(udp.srcPort, (0xC000 | rewrite_metadata.entropy_hash));
    modify_field(udp.dstPort, UDP_PORT_VXLAN_GPE);
    modify_field(udp.checksum, 0);
    add(udp.len, capri_p4_intrinsic.packet_len, 16);

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
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len, 36);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
        add_to_field(capri_p4_intrinsic.packet_len, 50);
    } else {
#ifdef PHASE2
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv6.payloadLen, capri_p4_intrinsic.packet_len, 16);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
        add_to_field(capri_p4_intrinsic.packet_len, 70);
#endif /* PHASE2 */
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

    modify_field(udp.srcPort, (0xC000 | rewrite_metadata.entropy_hash));
    modify_field(udp.dstPort, UDP_PORT_GENV);
    modify_field(udp.checksum, 0);
    add(udp.len, capri_p4_intrinsic.packet_len, 16);

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
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len, 36);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
        add_to_field(capri_p4_intrinsic.packet_len, 50);
    } else {
#ifdef PHASE2
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_UDP);
        add(ipv6.payloadLen, capri_p4_intrinsic.packet_len, 16);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
        add_to_field(capri_p4_intrinsic.packet_len, 70);
#endif /* PHASE2 */
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
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len, 28);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
        add_to_field(capri_p4_intrinsic.packet_len, 42);
    } else {
#ifdef PHASE2
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_GRE);
        add(ipv6.payloadLen, capri_p4_intrinsic.packet_len, 8);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
        add_to_field(capri_p4_intrinsic.packet_len, 62);
#endif /* PHASE2 */
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
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len, 24);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
        add_to_field(capri_p4_intrinsic.packet_len, 38);
    } else {
#ifdef PHASE2
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_GRE);
        add(ipv6.payloadLen, capri_p4_intrinsic.packet_len, 4);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV6);
        add_to_field(capri_p4_intrinsic.packet_len, 58);
#endif /* PHASE2 */
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
    add_header(erspan_t3);
    add_header(erspan_t3_opt);
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
    modify_field(erspan_t3.version, 2);
    modify_field(erspan_t3.sgt, 0);
    if (capri_intrinsic.tm_iport == TM_PORT_EGRESS) {
        modify_field(erspan_t3.direction, 1);
    }
    modify_field(erspan_t3.granularity, 0x3);
    if (control_metadata.current_time_in_ns != 0) {
        modify_field(erspan_t3.options, 1);
        modify_field(erspan_t3_opt.platf_id, 0x3);
        modify_field(erspan_t3_opt.port_id, capri_intrinsic.lif);
        modify_field(erspan_t3.timestamp, control_metadata.current_time_in_ns);
        modify_field(erspan_t3_opt.timestamp,
                     control_metadata.current_time_in_ns >> 32);
    }
}

action encap_erspan(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    f_insert_erspan_t3_header(mac_sa, mac_da);
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_GRE);
        if (control_metadata.current_time_in_ns != 0) {
            /* IPv4 header (20) + GRE (4) + ERSPAN (12) + ERSPAN options (8) */
            modify_field(scratch_metadata.packet_len, 44);
        } else {
            /* IPv4 header (20) + GRE (4) + ERSPAN (12) */
            modify_field(scratch_metadata.packet_len, 36);
        }
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len,
            scratch_metadata.packet_len);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
        add_to_field(capri_p4_intrinsic.packet_len,
                     scratch_metadata.packet_len + 14);
    } else {
#ifdef PHASE2
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_GRE);
        if (control_metadata.current_time_in_ns != 0) {
            /* GRE (4) + ERSPAN (12) + ERSPAN options (8) */
            modify_field(scratch_metadata.packet_len, 24);
        } else {
            /* GRE (4) + ERSPAN (12) */
            modify_field(scratch_metadata.packet_len, 16);
        }
        add(ipv6.payloadLen, capri_p4_intrinsic.packet_len,
            scratch_metadata.packet_len);
        modify_field(ethernet.etherType, ETHERTYPE_IPV6);
        add_to_field(capri_p4_intrinsic.packet_len,);
                     scratch_metadata.packet_len + 40 + 14);
#endif /* PHASE2 */
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
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len, 20);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
        add_to_field(capri_p4_intrinsic.packet_len, 20);
    } else {
#ifdef PHASE2
        f_insert_ipv6_header(ip_sa, ip_da, tunnel_metadata.inner_ip_proto);
        modify_field(ipv6.payloadLen, capri_p4_intrinsic.packet_len);
        modify_field(ethernet.etherType, ETHERTYPE_IPV6);
        add_to_field(capri_p4_intrinsic.packet_len, 40);
#endif /* PHASE2 */
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
    modify_field(control_metadata.checksum_ctl, (1 << CHECKSUM_CTL_IP_CHECKSUM),
                 (1 << CHECKSUM_CTL_IP_CHECKSUM));
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

action encap_ipsec(mac_sa, mac_da, ip_sa, ip_da, ip_type, vlan_valid, vlan_id) {
    if (ip_type == 0) {
        f_insert_ipv4_header(ip_sa, ip_da, IP_PROTO_IPSEC_ESP);
        add(ipv4.totalLen, capri_p4_intrinsic.packet_len, 20);
        modify_field(scratch_metadata.ethtype, ETHERTYPE_IPV4);
    } else {
        f_insert_ipv6_header(ip_sa, ip_da, IP_PROTO_IPSEC_ESP);
        modify_field(ipv6.payloadLen, capri_p4_intrinsic.packet_len);
        modify_field(ethernet.etherType, ETHERTYPE_IPV6);
    }
    if (vlan_valid == TRUE) {
        f_encap_vlan(vlan_id, scratch_metadata.ethtype);
    } else {
        modify_field(ethernet.etherType, scratch_metadata.ethtype);
    }
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.vlan_valid, vlan_valid);
    modify_field(scratch_metadata.vlan_id, vlan_id);
    modify_field(scratch_metadata.flag, ip_type);
}

@pragma stage 4
table tunnel_rewrite {
    reads {
        rewrite_metadata.tunnel_rewrite_index : exact;
    }
    actions {
        nop;
        encap_vxlan;
        encap_erspan;
        encap_vlan;
        encap_mpls_udp;
#ifdef PHASE2
        encap_ipsec;
        encap_nvgre;
        encap_genv;
        encap_ip;
        encap_gre;
        encap_vxlan_gpe;
        encap_mpls;
#endif /* PHASE2 */
    }
    size : TUNNEL_REWRITE_TABLE_SIZE;
}

/*****************************************************************************/
/* Update L4 headers on tunnel encap                                         */
/*****************************************************************************/
action encap_inner_ipv4_udp_rewrite() {
    copy_header(inner_ipv4, ipv4);
    copy_header(inner_udp, udp);
    if (ipv4_options_blob.valid == TRUE) {
        // copy_header(inner_ipv4_options_blob2, ipv4_options_blob);
    }
    if (ipv4_options_blob2.valid == TRUE) {
        // copy_header(inner_ipv4_options_blob2, ipv4_options_blob2);
    }
    remove_header(udp);
    remove_header(ipv4);
    remove_header(ipv4_options_blob);
    remove_header(ipv4_options_blob2);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV4);

    /* update checksum flags */
    if ((control_metadata.checksum_ctl & (1 << CHECKSUM_CTL_IP_CHECKSUM)) != 0) {
        bit_or(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
               (1 << CHECKSUM_CTL_INNER_IP_CHECKSUM));
        bit_and(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
                ~(1 << CHECKSUM_CTL_IP_CHECKSUM));
    }
    if ((control_metadata.checksum_ctl & (1 << CHECKSUM_CTL_L4_CHECKSUM)) != 0) {
        bit_or(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
               (1 << CHECKSUM_CTL_INNER_L4_CHECKSUM));
        bit_and(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
                ~(1 << CHECKSUM_CTL_L4_CHECKSUM));
    }
}

action encap_inner_ipv4_tcp_rewrite() {
    copy_header(inner_ipv4, ipv4);
    if (ipv4_options_blob.valid == TRUE) {
        // copy_header(inner_ipv4_options_blob2, ipv4_options_blob);
    }
    if (ipv4_options_blob2.valid == TRUE) {
        // decap followed by encap, retain the original inner blob header
        // modify_field(inner_ipv4_options_blob.valid, TRUE);
    }
    remove_header(ipv4);
    remove_header(ipv4_options_blob);
    remove_header(ipv4_options_blob2);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV4);

    /* update checksum flags */
    if ((control_metadata.checksum_ctl & (1 << CHECKSUM_CTL_IP_CHECKSUM)) != 0) {
        bit_or(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
               (1 << CHECKSUM_CTL_INNER_IP_CHECKSUM));
        bit_and(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
                ~(1 << CHECKSUM_CTL_IP_CHECKSUM));
    }
    if ((control_metadata.checksum_ctl & (1 << CHECKSUM_CTL_L4_CHECKSUM)) != 0) {
        bit_or(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
               (1 << CHECKSUM_CTL_INNER_L4_CHECKSUM));
        bit_and(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
                ~(1 << CHECKSUM_CTL_L4_CHECKSUM));
    }
}

action encap_inner_ipv4_icmp_rewrite() {
    copy_header(inner_ipv4, ipv4);
    if (ipv4_options_blob.valid == TRUE) {
        // copy_header(inner_ipv4_options_blob2, ipv4_options_blob);
    }
    if (ipv4_options_blob2.valid == TRUE) {
        // decap followed by encap, retain the original inner blob header
        // modify_field(inner_ipv4_options_blob.valid, TRUE);
    }
    remove_header(ipv4);
    remove_header(ipv4_options_blob);
    remove_header(ipv4_options_blob2);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV4);

    /* update checksum flags */
    if ((control_metadata.checksum_ctl & (1 << CHECKSUM_CTL_IP_CHECKSUM)) != 0) {
        bit_or(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
               (1 << CHECKSUM_CTL_INNER_IP_CHECKSUM));
        bit_and(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
                ~(1 << CHECKSUM_CTL_IP_CHECKSUM));
    }
}

action encap_inner_ipv4_unknown_rewrite() {
    copy_header(inner_ipv4, ipv4);
    if (ipv4_options_blob.valid == TRUE) {
        // copy_header(inner_ipv4_options_blob2, ipv4_options_blob);
    }
    if (ipv4_options_blob2.valid == TRUE) {
        // decap followed by encap, retain the original inner blob header
        // modify_field(inner_ipv4_options_blob.valid, TRUE);
    }
    remove_header(ipv4);
    remove_header(ipv4_options_blob);
    remove_header(ipv4_options_blob2);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV4);

    /* update checksum flags */
    if ((control_metadata.checksum_ctl & (1 << CHECKSUM_CTL_IP_CHECKSUM)) != 0) {
        bit_or(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
               (1 << CHECKSUM_CTL_INNER_IP_CHECKSUM));
        bit_and(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
                ~(1 << CHECKSUM_CTL_IP_CHECKSUM));
    }
}

action encap_inner_ipv6_udp_rewrite() {
    copy_header(inner_ipv6, ipv6);
    copy_header(inner_udp, udp);
    remove_header(udp);
    remove_header(ipv6);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV6);

    /* update checksum flags */
    if ((control_metadata.checksum_ctl & (1 << CHECKSUM_CTL_L4_CHECKSUM)) != 0) {
        bit_or(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
               (1 << CHECKSUM_CTL_INNER_L4_CHECKSUM));
        bit_and(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
                ~(1 << CHECKSUM_CTL_L4_CHECKSUM));
    }
}

action encap_inner_ipv6_tcp_rewrite() {
    copy_header(inner_ipv6, ipv6);
    remove_header(ipv6);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV6);

    /* update checksum flags */
    if ((control_metadata.checksum_ctl & (1 << CHECKSUM_CTL_L4_CHECKSUM)) != 0) {
        bit_or(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
               (1 << CHECKSUM_CTL_INNER_L4_CHECKSUM));
        bit_and(control_metadata.checksum_ctl, control_metadata.checksum_ctl,
                ~(1 << CHECKSUM_CTL_L4_CHECKSUM));
    }
}

action encap_inner_ipv6_icmp_rewrite() {
    copy_header(inner_ipv6, ipv6);
    remove_header(ipv6);
    modify_field(tunnel_metadata.inner_ip_proto, IP_PROTO_IPV6);
}

action encap_inner_ipv6_unknown_rewrite() {
    copy_header(inner_ipv6, ipv6);
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
