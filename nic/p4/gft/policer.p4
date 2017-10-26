action execute_ingress_policer() {
}

@pragma stage 5
@pragma policer_table two_color
table ingress_policer {
    reads {
        flow_action_metadata.policer_index : exact;
    }
    actions {
        execute_ingress_policer;
    }
    size : POLICER_TABLE_SIZE;
}

control ingress_policer {
    apply(ingress_policer);
}
