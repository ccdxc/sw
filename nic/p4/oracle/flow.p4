/*****************************************************************************/
/* Common actions                                                            */
/*****************************************************************************/
action flow_hash_info(entry_valid, prio, flow_index,
                      parent_policer_index, child_policer_index,
                      hash1, hint1, hash2, hint2, hash3, hint3,
                      hash4, hint4, hash5, hint5, hash6, hint6,
                      hash7, hint7, hash8, hint8, hash9, hint9,
                      hash10, hint10) {
    if (entry_valid == TRUE) {
        // if register c1 indicates hit, take the results.
        modify_field(flow_action_metadata.prio, prio);
        modify_field(flow_action_metadata.flow_index, flow_index);
        modify_field(flow_action_metadata.parent_policer_index,
                     parent_policer_index);
        modify_field(flow_action_metadata.child_policer_index,
                     child_policer_index);

        // if register c1 indicates miss, compare hints and lookup
        // overflow hash table
        modify_field(flow_action_metadata.overflow_lkp, TRUE);
        modify_field(flow_lkp_metadata.overflow_hash, hint1);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.hash, hash1);
    modify_field(scratch_metadata.hash, hash2);
    modify_field(scratch_metadata.hash, hash3);
    modify_field(scratch_metadata.hash, hash4);
    modify_field(scratch_metadata.hash, hash5);
    modify_field(scratch_metadata.hash, hash6);
    modify_field(scratch_metadata.hash, hash7);
    modify_field(scratch_metadata.hash, hash8);
    modify_field(scratch_metadata.hash, hash9);
    modify_field(scratch_metadata.hash, hash10);
    modify_field(scratch_metadata.hint, hint1);
    modify_field(scratch_metadata.hint, hint2);
    modify_field(scratch_metadata.hint, hint3);
    modify_field(scratch_metadata.hint, hint4);
    modify_field(scratch_metadata.hint, hint5);
    modify_field(scratch_metadata.hint, hint6);
    modify_field(scratch_metadata.hint, hint7);
    modify_field(scratch_metadata.hint, hint8);
    modify_field(scratch_metadata.hint, hint9);
    modify_field(scratch_metadata.hint, hint10);
}

action flow_tcam_info(tcam_index, prio, flow_index, parent_policer_index,
                      child_policer_index) {
    modify_field(flow_action_metadata.tcam_prio, prio);
    modify_field(flow_action_metadata.tcam_index, tcam_index);
    modify_field(flow_action_metadata.tcam_flow_index, flow_index);
    modify_field(flow_action_metadata.tcam_parent_policer_index,
                 parent_policer_index);
    modify_field(flow_action_metadata.tcam_child_policer_index,
                 child_policer_index);
}


