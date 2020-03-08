action mirror_truncate(truncate_len, is_erspan) {
    modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.frame_size,
             offset_metadata.l2_1);
    if ((truncate_len != 0) and
        (truncate_len < (capri_p4_intrinsic.packet_len - 14))) {
        modify_field(capri_deparser_len.trunc_pkt_len, truncate_len);
        add(capri_p4_intrinsic.packet_len, truncate_len, 14);
        if (is_erspan == TRUE) {
            modify_field(erspan.truncated, TRUE);
        }
    }
}

action lspan(nexthop_type, nexthop_id, truncate_len) {
    modify_field(egress_recirc.mapping_done, TRUE);
    modify_field(control_metadata.mapping_done, TRUE);

    mirror_truncate(truncate_len, FALSE);

    remove_header(mirror_blob);
    remove_header(ctag_1);
    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(rewrite_metadata.nexthop_type, nexthop_type);
    modify_field(p4e_i2e.nexthop_id, nexthop_id);
}

action rspan(nexthop_type, nexthop_id, ctag, truncate_len) {
    modify_field(egress_recirc.mapping_done, TRUE);
    modify_field(control_metadata.mapping_done, TRUE);

    add_header(ctag_1);
    modify_field(ctag_1.vid, ctag);
    modify_field(ctag_1.dei, 0);
    modify_field(ctag_1.pcp, 0);
    modify_field(ctag_1.etherType, ethernet_1.etherType);
    modify_field(ethernet_1.etherType, ETHERTYPE_VLAN);

    mirror_truncate(truncate_len, FALSE);
    add_to_field(capri_p4_intrinsic.packet_len, 4);

    remove_header(mirror_blob);
    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(rewrite_metadata.nexthop_type, nexthop_type);
    modify_field(p4e_i2e.nexthop_id, nexthop_id);
}

action erspan(nexthop_type, nexthop_id, apply_tunnel2, tunnel2_id, tunnel2_vni,
              ctag, truncate_len, dmac, smac, sip, dip, span_id, dscp) {
    modify_field(egress_recirc.mapping_done, TRUE);
    modify_field(control_metadata.mapping_done, TRUE);

    add_header(ethernet_0);
    add_header(ipv4_0);
    add_header(gre_0);
    add_header(erspan);

    mirror_truncate(truncate_len, FALSE);

    modify_field(ethernet_0.dstAddr, dmac);
    modify_field(ethernet_0.srcAddr, smac);

    if (ctag == 0) {
        modify_field(ethernet_0.etherType, ETHERTYPE_IPV4);
        modify_field(scratch_metadata.packet_len, 50);
    } else {
        add_header(ctag_0);
        modify_field(ethernet_0.etherType, ETHERTYPE_VLAN);
        modify_field(ctag_0.vid, ctag);
        modify_field(ctag_0.etherType, ETHERTYPE_IPV4);
        modify_field(scratch_metadata.packet_len, 54);
    }

    modify_field(ipv4_0.version, 4);
    modify_field(ipv4_0.ihl, 5);
    modify_field(ipv4_0.diffserv, dscp);
    add(ipv4_0.totalLen, capri_p4_intrinsic.packet_len, 36);
    add_to_field(capri_p4_intrinsic.packet_len, scratch_metadata.packet_len);
    modify_field(ipv4_0.ttl, 64);
    modify_field(ipv4_0.protocol, IP_PROTO_GRE);
    modify_field(ipv4_0.srcAddr, sip);
    modify_field(ipv4_0.dstAddr, dip);

    modify_field(gre_0.C, 0);
    modify_field(gre_0.R, 0);
    modify_field(gre_0.K, 0);
    modify_field(gre_0.S, 0);
    modify_field(gre_0.s, 0);
    modify_field(gre_0.recurse, 0);
    modify_field(gre_0.flags, 0);
    modify_field(gre_0.ver, 0);
    modify_field(gre_0.proto, GRE_PROTO_ERSPAN_T3);
    modify_field(erspan.timestamp, capri_intrinsic.timestamp);
    modify_field(erspan.version, 2);
    modify_field(erspan.sgt, 0);
    if (capri_intrinsic.tm_iport == TM_PORT_EGRESS) {
        modify_field(erspan.direction, 1);
    }
    modify_field(erspan.granularity, 0x3);
    if (ctag_1.valid == TRUE) {
        modify_field(erspan.vlan, ctag_1.vid);
        modify_field(erspan.cos, ctag_1.pcp);
    }
    modify_field(erspan.span_id, span_id);

    remove_header(mirror_blob);
    remove_header(ctag_1);
    modify_field(capri_intrinsic.tm_span_session, 0);

    modify_field(rewrite_metadata.nexthop_type, nexthop_type);
    modify_field(p4e_i2e.nexthop_id, nexthop_id);
    modify_field(control_metadata.apply_tunnel2, apply_tunnel2);
    modify_field(rewrite_metadata.tunnel2_id, tunnel2_id);
    modify_field(rewrite_metadata.tunnel2_vni, tunnel2_vni);
}

@pragma stage 0
table mirror {
    reads {
        capri_intrinsic.tm_span_session : exact;
    }
    actions {
        lspan;
        rspan;
        erspan;
    }
    size : MIRROR_SESSION_TABLE_SIZE;
}

control mirror {
    apply(mirror);
}
