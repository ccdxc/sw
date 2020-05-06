/*****************************************************************************/
/* Network ACL                                                               */
/*****************************************************************************/
action nacl_permit() {
}

action nacl_drop() {
    ingress_drop(P4I_DROP_NACL);
}

action nacl_redirect(nexthop_type, nexthop_id, copp_policer_id) {
    if (arm_to_p4i.nexthop_valid == TRUE) {
        modify_field(p4i_i2e.nexthop_type, arm_to_p4i.nexthop_type);
        modify_field(p4i_i2e.nexthop_id, arm_to_p4i.nexthop_id);
    } else {
        modify_field(p4i_i2e.nexthop_type, nexthop_type);
        modify_field(p4i_i2e.nexthop_id, nexthop_id);
        modify_field(p4i_i2e.copp_policer_id, copp_policer_id);
    }

    modify_field(p4i_i2e.mapping_bypass, TRUE);
    modify_field(p4i_i2e.session_id, -1);
    modify_field(capri_intrinsic.drop, 0);
    modify_field(control_metadata.p4i_drop_reason, 0);
}

action nacl_redirect_to_arm(nexthop_type, nexthop_id, copp_policer_id, data) {
    modify_field(p4i_i2e.nexthop_type, nexthop_type);
    modify_field(p4i_i2e.nexthop_id, nexthop_id);
    modify_field(p4i_i2e.session_id, -1);
    modify_field(p4i_i2e.copp_policer_id, copp_policer_id);
    modify_field(control_metadata.redirect_to_arm, TRUE);
    modify_field(p4i_to_arm.nacl_data, data);
}

@pragma stage 4
table nacl {
    reads {
        vnic_metadata.vnic_id               : ternary;
        key_metadata.flow_lkp_id            : ternary;
        key_metadata.ktype                  : ternary;
        key_metadata.src                    : ternary;
        key_metadata.dst                    : ternary;
        key_metadata.proto                  : ternary;
        key_metadata.sport                  : ternary;
        key_metadata.dport                  : ternary;
        ethernet_1.dstAddr                  : ternary;
        capri_intrinsic.lif                 : ternary;
        control_metadata.rx_packet          : ternary;
        control_metadata.tunneled_packet    : ternary;
        control_metadata.flow_miss          : ternary;
        control_metadata.local_mapping_miss : ternary;
        control_metadata.learn_enabled      : ternary;
        control_metadata.lif_type           : ternary;
        arm_to_p4i.nexthop_valid            : ternary;
        key_metadata.entry_valid            : ternary;
    }
    actions {
        nacl_permit;
        nacl_redirect;
        nacl_redirect_to_arm;
        nacl_drop;
    }
    size : NACL_TABLE_SIZE;
}

control nacl {
    apply(nacl);
}
