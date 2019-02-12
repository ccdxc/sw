/*****************************************************************************/
/* Security ACL IPv4 LPM lookup                                              */
/*****************************************************************************/

#define LPM_KEY_SIZE        4
#define LPM_DATA_SIZE       2
#define LPM_S0_ENTRY_PAD    4
#define LPM_S1_ENTRY_PAD    4
#define LPM_S2_ENTRY_PAD    20
#define s0_stage            1
#define s1_stage            2
#define s2_stage            3
#define s0_name             sacl_ipv4_lpm_s0
#define s1_name             sacl_ipv4_lpm_s1
#define s2_name             sacl_ipv4_lpm_s2
#define s2_name_ext         sacl_ipv4_lpm_s2_ext
#define key_field           scratch_metadata.ipv4_addr
#define res_field           scratch_metadata.class_id16
#define key                 sacl_metadata.ip
#define next_addr           sacl_metadata.ipv4_table_addr
#define s2_offset           sacl_metadata.ipv4_lpm_s2_offset
#define base_addr           sacl_metadata.ipv4_table_addr
#define result              scratch_metadata.class_id10

action sacl_ipv4_lpm_s2_ext() {
    modify_field(sacl_metadata.p1_table_addr,
        p4_to_rxdma_header.sacl_base_addr + SACL_P1_TABLE_OFFSET +
        (((scratch_metadata.class_id10 |
           (sacl_metadata.sport_class_id << 10)) / 51) << 6));
    modify_field(sacl_metadata.ip_sport_class_id,
        (scratch_metadata.class_id10 | (sacl_metadata.sport_class_id << 10)));
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
