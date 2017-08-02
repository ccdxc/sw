/*****************************************************************************/
/* NACL table processing                                                     */
/*****************************************************************************/
action nacl_permit(force_flow_hit, policer_index, log_en,
                   ingress_mirror_session_id, egress_mirror_session_id,
                   qid_en, qid) {
    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.force_flow_hit, force_flow_hit);
    modify_field(scratch_metadata.qid_en, qid_en);
    modify_field(scratch_metadata.log_en, log_en);

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
    modify_field(capri_intrinsic.tm_span_session, ingress_mirror_session_id);
    modify_field(control_metadata.egress_mirror_session_id,
                 egress_mirror_session_id);
    modify_field(copp_metadata.policer_index, policer_index);
}

action nacl_deny() {
    modify_field(control_metadata.drop_reason, DROP_NACL);
    drop_packet();
}

@pragma stage 3
table nacl {
    reads {
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
        capri_intrinsic.lif                : ternary;
        control_metadata.flow_miss_ingress : ternary;
        flow_lkp_metadata.lkp_dstMacAddr   : ternary;
        vlan_tag.valid                     : ternary;
        vlan_tag.vid                       : ternary;
        control_metadata.drop_reason       : ternary;
        l3_metadata.ipv4_option_seen       : ternary;
        l3_metadata.ipv4_frag              : ternary;
        // Removing srcMacAddr to accomodate other fields
        // flow_lkp_metadata.lkp_srcMacAddr   : ternary;
        // lkp_type and lkp_sport together can be used in
        // place of lkp_etherType (basiclly outer or inner
        // based on encap packet or not)
        // ethernet.etherType                 : ternary;
    }
    actions {
        nacl_permit;
        nacl_deny;
    }
    default_action : nacl_permit;
    size : NACL_TABLE_SIZE;
}

control process_nacl {
    apply(nacl);
}
