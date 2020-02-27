/*****************************************************************************/
/* Rewrite table                                                             */
/*****************************************************************************/
header_type rewrite_metadata_t {
    fields {
        rewrite_index        : 12;
        tunnel_rewrite_index : 10;
        tunnel_vnid          : 24;
        entropy_hash         : 32;
        flags                : 8;
        tunnel_ip            : 32;
    }
}

metadata rewrite_metadata_t rewrite_metadata;

action rewrite(mac_sa, mac_da) {
    if (vlan_tag.valid == TRUE) {
        modify_field(ethernet.etherType, vlan_tag.etherType);
        subtract(capri_p4_intrinsic.packet_len,
                 capri_p4_intrinsic.packet_len, 4);
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
            modify_field(control_metadata.checksum_ctl,
                         (1 << CHECKSUM_CTL_IP_CHECKSUM),
                         (1 << CHECKSUM_CTL_IP_CHECKSUM));
        } else {
            if (ipv6.valid == TRUE) {
                subtract(ipv6.hopLimit, ipv6.hopLimit, 1);
            }
        }
    }

    if (qos_metadata.dscp_en == TRUE) {
        if (ipv4.valid == TRUE) {
            modify_field(ipv4.diffserv, qos_metadata.dscp);
            modify_field(control_metadata.checksum_ctl,
                         (1 << CHECKSUM_CTL_IP_CHECKSUM),
                         (1 << CHECKSUM_CTL_IP_CHECKSUM));
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
    }
    default_action : nop;
    size : REWRITE_TABLE_SIZE;
}

control process_rewrites {
    apply(replica);
    if (control_metadata.span_copy == TRUE) {
        apply(mirror);
        apply(clock);
    } else {
        if (tunnel_metadata.tunnel_terminate_egress == TRUE) {
            apply(tunnel_decap_copy_inner);
            apply(tunnel_decap);
        }
        apply(rewrite);
    }
    if (tunnel_metadata.tunnel_originate_egress == TRUE) {
        apply(tunnel_encap_update_inner);
    }
    apply(tunnel_rewrite);
    apply(p4plus_app_prep);
    apply(p4plus_app);
}
