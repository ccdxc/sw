@pragma stage 5
table policer {
    reads {
        policer_metadata.resource_group1    : exact;
    }
    actions {
        execute_policer;
    }
    size : POLICER_TABLE_SIZE;
}

control policer {
    apply(policer);
}
