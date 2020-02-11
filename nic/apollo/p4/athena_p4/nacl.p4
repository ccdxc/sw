action nacl_drop() {
    modify_field(control_metadata.p4i_drop_reason, 1 << P4I_DROP_NACL);
    drop_packet();
}

action nacl_permit() {
}

action nacl_redirect(redir_type, app_id, oport, lif, qtype, qid) {
    modify_field(control_metadata.redir_type, redir_type);
    modify_field(control_metadata.redir_oport, oport);
    modify_field(control_metadata.redir_lif, lif);
    modify_field(control_metadata.redir_qtype, qtype);
    modify_field(control_metadata.redir_qid, qid);
    modify_field(control_metadata.redir_app_id, app_id);

    modify_field(capri_intrinsic.drop, 0);
    modify_field(control_metadata.p4i_drop_reason, 0);
}

@pragma stage 5
table nacl {
    reads {
        control_metadata.direction          : ternary;
        control_metadata.flow_miss          : ternary;
        key_metadata.ktype                  : ternary;
        key_metadata.dst                    : ternary;
        key_metadata.src                    : ternary;
        key_metadata.proto                  : ternary;
        key_metadata.sport                  : ternary;
        key_metadata.dport                  : ternary;
        ethernet_1.dstAddr                  : ternary;
        capri_intrinsic.lif                 : ternary;
        ctag_1.valid                        : ternary;
        ctag_1.vid                          : ternary;
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
