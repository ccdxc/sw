/*****************************************************************************/
/* Network ACL                                                               */
/*****************************************************************************/
action nacl_permit() {
}

action nacl_drop() {
    ingress_drop(P4I_DROP_NACL);
}

action nacl_redirect(nexthop_type, nexthop_id, copp_policer_id) {
    modify_field(p4i_i2e.mapping_bypass, TRUE);
    modify_field(p4i_i2e.nexthop_type, nexthop_type);
    modify_field(p4i_i2e.nexthop_id, nexthop_id);
    modify_field(p4i_i2e.session_id, -1);
    modify_field(p4i_i2e.copp_policer_id, copp_policer_id);

    modify_field(capri_intrinsic.drop, 0);
    modify_field(control_metadata.p4i_drop_reason, 0);
}

action nacl_flow_miss(nexthop_type, nexthop_id, copp_policer_id) {
    modify_field(p4i_i2e.nexthop_type, nexthop_type);
    modify_field(p4i_i2e.nexthop_id, nexthop_id);
    modify_field(p4i_i2e.session_id, -1);
    modify_field(p4i_i2e.copp_policer_id, copp_policer_id);
    modify_field(control_metadata.flow_miss_redirect, TRUE);
}

@pragma stage 4
table nacl {
    reads {
        vnic_metadata.vnic_id               : ternary;
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
    }
    actions {
        nacl_permit;
        nacl_redirect;
        nacl_flow_miss;
        nacl_drop;
    }
    size : NACL_TABLE_SIZE;
}

control nacl {
    apply(nacl);
}
