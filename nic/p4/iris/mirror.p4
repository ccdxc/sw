/*****************************************************************************/
/* Mirror                                                                    */
/*****************************************************************************/
action mirror_truncate(truncate_len) {
    if ((truncate_len != 0) and
        (truncate_len < capri_p4_intrinsic.packet_len)) {
        modify_field(capri_deparser_len.trunc_pkt_len, truncate_len);
        modify_field(capri_p4_intrinsic.packet_len, truncate_len, 14);
    }
}

action local_span(dst_lport, truncate_len, span_tm_oq) {
    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(control_metadata.dest_tm_oq, span_tm_oq);
    modify_field(rewrite_metadata.tunnel_rewrite_index, 0);
    modify_field(tunnel_metadata.tunnel_originate, FALSE);
    mirror_truncate(truncate_len);
}

action remote_span(dst_lport, truncate_len, tunnel_rewrite_index, vlan, span_tm_oq) {
    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(control_metadata.dest_tm_oq, span_tm_oq);
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(tunnel_metadata.tunnel_originate, TRUE);
    modify_field(rewrite_metadata.tunnel_vnid, vlan);
    mirror_truncate(truncate_len);
}

action erspan_mirror(dst_lport, truncate_len, tunnel_rewrite_index, span_tm_oq) {
    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(control_metadata.dest_tm_oq, span_tm_oq);
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(tunnel_metadata.tunnel_originate, TRUE);
    mirror_truncate(truncate_len);
}

action drop_mirror() {
    modify_field(control_metadata.dst_lport, 0);
    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(control_metadata.egress_drop_reason, EGRESS_DROP_MIRROR);
    drop_packet();
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
        drop_mirror;
    }
    size : MIRROR_SESSION_TABLE_SIZE;
}
