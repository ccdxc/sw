action nacl_drop() {
    drop_packet();
}

action nacl_permit() {
}

@pragma stage 2
table nacl {
    reads {
        key_metadata.dst : ternary;
        key_metadata.src : ternary;
        key_metadata.sport : ternary;
        key_metadata.dport : ternary;
    }
    actions {
        nacl_permit;
        nacl_drop;
    }
    size : NACL_TABLE_SIZE;
}

control nacl {
    apply(nacl);
}