action flow_result() {
    if (flow_action_metadata.tcam_prio < flow_action_metadata.prio) {
        modify_field(flow_action_metadata.flow_index,
                     flow_action_metadata.tcam_flow_index);
        modify_field(flow_action_metadata.parent_policer_index,
                     flow_action_metadata.tcam_parent_policer_index);
        modify_field(flow_action_metadata.child_policer_index,
                     flow_action_metadata.tcam_child_policer_index);
    }
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
@pragma stage 2
@pragma hbm_table
table ingress_flow_hash {
    reads {
        flow_lkp_metadata.ethernet_dst_1     : exact;
        flow_lkp_metadata.ethernet_src_1     : exact;
        flow_lkp_metadata.ethernet_type_1    : exact;
        flow_lkp_metadata.ctag_1             : exact;
        flow_lkp_metadata.stag_1             : exact;
        flow_lkp_metadata.ip_src_1           : exact;
        flow_lkp_metadata.ip_dst_1           : exact;
        flow_lkp_metadata.ip_dscp_1          : exact;
        flow_lkp_metadata.ip_proto_1         : exact;
        flow_lkp_metadata.ip_ttl_1           : exact;
        flow_lkp_metadata.l4_sport_1         : exact;
        flow_lkp_metadata.l4_dport_1         : exact;
        flow_lkp_metadata.tcp_flags_1        : exact;
        flow_lkp_metadata.tenant_id_1        : exact;
        flow_lkp_metadata.gre_proto_1        : exact;

        flow_lkp_metadata.ethernet_dst_2     : exact;
        flow_lkp_metadata.ethernet_src_2     : exact;
        flow_lkp_metadata.ethernet_type_2    : exact;
        flow_lkp_metadata.ctag_2             : exact;
        flow_lkp_metadata.stag_2             : exact;
        flow_lkp_metadata.ip_src_2           : exact;
        flow_lkp_metadata.ip_dst_2           : exact;
        flow_lkp_metadata.ip_dscp_2          : exact;
        flow_lkp_metadata.ip_proto_2         : exact;
        flow_lkp_metadata.ip_ttl_2           : exact;
        flow_lkp_metadata.l4_sport_2         : exact;
        flow_lkp_metadata.l4_dport_2         : exact;
        flow_lkp_metadata.tcp_flags_2        : exact;
        flow_lkp_metadata.tenant_id_2        : exact;
        flow_lkp_metadata.gre_proto_2        : exact;

        flow_lkp_metadata.custom_field_1     : exact;
        flow_lkp_metadata.custom_field_2     : exact;
        flow_lkp_metadata.custom_field_3     : exact;
    }
    actions {
        flow_hash_info;
    }
    size : FLOW_HASH_TABLE_SIZE;
}

@pragma stage 2
table ingress_flow_tcam_1 {
    reads {
        flow_lkp_metadata.ethernet_dst_1     : ternary;
        flow_lkp_metadata.ethernet_src_1     : ternary;
        flow_lkp_metadata.ethernet_type_1    : ternary;
        flow_lkp_metadata.ip_src_1           : ternary;
        flow_lkp_metadata.ip_dst_1           : ternary;
        flow_lkp_metadata.ip_dscp_1          : ternary;
        flow_lkp_metadata.ip_proto_1         : ternary;
        flow_lkp_metadata.l4_sport_1         : ternary;
        flow_lkp_metadata.l4_dport_1         : ternary;
        flow_lkp_metadata.tcp_flags_1        : ternary;
        flow_lkp_metadata.tenant_id_1        : ternary;
    }
    actions {
        flow_tcam_info;
    }
    size : FLOW_TCAM_TABLE_SIZE;
}

@pragma stage 3
table ingress_flow_tcam_2 {
    reads {
        flow_action_metadata.tcam_index      : ternary;
        flow_lkp_metadata.ethernet_dst_2     : ternary;
        flow_lkp_metadata.ethernet_src_2     : ternary;
        flow_lkp_metadata.ethernet_type_2    : ternary;
        flow_lkp_metadata.ip_src_2           : ternary;
        flow_lkp_metadata.ip_dst_2           : ternary;
        flow_lkp_metadata.ip_dscp_2          : ternary;
        flow_lkp_metadata.ip_proto_2         : ternary;
        flow_lkp_metadata.l4_sport_2         : ternary;
        flow_lkp_metadata.l4_dport_2         : ternary;
        flow_lkp_metadata.tcp_flags_2        : ternary;
        flow_lkp_metadata.tenant_id_2        : ternary;
    }
    actions {
        flow_tcam_info;
    }
    size : FLOW_TCAM_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table ingress_flow_hash
table ingress_flow_hash_overflow {
    reads {
        flow_lkp_metadata.overflow_hash : exact;
    }
    actions {
        flow_hash_info;
    }
    size : FLOW_HASH_OVERFLOW_TABLE_SIZE;
}

@pragma stage 4
table ingress_flow_result {
    actions {
        flow_result;
    }
}

control ingress_flow_lookup {
    apply(ingress_flow_hash);
    if (flow_action_metadata.overflow_lkp == TRUE) {
        apply(ingress_flow_hash_overflow);
    }
    apply(ingress_flow_tcam_1);
    apply(ingress_flow_tcam_2);
    apply(ingress_flow_result);
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
@pragma stage 2
@pragma hbm_table
table egress_flow_hash {
    reads {
        flow_lkp_metadata.ethernet_dst_1     : exact;
        flow_lkp_metadata.ethernet_src_1     : exact;
        flow_lkp_metadata.ethernet_type_1    : exact;
        flow_lkp_metadata.ctag_1             : exact;
        flow_lkp_metadata.stag_1             : exact;
        flow_lkp_metadata.ip_src_1           : exact;
        flow_lkp_metadata.ip_dst_1           : exact;
        flow_lkp_metadata.ip_dscp_1          : exact;
        flow_lkp_metadata.ip_proto_1         : exact;
        flow_lkp_metadata.ip_ttl_1           : exact;
        flow_lkp_metadata.l4_sport_1         : exact;
        flow_lkp_metadata.l4_dport_1         : exact;
        flow_lkp_metadata.tcp_flags_1        : exact;
        flow_lkp_metadata.tenant_id_1        : exact;
        flow_lkp_metadata.gre_proto_1        : exact;

        flow_lkp_metadata.ethernet_dst_2     : exact;
        flow_lkp_metadata.ethernet_src_2     : exact;
        flow_lkp_metadata.ethernet_type_2    : exact;
        flow_lkp_metadata.ctag_2             : exact;
        flow_lkp_metadata.stag_2             : exact;
        flow_lkp_metadata.ip_src_2           : exact;
        flow_lkp_metadata.ip_dst_2           : exact;
        flow_lkp_metadata.ip_dscp_2          : exact;
        flow_lkp_metadata.ip_proto_2         : exact;
        flow_lkp_metadata.ip_ttl_2           : exact;
        flow_lkp_metadata.l4_sport_2         : exact;
        flow_lkp_metadata.l4_dport_2         : exact;
        flow_lkp_metadata.tcp_flags_2        : exact;
        flow_lkp_metadata.tenant_id_2        : exact;
        flow_lkp_metadata.gre_proto_2        : exact;

        flow_lkp_metadata.custom_field_1     : exact;
        flow_lkp_metadata.custom_field_2     : exact;
        flow_lkp_metadata.custom_field_3     : exact;
    }
    actions {
        flow_hash_info;
    }
    size : FLOW_HASH_TABLE_SIZE;
}

@pragma stage 2
table egress_flow_tcam_1 {
    reads {
        flow_lkp_metadata.ethernet_dst_1     : ternary;
        flow_lkp_metadata.ethernet_src_1     : ternary;
        flow_lkp_metadata.ethernet_type_1    : ternary;
        flow_lkp_metadata.ip_src_1           : ternary;
        flow_lkp_metadata.ip_dst_1           : ternary;
        flow_lkp_metadata.ip_dscp_1          : ternary;
        flow_lkp_metadata.ip_proto_1         : ternary;
        flow_lkp_metadata.l4_sport_1         : ternary;
        flow_lkp_metadata.l4_dport_1         : ternary;
        flow_lkp_metadata.tcp_flags_1        : ternary;
        flow_lkp_metadata.tenant_id_1        : ternary;
    }
    actions {
        flow_tcam_info;
    }
    size : FLOW_TCAM_TABLE_SIZE;
}

@pragma stage 3
table egress_flow_tcam_2 {
    reads {
        flow_action_metadata.tcam_index      : ternary;
        flow_lkp_metadata.ethernet_dst_2     : ternary;
        flow_lkp_metadata.ethernet_src_2     : ternary;
        flow_lkp_metadata.ethernet_type_2    : ternary;
        flow_lkp_metadata.ip_src_2           : ternary;
        flow_lkp_metadata.ip_dst_2           : ternary;
        flow_lkp_metadata.ip_dscp_2          : ternary;
        flow_lkp_metadata.ip_proto_2         : ternary;
        flow_lkp_metadata.l4_sport_2         : ternary;
        flow_lkp_metadata.l4_dport_2         : ternary;
        flow_lkp_metadata.tcp_flags_2        : ternary;
        flow_lkp_metadata.tenant_id_2        : ternary;
    }
    actions {
        flow_tcam_info;
    }
    size : FLOW_TCAM_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table egress_flow_hash
table egress_flow_hash_overflow {
    reads {
        flow_lkp_metadata.overflow_hash : exact;
    }
    actions {
        flow_hash_info;
    }
    size : FLOW_HASH_OVERFLOW_TABLE_SIZE;
}

@pragma stage 4
table egress_flow_result {
    actions {
        flow_result;
    }
}

control egress_flow_lookup {
    apply(egress_flow_hash);
    if (flow_action_metadata.overflow_lkp == TRUE) {
        apply(egress_flow_hash_overflow);
    }
    apply(egress_flow_tcam_1);
    apply(egress_flow_tcam_2);
    apply(egress_flow_result);
}
