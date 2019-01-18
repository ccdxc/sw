#include "../include/slacl_defines.h"
#include "slacl_ipv4.p4"
#include "slacl_sport.p4"
#include "slacl_proto_dport.p4"

action slacl_ip_sport_p1(pad, data) {
    modify_field(scratch_metadata.pad2, pad);
    modify_field(scratch_metadata.data510, data);
    modify_field(slacl_metadata.p1_class_id,
                 (data >> ((slacl_metadata.ip_sport_class_id % 51) * 10)));
}

action slacl_p2(data) {
    modify_field(scratch_metadata.data512, data);
    modify_field(p4_to_rxdma_header.slacl_result,
                 (data >> (slacl_metadata.proto_dport_class_id * 2)));
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
         * 256 sport ranges - 512 nodes in LPM tree, outputs 7 bit class-id
         *                    32*16 entries packed in 2 level LPM
         * 512 proto+dport ranges - 2k nodes in LPM tree, outputs 8 bit class-id
         *                          16*16*8 entres packed in 3 level LPM
         * 1k ipv4 - 2k nodes in LPM tree, outputs 10 bit class-id
         *           16*16*8 entries packed in 3 level LPM
         *
         * Phase 1:
         * Combine 7 bit sport-class-id and 10 bit ipv4-class-id, output
         * 10 bit class-id
         *
         * Phase2:
         * Combine 8 bit prot+dport-class-id and 10 bit phase 1 class-id to
         * yield 2 bit result
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
