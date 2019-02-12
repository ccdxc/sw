/*****************************************************************************/
/* Security ACL protocol + destination port lookup                           */
/*****************************************************************************/

#define LPM_KEY_SIZE        3
#define LPM_DATA_SIZE       2
#define LPM_S0_ENTRY_PAD    19
#define LPM_S1_ENTRY_PAD    19
#define LPM_S2_ENTRY_PAD    27
#define s0_stage            3
#define s1_stage            4
#define s2_stage            5
#define s0_name             sacl_proto_dport_lpm_s0
#define s1_name             sacl_proto_dport_lpm_s1
#define s2_name             sacl_proto_dport_lpm_s2
#define s2_name_ext         sacl_proto_dport_lpm_s2_ext
#define key_field           scratch_metadata.proto_dport
#define res_field           scratch_metadata.class_id16
#define key                 sacl_metadata.proto_dport
#define next_addr           sacl_metadata.proto_dport_table_addr
#define s2_offset           sacl_metadata.proto_dport_lpm_s2_offset
#define base_addr           sacl_metadata.proto_dport_table_addr
#define result              sacl_metadata.proto_dport_class_id

action sacl_proto_dport_lpm_s2_ext() {
    modify_field(sacl_metadata.p2_table_addr,
                 p4_to_rxdma_header.sacl_base_addr + SACL_P2_TABLE_OFFSET +
                 (sacl_metadata.p1_class_id << 8));
}

#include "../include/lpm.h"

#undef LPM_KEY_SIZE
#undef LPM_DATA_SIZE
#undef LPM_S0_ENTRY_PAD
#undef LPM_S1_ENTRY_PAD
#undef LPM_S2_ENTRY_PAD
#undef s0_stage
#undef s1_stage
#undef s2_stage
#undef s0_name
#undef s1_name
#undef s2_name
#undef s2_name_ext
#undef key_field
#undef res_field
#undef key
#undef next_addr
#undef s2_offset
#undef base_addr
#undef result
