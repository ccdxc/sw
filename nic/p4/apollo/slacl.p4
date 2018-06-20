#define SLACL_IP_TABLE_SIZE                                 65536
#define SLACL_PORT_TABLE_SIZE                               65536
#define SLACL_PROTO_TABLE_SIZE                              256
#define SLACL_CLASS0_TABLE_SIZE                             16384
#define SLACL_CLASS1_TABLE_SIZE                             65536
#define SLACL_CLASS2_TABLE_SIZE                             65536
#define SLACL_STATS_TABLE_SIZE                              1048576

#define SLACL_IP_TABLE_OFFSET                               (64*1024)
#define SLACL_DPORT_TABLE_OFFSET                            (128*1024)
#define SLACL_SPORT_TABLE_OFFSET                            (192*1024)
#define SLACL_PROTO_TABLE_OFFSET                            (256*1024)
#define SLACL_CLASS1_TABLE_OFFSET                           (257*1024)
#define SLACL_CLASS2_TABLE_OFFSET                           (273*1024)
#define SLACL_CLASS3_TABLE_OFFSET                           (337*1024)

action slacl_init(base_addr) {
    modify_field(slacl_metadata.base_addr, base_addr);
    modify_field(slacl_metadata.addr1, base_addr);
    modify_field(slacl_metadata.addr2, base_addr + SLACL_IP_TABLE_OFFSET);
}

action slacl_ip_15_00(class_id) {
    modify_field(slacl_metadata.addr1, (slacl_metadata.base_addr +
                                        SLACL_DPORT_TABLE_OFFSET +
                                        key_metadata.dport));
    modify_field(scratch_metadata.class_id, class_id);
    modify_field(slacl_metadata.class_id0,
                 scratch_metadata.class_id << 8, 0x3FC0);
}

@pragma stage 0
@pragma hbm_table
@pragma raw_index_table
table slacl_ip_15_00 {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_ip_15_00;
    }
    size : SLACL_IP_TABLE_SIZE;
}

action slacl_ip_31_16(class_id) {
    modify_field(slacl_metadata.addr2, (slacl_metadata.base_addr +
                                        SLACL_SPORT_TABLE_OFFSET +
                                        key_metadata.sport));
    modify_field(scratch_metadata.class_id, class_id);
    modify_field(slacl_metadata.class_id1,
                 scratch_metadata.class_id << 8, 0xFF00);
}

@pragma stage 0
@pragma hbm_table
@pragma raw_index_table
table slacl_ip_31_16 {
    reads {
        slacl_metadata.addr2    : exact;
    }
    actions {
        slacl_ip_31_16;
    }
    size : SLACL_IP_TABLE_SIZE;
}

action slacl_dport(class_id) {
    modify_field(scratch_metadata.class_id, class_id);
    modify_field(slacl_metadata.addr1, (slacl_metadata.base_addr +
                                        SLACL_CLASS1_TABLE_OFFSET +
                                        slacl_metadata.class_id0 +
                                        scratch_metadata.class_id));
}

@pragma stage 1
@pragma hbm_table
@pragma raw_index_table
table slacl_dport {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_dport;
    }
    size : SLACL_PORT_TABLE_SIZE;
}

action slacl_sport(class_id) {
    modify_field(slacl_metadata.addr2, (slacl_metadata.base_addr +
                                        SLACL_PROTO_TABLE_OFFSET +
                                        key_metadata.proto));
    modify_field(scratch_metadata.class_id, class_id);
    modify_field(slacl_metadata.class_id1,
                 scratch_metadata.class_id << 4, 0x0F0);
}

@pragma stage 1
@pragma hbm_table
@pragma raw_index_table
table slacl_sport {
    reads {
        slacl_metadata.addr2    : exact;
    }
    actions {
        slacl_sport;
    }
    size : SLACL_PORT_TABLE_SIZE;
}
action slacl_proto(class_id) {
    modify_field(scratch_metadata.class_id, class_id);
    modify_field(slacl_metadata.addr2, (slacl_metadata.base_addr +
                                        SLACL_CLASS2_TABLE_OFFSET +
                                        slacl_metadata.class_id1 +
                                        scratch_metadata.class_id));
}

@pragma stage 2
@pragma hbm_table
@pragma raw_index_table
table slacl_proto {
    reads {
        slacl_metadata.addr2    : exact;
    }
    actions {
        slacl_proto;
    }
    size : SLACL_PROTO_TABLE_SIZE;
}

action slacl_class0(class_id) {
    modify_field(scratch_metadata.class_id, class_id);
    modify_field(slacl_metadata.addr1, (slacl_metadata.base_addr +
                                        SLACL_CLASS2_TABLE_OFFSET));
    modify_field(slacl_metadata.addr1, scratch_metadata.class_id << 8, 0xFF00);
}

@pragma stage 3
@pragma hbm_table
@pragma raw_index_table
table slacl_class0 {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_class0;
    }
    size : SLACL_CLASS0_TABLE_SIZE;
}

action slacl_class1(class_id) {
    modify_field(scratch_metadata.class_id, class_id);
    modify_field(slacl_metadata.addr1, scratch_metadata.class_id, 0x00FF);
}

@pragma stage 3
@pragma hbm_table
@pragma raw_index_table
table slacl_class1 {
    reads {
        slacl_metadata.addr2    : exact;
    }
    actions {
        slacl_class1;
    }
    size : SLACL_CLASS1_TABLE_SIZE;
}

action slacl_action(rule_id, drop) {
    modify_field(scratch_metadata.rule_id, rule_id);
    modify_field(slacl_metadata.drop, drop);
    if (control_metadata.direction == RX_PACKET) {
        modify_field(slacl_metadata.stats_index, rule_id +
                     (control_metadata.egress_vnic * (4 * 1024)));
    } else {
        modify_field(slacl_metadata.stats_index, rule_id +
                     (control_metadata.ingress_vnic * (4 * 1024)));
    }
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table slacl_class2 {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_action;
    }
    size : SLACL_CLASS2_TABLE_SIZE;
}

action slacl_stats(permit_packets, permit_bytes, deny_packets, deny_bytes) {
    if (slacl_metadata.drop == FALSE) {
        modify_field(scratch_metadata.in_packets, permit_packets);
        modify_field(scratch_metadata.in_bytes, permit_bytes);
    } else {
        modify_field(scratch_metadata.in_packets, deny_packets);
        modify_field(scratch_metadata.in_bytes, deny_bytes);
    }
}

@pragma stage 5
@pragma hbm_table
table slacl_stats {
    reads {
        slacl_metadata.stats_index  : exact;
    }
    actions {
        slacl_stats;
    }
    size : SLACL_STATS_TABLE_SIZE;
}

control slacl {
    apply(slacl_ip_15_00);
    apply(slacl_ip_31_16);
    apply(slacl_sport);
    apply(slacl_dport);
    apply(slacl_proto);
    apply(slacl_class0);
    apply(slacl_class1);
    apply(slacl_class2);
    apply(slacl_stats);
}
