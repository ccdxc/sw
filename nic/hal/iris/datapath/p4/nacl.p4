/*****************************************************************************/
/* NACL table processing                                                     */
/*****************************************************************************/
action nacl_permit(force_flow_hit, policer_index, log_en,
                   ingress_mirror_en, egress_mirror_en,
                   ingress_mirror_session_id, egress_mirror_session_id,
                   qid_en, qid,
                   rewrite_en, rewrite_index, rewrite_flags,
                   tunnel_rewrite_en, tunnel_rewrite_index, tunnel_vnid,
                   tunnel_originate,
                   dst_lport_en, dst_lport, discard_drop, 
                   egress_drop, 
                   stats_idx) {
    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.force_flow_hit, force_flow_hit);
    modify_field(scratch_metadata.qid_en, qid_en);
    modify_field(scratch_metadata.log_en, log_en);
    modify_field(scratch_metadata.ingress_mirror_en, ingress_mirror_en);
    modify_field(scratch_metadata.egress_mirror_en, egress_mirror_en);
    modify_field(scratch_metadata.rewrite_en, rewrite_en);
    modify_field(scratch_metadata.tunnel_rewrite_en, tunnel_rewrite_en);
    modify_field(scratch_metadata.dst_lport_en, dst_lport_en);
    modify_field(scratch_metadata.discard_drop, discard_drop);

    if (discard_drop == TRUE) {
        permit_packet();
        modify_field(control_metadata.drop_reason, 0);
    }

    if (force_flow_hit == TRUE) {
        modify_field(control_metadata.i2e_flags, 0,
                     (1 << P4_I2E_FLAGS_FLOW_MISS));
        modify_field(control_metadata.flow_miss_ingress, FALSE);
    }
    if (qid_en == TRUE) {
        modify_field(control_metadata.qid, qid);
    }
    if (log_en == TRUE) {
        modify_field(capri_intrinsic.tm_cpu, TRUE);
    }
    if (ingress_mirror_en == TRUE) {
        modify_field(capri_intrinsic.tm_span_session, ingress_mirror_session_id);
    }

    modify_field(copp_metadata.policer_index, policer_index);
    modify_field(control_metadata.nacl_stats_idx, stats_idx);
    modify_field(control_metadata.nacl_egress_drop, egress_drop);

    if (egress_mirror_en == TRUE) {
        modify_field(control_metadata.egress_mirror_session_id,
                egress_mirror_session_id);
    }

    if (rewrite_en == TRUE) {
        modify_field(rewrite_metadata.rewrite_index, rewrite_index);
        modify_field(rewrite_metadata.flags, rewrite_flags);
    }

    if (tunnel_rewrite_en == TRUE) {
        modify_field(tunnel_metadata.tunnel_originate, tunnel_originate);
        modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
        modify_field(rewrite_metadata.tunnel_vnid, tunnel_vnid);
    }

    if (dst_lport_en == TRUE) {
        modify_field(control_metadata.dst_lport, dst_lport);
        modify_field(capri_intrinsic.tm_replicate_en, FALSE);
        modify_field(capri_intrinsic.tm_replicate_ptr, 0);
        modify_field(capri_intrinsic.tm_cpu, FALSE);
    }
}

action nacl_deny(stats_idx) {
    modify_field(control_metadata.nacl_stats_idx, stats_idx);
    modify_field(control_metadata.drop_reason, DROP_NACL);
    drop_packet();
}

@pragma stage 4
table nacl {
    reads {
        entry_inactive.nacl                 : ternary;
        capri_intrinsic.lif                 : ternary;
        // control_metadata.nic_mode           : ternary;
        control_metadata.from_cpu           : ternary;
        flow_lkp_metadata.lkp_dir           : ternary;
        flow_lkp_metadata.lkp_type          : ternary;
        flow_lkp_metadata.lkp_vrf           : ternary;
        flow_lkp_metadata.lkp_src           : ternary;
        flow_lkp_metadata.lkp_dst           : ternary;
        flow_lkp_metadata.lkp_proto         : ternary;
        flow_lkp_metadata.lkp_sport         : ternary;
        flow_lkp_metadata.lkp_dport         : ternary;
        tcp.flags                           : ternary;
        control_metadata.src_lport          : ternary;
        control_metadata.dst_lport          : ternary;
        control_metadata.flow_miss_ingress  : ternary;
        control_metadata.drop_reason        : ternary;
        l3_metadata.ip_option_seen          : ternary;
        l3_metadata.ip_frag                 : ternary;
        ethernet.dstAddr                    : ternary;
        control_metadata.ep_learn_en        : ternary;
    }
    actions {
        nop;
        nacl_permit;
        nacl_deny;
    }
    default_action : nop;
    size : NACL_TABLE_SIZE;
}

control process_nacl {
    apply(nacl);
}
