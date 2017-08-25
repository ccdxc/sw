/*****************************************************************************/
/* Rewrite table                                                             */
/*****************************************************************************/
header_type rewrite_metadata_t {
    fields {
        rewrite_index        : 12;
        tunnel_rewrite_index : 10;
        tunnel_vnid          : 24;
        entropy_hash         : 16;
        flags                : 8;
    }
}

header_type qos_metadata_t {
    fields {
        cos_en  : 1;
        cos     : 3;
        dscp_en : 1;
        dscp    : 8;
    }
}

metadata rewrite_metadata_t rewrite_metadata;
metadata qos_metadata_t qos_metadata;

action rewrite(mac_sa, mac_da) {
    if (vlan_tag.valid == TRUE) {
        modify_field(ethernet.etherType, vlan_tag.etherType);
        remove_header(vlan_tag);
    }

    if ((rewrite_metadata.flags & REWRITE_FLAGS_MAC_SA) != 0) {
        modify_field(ethernet.srcAddr, mac_sa);
    }

    if ((rewrite_metadata.flags & REWRITE_FLAGS_MAC_DA) != 0) {
        modify_field(ethernet.dstAddr, mac_da);
    }

    if ((rewrite_metadata.flags & REWRITE_FLAGS_TTL_DEC) != 0) {
        if (ipv4.valid == TRUE) {
            subtract(ipv4.ttl, ipv4.ttl, 1);
        } else {
            if (ipv6.valid == TRUE) {
                subtract(ipv6.hopLimit, ipv6.hopLimit, 1);
            }
        }
    }

    if (qos_metadata.dscp_en == TRUE) {
        if (ipv4.valid == TRUE) {
            modify_field(ipv4.diffserv, qos_metadata.dscp);
        } else {
            if (ipv6.valid == TRUE) {
                modify_field(ipv6.trafficClass, qos_metadata.dscp);
            }
        }
    }
}

@pragma stage 2
table rewrite {
    reads {
        rewrite_metadata.rewrite_index : exact;
    }
    actions {
        nop;
        rewrite;
        ipv4_nat_src_rewrite;
        ipv4_nat_dst_rewrite;
        ipv4_nat_src_udp_rewrite;
        ipv4_nat_dst_udp_rewrite;
        ipv4_nat_src_tcp_rewrite;
        ipv4_nat_dst_tcp_rewrite;
        ipv4_twice_nat_rewrite;
        ipv4_twice_nat_udp_rewrite;
        ipv4_twice_nat_tcp_rewrite;
        ipv6_nat_src_rewrite;
        ipv6_nat_dst_rewrite;
        ipv6_nat_src_udp_rewrite;
        ipv6_nat_dst_udp_rewrite;
        ipv6_nat_src_tcp_rewrite;
        ipv6_nat_dst_tcp_rewrite;
        ipv6_twice_nat_rewrite;
        ipv6_twice_nat_udp_rewrite;
        ipv6_twice_nat_tcp_rewrite;
    }
    default_action : nop;
    size : REWRITE_TABLE_SIZE;
}

/*****************************************************************************/
/* Mirror                                                                    */
/*****************************************************************************/
action mirror_truncate(truncate_len) {
    if (truncate_len != 0) {
        modify_field(capri_p4_intrinsic.frame_size, truncate_len);
    }
}

action local_span(dst_lport, truncate_len) {
    modify_field(control_metadata.dst_lport, dst_lport);
    mirror_truncate(truncate_len);
}

action remote_span(dst_lport, truncate_len, tunnel_rewrite_index, vlan) {
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(tunnel_metadata.tunnel_originate, TRUE);
    modify_field(rewrite_metadata.tunnel_vnid, vlan);
    mirror_truncate(truncate_len);
}

action erspan_mirror(dst_lport, truncate_len, tunnel_rewrite_index) {
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(tunnel_metadata.tunnel_originate, TRUE);
    mirror_truncate(truncate_len);
}

@pragma stage 0
table mirror {
    reads {
        capri_intrinsic.tm_span_session : exact;
    }
    actions {
        nop;
        local_span;
        remote_span;
        erspan_mirror;
    }
    size : MIRROR_SESSION_TABLE_SIZE;
}

control process_rewrites {
    if (capri_intrinsic.tm_instance_type == TM_INSTANCE_TYPE_SPAN) {
        apply(mirror);
    } else {
        if (tunnel_metadata.tunnel_terminate_egress == TRUE) {
            apply(tunnel_decap_copy_inner);
            apply(tunnel_decap);
        }
        apply(twice_nat);
        apply(rewrite);
    }
    if (tunnel_metadata.tunnel_originate_egress == TRUE) {
        apply(tunnel_encap_update_inner);
    }
    apply(tunnel_rewrite);
    apply(p4plus_app);
}
