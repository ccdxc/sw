/*****************************************************************************/
/* Stateless ACL IPv4 LPM lookup                                             */
/*****************************************************************************/

#define LPM_KEY_SIZE        4
#define LPM_DATA_SIZE       2
#define s0_stage            1
#define s1_stage            2
#define s2_stage            3
#define s0_name             slacl_ipv4_lpm_s0
#define s1_name             slacl_ipv4_lpm_s1
#define s2_name             slacl_ipv4_lpm_s2
#define s2_name_ext         slacl_ipv4_lpm_s2_ext
#define key_field           scratch_metadata.ipv4_addr
#define res_field           scratch_metadata.class_id16
#define key                 slacl_metadata.ip
#define next_addr           slacl_metadata.ipv4_table_addr
#define s2_offset           slacl_metadata.ipv4_lpm_s2_offset
#define base_addr           slacl_metadata.ipv4_table_addr
#define result              slacl_metadata.ip_class_id

action slacl_ipv4_lpm_s2_ext() {
    modify_field(slacl_metadata.p1_table_addr,
                 p4_to_rxdma_header.slacl_base_addr +
                 SLACL_P1_TABLE_OFFSET +
                 slacl_metadata.sport_class_id << 10);
}

#include "../include/lpm.h"

#undef LPM_KEY_SIZE
#undef LPM_DATA_SIZE
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
