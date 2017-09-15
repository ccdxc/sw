/*****************************************************************************/
/* NACL table processing                                                     */
/*****************************************************************************/
action nacl_permit(force_flow_hit, policer_index, log_en,
                   ingress_mirror_en, egress_mirror_en, 
                   ingress_mirror_session_id, egress_mirror_session_id,
                   qid_en, qid, 
                   rewrite_en, rewrite_index,
                   tunnel_rewrite_en, tunnel_rewrite_index,
                   dst_lport_en, dst_lport,
                   egress_policer_en, egress_policer_index) {
    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.force_flow_hit, force_flow_hit);
    modify_field(scratch_metadata.qid_en, qid_en);
    modify_field(scratch_metadata.log_en, log_en);
    modify_field(scratch_metadata.ingress_mirror_en, ingress_mirror_en);
    modify_field(scratch_metadata.egress_mirror_en, egress_mirror_en);
    modify_field(scratch_metadata.rewrite_en, rewrite_en);
    modify_field(scratch_metadata.tunnel_rewrite_en, tunnel_rewrite_en);
    modify_field(scratch_metadata.dst_lport_en, dst_lport_en);
    modify_field(scratch_metadata.egress_policer_en, egress_policer_en);

    if (force_flow_hit == TRUE) {
        modify_field(control_metadata.flow_miss, FALSE);
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

    if (egress_mirror_en == TRUE) {
        modify_field(control_metadata.egress_mirror_session_id,
                egress_mirror_session_id);
    }

    if (rewrite_en == TRUE) {
        modify_field(rewrite_metadata.rewrite_index, rewrite_index);
    }

    if (tunnel_rewrite_en == TRUE) {
        modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    }

    if (dst_lport_en == TRUE) {
        modify_field(control_metadata.dst_lport, dst_lport);
    }

    if (egress_policer_en == TRUE) {
        modify_field(policer_metadata.egress_policer_index, egress_policer_index);
    }
}

action nacl_deny() {
    modify_field(control_metadata.drop_reason, DROP_NACL);
    drop_packet();
}

@pragma stage 3
table nacl {
    reads {
        // lkp_vrf identifies a segment and the vni/vlan derive a lkp_vrf. 
        // So these fields are not needed
        // vxlan.vni                                          : ternary;
        // vlan_tag.valid                                     : ternary;
        // vlan_tag.vid                                       : ternary;
        entry_status.inactive                              : ternary;
        flow_lkp_metadata.lkp_type                         : ternary;
        flow_lkp_metadata.lkp_vrf                          : ternary;
        flow_lkp_metadata.lkp_src                          : ternary;
        flow_lkp_metadata.lkp_dst                          : ternary;
        flow_lkp_metadata.lkp_proto                        : ternary;
        flow_lkp_metadata.lkp_sport                        : ternary;
        flow_lkp_metadata.lkp_dport                        : ternary;
        tcp.flags                                          : ternary;
        tunnel_metadata.tunnel_terminate                   : ternary;
        control_metadata.src_lport                         : ternary;
        control_metadata.dst_lport                         : ternary;
        control_metadata.flow_miss_ingress                 : ternary;
        control_metadata.drop_reason                       : ternary;
        l3_metadata.ip_option_seen                         : ternary;
        l3_metadata.tcp_option_seen                        : ternary;
        l3_metadata.ip_frag                                : ternary;
        // Outer mac da for tunnel cases
        ethernet.dstAddr                                   : ternary;
        // Removing srcMacAddr to accomodate other fields
        // flow_lkp_metadata.lkp_srcMacAddr                : ternary;
        // flow_lkp_metadata.lkp_dstMacAddr                : ternary;
        // lkp_type and lkp_sport together can be used in
        // place of lkp_etherType (basiclly outer or inner
        // based on encap packet or not)
        // ethernet.etherType                              : ternary;
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
