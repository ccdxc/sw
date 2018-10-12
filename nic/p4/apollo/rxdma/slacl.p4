#include "../include/slacl_defines.h"
#include "slacl_ipv4_lpm.p4"

action slacl_sport_lpm_s0(data) {
    modify_field(scratch_metadata.data512, data);
    // Form the address for sport s1 lookup
    modify_field(slacl_metadata.sport_table_addr,
        p4_to_rxdma_header.slacl_base_addr + SLACL_SPORT_TABLE_OFFSET);
    // Form the address for ipv4 lookup
    modify_field(slacl_metadata.ipv4_table_addr,
        p4_to_rxdma_header.slacl_base_addr + SLACL_IPV4_TABLE_OFFSET);
    // Form the address for dport+proto lookup
    modify_field(slacl_metadata.proto_dport_table_addr,
        p4_to_rxdma_header.slacl_base_addr + SLACL_PROTO_DPORT_TABLE_OFFSET);
    if (p4_to_rxdma_header.direction == TX_FROM_HOST) {
        modify_field(slacl_metadata.ip, p4_to_rxdma_header.flow_dst);
    } else {
        modify_field(slacl_metadata.ip, p4_to_rxdma_header.flow_src);
    }
}

action slacl_sport_lpm_s1(data) {
    modify_field(scratch_metadata.data512, data);
}

action slacl_proto_dport_lpm_s0(data) {
    modify_field(scratch_metadata.data512, data);
}

action slacl_proto_dport_lpm_s1(data) {
    modify_field(scratch_metadata.data512, data);
}

action slacl_proto_dport_lpm_s2(data) {
    modify_field(scratch_metadata.data512, data);
    modify_field(scratch_metadata.class_id10, slacl_metadata.p1_class_id);
    modify_field(slacl_metadata.proto_dport_class_id, data);
    modify_field(slacl_metadata.p2_table_addr,
                 p4_to_rxdma_header.slacl_base_addr + SLACL_P2_TABLE_OFFSET +
                 slacl_metadata.p1_class_id << 8 +
                 slacl_metadata.proto_dport_class_id);
}

action slacl_ip_sport_p1(data) {
    modify_field(scratch_metadata.data512, data);
}

action slacl_p2(slacl_result) {
    modify_field(p4_to_rxdma_header.slacl_result, slacl_result);
}

@pragma stage 0
@pragma hbm_table
@pragma raw_index_table
table slacl_sport_lpm_s0 {
    reads {
        p4_to_rxdma_header.slacl_base_addr : exact;
    }
    actions {
        slacl_sport_lpm_s0;
    }
}

@pragma stage 1
@pragma hbm_table
@pragma raw_index_table
table slacl_sport_lpm_s1 {
    reads {
        slacl_metadata.sport_table_addr : exact;
    }
    actions {
        slacl_sport_lpm_s1;
    }
}

@pragma stage 3
@pragma hbm_table
@pragma raw_index_table
table slacl_proto_dport_lpm_s0 {
    reads {
        slacl_metadata.proto_dport_table_addr : exact;
    }
    actions {
        slacl_proto_dport_lpm_s0;
    }
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table slacl_proto_dport_lpm_s1 {
    reads {
        slacl_metadata.proto_dport_table_addr : exact;
    }
    actions {
        slacl_proto_dport_lpm_s1;
    }
}

@pragma stage 5
@pragma hbm_table
@pragma raw_index_table
table slacl_proto_dport_lpm_s2 {
    reads {
        slacl_metadata.proto_dport_table_addr : exact;
    }
    actions {
        slacl_proto_dport_lpm_s2;
    }
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table slacl_ip_sport_p1 {
    reads {
        slacl_metadata.p1_table_addr : exact;
    }
    actions {
        slacl_ip_sport_p1;
    }
}

@pragma stage 6
@pragma hbm_table
@pragma raw_index_table
table slacl_p2 {
    reads {
        slacl_metadata.p2_table_addr : exact;
    }
    actions {
        slacl_p2;
    }
}

control slacl {
    if (p4_to_rxdma_header.slacl_bypass == FALSE) {
        /* This assumes we're supporting
         * 128 sport ranges - 256 nodes in LPM tree, outputs 7 bit class-id
         *                    16*16 entries packed in 2 level LPM
         *                    (class-id is adjacent in each node)
         * 128 proto+dport ranges - 512 nodes in LPM tree, outputs 8 bit class-id
         *                          16*16*16 entres packed in 3 level LPM
         *                          (class-id is adjacent in each node)
         * 1k ipv4 - 2k nodes in LPM tree, outputs 10 bit class-id
         *           16*16*8 entries packed in 3 level LPM
         *
         * Phase 1:
         * Combine 7 bit sport-class-id and 10 bit ipv4-class-id, output
         * 10 bit class-id
         *
         * Phase2:
         * Combine 8 bit prot+dport-class-id and 10 bit phase 1 class-id to yield
         * 2 bit result
         */
        apply(slacl_sport_lpm_s0);
        apply(slacl_sport_lpm_s1);
        apply(slacl_ipv4_lpm_s0);
        apply(slacl_ipv4_lpm_s1);
        apply(slacl_ipv4_lpm_s2);
        apply(slacl_proto_dport_lpm_s0);
        apply(slacl_proto_dport_lpm_s1);
        apply(slacl_proto_dport_lpm_s2);
        apply(slacl_ip_sport_p1);
        apply(slacl_p2);
    }
}
