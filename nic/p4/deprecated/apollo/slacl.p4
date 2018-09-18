#include "include/slacl_defines.h"

action slacl_init(base_addr) {
    modify_field(slacl_metadata.base_addr, base_addr);
    modify_field(slacl_metadata.addr1, base_addr);
    modify_field(slacl_metadata.addr2, base_addr + SLACL_IP_TABLE_OFFSET);
}

action slacl_ip_31_16(class_ids, pad) {
    modify_field(scratch_metadata.class_ids, class_ids);
    modify_field(scratch_metadata.class_pad, pad);
    modify_field(scratch_metadata.class_id10, slacl_metadata.ip_31_16 % 51);
    modify_field(slacl_metadata.class_id0, scratch_metadata.class_id10);

    modify_field(slacl_metadata.addr1, (slacl_metadata.base_addr +
                                        SLACL_SPORT_TABLE_OFFSET +
                                        key_metadata.sport));
}

@pragma stage 0
@pragma hbm_table
@pragma raw_index_table
table slacl_ip_31_16 {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_ip_31_16;
    }
    size : SLACL_IP_TABLE_SIZE;
}

action slacl_ip_15_00(class_ids, pad) {
    modify_field(scratch_metadata.class_ids, class_ids);
    modify_field(scratch_metadata.class_pad, pad);
    modify_field(scratch_metadata.class_id10, slacl_metadata.ip_15_00 % 51);
    modify_field(slacl_metadata.class_id1, scratch_metadata.class_id10);

    modify_field(slacl_metadata.addr2, (slacl_metadata.base_addr +
                                        SLACL_DPORT_TABLE_OFFSET +
                                        key_metadata.dport));
    modify_field(slacl_metadata.addr3, (slacl_metadata.base_addr +
                                        SLACL_PROTO_TABLE_OFFSET +
                                        (key_metadata.proto >> 1)));
}

@pragma stage 0
@pragma hbm_table
@pragma raw_index_table
table slacl_ip_15_00 {
    reads {
        slacl_metadata.addr2    : exact;
    }
    actions {
        slacl_ip_15_00;
    }
    size : SLACL_IP_TABLE_SIZE;
}

action slacl_sport(class_id) {
    modify_field(scratch_metadata.class_id8, class_id);
    modify_field(
        slacl_metadata.addr1,
        (slacl_metadata.base_addr + SLACL_P1_C0C2_TABLE_OFFSET +
         (((slacl_metadata.class_id0 << 8 + scratch_metadata.class_id8) / 51) << 6)));
    modify_field(slacl_metadata.class_id2, class_id);
}

@pragma stage 1
@pragma hbm_table
@pragma raw_index_table
table slacl_sport {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_sport;
    }
    size : SLACL_PORT_TABLE_SIZE;
}

action slacl_dport(class_id) {
    modify_field(scratch_metadata.class_id8, class_id);
    modify_field(
        slacl_metadata.addr2,
        (slacl_metadata.base_addr + SLACL_P1_C1C3_TABLE_OFFSET +
         (((slacl_metadata.class_id1 << 8 + scratch_metadata.class_id8) / 51) << 6)));
    modify_field(slacl_metadata.class_id3, class_id);
}

@pragma stage 1
@pragma hbm_table
@pragma raw_index_table
table slacl_dport {
    reads {
        slacl_metadata.addr2    : exact;
    }
    actions {
        slacl_dport;
    }
    size : SLACL_PORT_TABLE_SIZE;
}

action slacl_proto(class_id) {
    modify_field(scratch_metadata.class_id8, class_id);
    modify_field(slacl_metadata.class_id4, key_metadata.proto % 2);
}

@pragma stage 1
@pragma hbm_table
@pragma raw_index_table
table slacl_proto {
    reads {
        slacl_metadata.addr3    : exact;
    }
    actions {
        slacl_proto;
    }
    size : SLACL_PROTO_TABLE_SIZE;
}

action slacl_p1_c0c2(class_ids, pad) {
    modify_field(scratch_metadata.class_ids, class_ids);
    modify_field(scratch_metadata.class_pad, pad);
    modify_field(scratch_metadata.class_id10,
                 (slacl_metadata.class_id0 << 8 + slacl_metadata.class_id2) % 51);
    modify_field(slacl_metadata.class_id5, scratch_metadata.class_id10);
}

@pragma stage 2
@pragma hbm_table
@pragma raw_index_table
table slacl_p1_c0c2 {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_p1_c0c2;
    }
    size : SLACL_P1_C0C2_TABLE_SIZE;
}

action slacl_p1_c1c3(class_ids, pad) {
    modify_field(scratch_metadata.class_ids, class_ids);
    modify_field(scratch_metadata.class_pad, pad);
    modify_field(scratch_metadata.class_id10,
                 (slacl_metadata.class_id1 << 8 + slacl_metadata.class_id3) % 51);
    modify_field(slacl_metadata.class_id6, scratch_metadata.class_id10);
    modify_field(slacl_metadata.addr1,
                 (slacl_metadata.base_addr + SLACL_P2_C6C4_TABLE_OFFSET +
                  (((slacl_metadata.class_id4 << 4 + scratch_metadata.class_id10) / 51) << 6)));
}

@pragma stage 2
@pragma hbm_table
@pragma raw_index_table
table slacl_p1_c1c3 {
    reads {
        slacl_metadata.addr2    : exact;
    }
    actions {
        slacl_p1_c1c3;
    }
    size : SLACL_P1_C1C3_TABLE_SIZE;
}

action slacl_p2_c6c4(class_ids, pad) {
    modify_field(scratch_metadata.class_ids, class_ids);
    modify_field(scratch_metadata.class_pad, pad);
    modify_field(scratch_metadata.class_id10,
                 (slacl_metadata.class_id6 << 4 + slacl_metadata.class_id4) % 51);
    modify_field(slacl_metadata.class_id7, scratch_metadata.class_id10);
    modify_field(slacl_metadata.addr1,
                 (slacl_metadata.base_addr + SLACL_P3_TABLE_OFFSET +
                  ((slacl_metadata.class_id5 << 10 + scratch_metadata.class_id10) << 1)));
}

@pragma stage 3
@pragma hbm_table
@pragma raw_index_table
table slacl_p2_c6c4 {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_p2_c6c4;
    }
    size : SLACL_P2_C6C4_TABLE_SIZE;
}

action slacl_action(rule_id, drop) {
    modify_field(scratch_metadata.rule_id, rule_id);
    modify_field(slacl_metadata.drop, drop);
    if (control_metadata.direction == RX_PACKET) {
        modify_field(slacl_metadata.stats_index, scratch_metadata.rule_id +
                     (control_metadata.egress_vnic * (4 * 1024)));
    } else {
        modify_field(slacl_metadata.stats_index, scratch_metadata.rule_id +
                     (control_metadata.ingress_vnic * (4 * 1024)));
    }
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table slacl_p3 {
    reads {
        slacl_metadata.addr1    : exact;
    }
    actions {
        slacl_action;
    }
    size : SLACL_P3_TABLE_SIZE;
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
    apply(slacl_p1_c0c2);
    apply(slacl_p1_c1c3);
    apply(slacl_p2_c6c4);
    apply(slacl_p3);
    apply(slacl_stats);
}
