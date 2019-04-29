action nacl_drop() {
    modify_field(control_metadata.p4i_drop_reason, 1 << P4I_DROP_NACL);
    drop_packet();
}

action nacl_permit() {
}

action nacl_redirect(app_id, oport, lif, qtype, qid, vlan_strip) {
    modify_field(capri_intrinsic.tm_oport, oport);
    modify_field(capri_intrinsic.lif, lif);
    modify_field(capri_rxdma_intrinsic.qtype, qtype);
    modify_field(capri_rxdma_intrinsic.qid, qid);
    modify_field(control_metadata.vlan_strip, vlan_strip);
    modify_field(control_metadata.p4plus_app_id, app_id);

    modify_field(capri_intrinsic.drop, 0);
    modify_field(control_metadata.p4i_drop_reason, 0);
}

@pragma stage 2
table nacl {
    reads {
        control_metadata.direction          : ternary;
        vnic_metadata.local_vnic_tag        : ternary;
        key_metadata.ktype                  : ternary;
        key_metadata.dst                    : ternary;
        key_metadata.src                    : ternary;
        key_metadata.proto                  : ternary;
        key_metadata.sport                  : ternary;
        key_metadata.dport                  : ternary;
        capri_intrinsic.lif                 : ternary;
        ctag_1.valid                        : ternary;
        ctag_1.vid                          : ternary;
        predicate_header.redirect_to_arm    : ternary;
        control_metadata.tunneled_packet    : ternary;
    }
    actions {
        nacl_permit;
        nacl_redirect;
        nacl_drop;
    }
    size : NACL_TABLE_SIZE;
}

control nacl {
    apply(nacl);
}
