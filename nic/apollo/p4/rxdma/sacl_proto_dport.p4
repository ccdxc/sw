/*****************************************************************************/
/* Security ACL protocol + destination port lookup                           */
/*****************************************************************************/

#include "../include/lpm_defines.h"

/* Global definitions */
#define key           sacl_metadata.proto_dport
#define key_field     scratch_metadata.proto_dport
#define dat_field     scratch_metadata.class_id16
#define base_addr     sacl_metadata.proto_dport_table_addr
#define next_addr     sacl_metadata.proto_dport_table_addr_next
#define res_field     scratch_metadata.class_id16

/* Stage 0 */
#define curr_addr     base_addr
#define table_name    sacl_proto_dport_keys
#define action_name   search_sacl_proto_dport
#define stage_num     3

#include "../include/lpm_32b_keys.h"

#undef curr_addr
#undef table_name
#undef action_name
#undef stage_num

/* Stage 1 */
#define curr_addr     next_addr
#define table_name    sacl_proto_dport_keys1
#define action_name   search_sacl_proto_dport1
#define stage_num     4

#include "../include/lpm_32b_keys.h"

#undef table_name
#undef action_name
#undef stage_num

/* Stage 2 */
#define table_name    sacl_proto_dport_data
#define action_name   search_sacl_proto_dport_retrieve
#define res_handler   sacl_proto_dport_handler
#define stage_num     5

#include "../include/lpm_32b_data.h"

action res_handler() {
    modify_field(sacl_metadata.p2_table_addr,
                 p4_to_rxdma_header.sacl_base_addr + SACL_P2_TABLE_OFFSET +
                 (sacl_metadata.p1_class_id << 6));
}

#undef table_name
#undef action_name
#undef stage_num

#undef key
#undef key_field
#undef dat_field
#undef base_addr
#undef curr_addr
#undef next_addr
#undef res_field
