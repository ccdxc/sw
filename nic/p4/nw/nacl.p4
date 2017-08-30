/*****************************************************************************/
/* NACL table processing                                                     */
/*****************************************************************************/
action nacl_permit(force_flow_hit, policer_index, log_en,
                   ingress_mirror_en, egress_mirror_en, 
                   ingress_mirror_session_id, egress_mirror_session_id,
                   qid_en, qid) {
    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.force_flow_hit, force_flow_hit);
    modify_field(scratch_metadata.qid_en, qid_en);
    modify_field(scratch_metadata.log_en, log_en);
    modify_field(scratch_metadata.ingress_mirror_en, ingress_mirror_en);
    modify_field(scratch_metadata.egress_mirror_en, egress_mirror_en);

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
}

action nacl_deny() {
    modify_field(control_metadata.drop_reason, DROP_NACL);
    drop_packet();
}

@pragma stage 3
table nacl {
    reads {
        entry_status.inactive              : ternary;
        flow_lkp_metadata.lkp_type         : ternary;
        flow_lkp_metadata.lkp_vrf          : ternary;
        flow_lkp_metadata.lkp_src          : ternary;
        flow_lkp_metadata.lkp_dst          : ternary;
        flow_lkp_metadata.lkp_proto        : ternary;
        flow_lkp_metadata.lkp_sport        : ternary;
        flow_lkp_metadata.lkp_dport        : ternary;
        tcp.flags                          : ternary;
        tunnel_metadata.tunnel_terminate   : ternary;
        vxlan.vni                          : ternary;
        control_metadata.src_lport         : ternary;
        control_metadata.dst_lport         : ternary;
        control_metadata.flow_miss_ingress : ternary;
        vlan_tag.valid                     : ternary;
        vlan_tag.vid                       : ternary;
        control_metadata.drop_reason       : ternary;
        l3_metadata.ipv4_option_seen       : ternary;
        l3_metadata.ipv4_frag              : ternary;
        // Removing srcMacAddr to accomodate other fields
        // flow_lkp_metadata.lkp_srcMacAddr   : ternary;
        // flow_lkp_metadata.lkp_dstMacAddr   : ternary; 
        // lkp_type and lkp_sport together can be used in
        // place of lkp_etherType (basiclly outer or inner
        // based on encap packet or not)
        // ethernet.etherType                 : ternary;
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
