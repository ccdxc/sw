/*****************************************************************************/
/* Mirror                                                                    */
/*****************************************************************************/
action mirror_truncate(truncate_len, is_erspan) {
    if ((truncate_len != 0) and
        (truncate_len < capri_p4_intrinsic.packet_len)) {
        modify_field(capri_deparser_len.trunc_pkt_len, truncate_len);
        modify_field(capri_p4_intrinsic.packet_len, truncate_len, 14);
        if (is_erspan == TRUE) {
            modify_field(erspan_t3.truncated, TRUE);
        }
    }
}

action local_span(dst_lport, truncate_len, span_tm_oq) {
    if (vlan_tag.valid == TRUE) {
        remove_header(vlan_tag);
    }

    modify_field(p4_to_p4plus_mirror.session_id,
                 capri_intrinsic.tm_span_session);
    modify_field(p4_to_p4plus_mirror.original_len,
                 capri_p4_intrinsic.packet_len);
    modify_field(p4_to_p4plus_mirror.lif, capri_intrinsic.lif);

    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(control_metadata.dest_tm_oq, span_tm_oq);
    modify_field(rewrite_metadata.tunnel_rewrite_index, 0);
    modify_field(tunnel_metadata.tunnel_originate, FALSE);
    mirror_truncate(truncate_len, FALSE);
}

action remote_span(dst_lport, truncate_len, tunnel_rewrite_index, vlan, span_tm_oq) {
    if (vlan_tag.valid == TRUE) {
        remove_header(vlan_tag);
    }

    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(control_metadata.dest_tm_oq, span_tm_oq);
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(tunnel_metadata.tunnel_originate, TRUE);
    modify_field(rewrite_metadata.tunnel_vnid, vlan);
    mirror_truncate(truncate_len, FALSE);
}

action erspan_mirror(dst_lport, truncate_len, tunnel_rewrite_index, span_tm_oq) {
    if (vlan_tag.valid == TRUE) {
        modify_field(erspan_t3.vlan, vlan_tag.vid);
        modify_field(erspan_t3.cos, vlan_tag.pcp);
        remove_header(vlan_tag);
    }

    modify_field(erspan_t3.span_id, capri_intrinsic.tm_span_session);
    modify_field(capri_intrinsic.tm_span_session, 0);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(control_metadata.dest_tm_oq, span_tm_oq);
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(tunnel_metadata.tunnel_originate, TRUE);
    mirror_truncate(truncate_len, TRUE);
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
