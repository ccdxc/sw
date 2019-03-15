/*****************************************************************************/
/* Security ACL IPv4 LPM lookup                                              */
/*****************************************************************************/

#include "../include/lpm_defines.h"

/* Global definitions */
#define key           sacl_metadata.ip
#define key_field     scratch_metadata.ipv4_addr
#define dat_field     scratch_metadata.class_id10
#define base_addr     sacl_metadata.ipv4_table_addr
#define next_addr     sacl_metadata.ipv4_table_addr_next
#define res_field     scratch_metadata.class_id10

/* Stage 0 */
#define curr_addr     base_addr
#define table_name    sacl_ipv4_keys
#define action_name   search_sacl_ip32b
#define stage_num     1

#include "../include/lpm_32b_keys.h"

#undef curr_addr
#undef table_name
#undef action_name
#undef stage_num

/* Stage 1 */
#define curr_addr     next_addr
#define table_name    sacl_ipv4_keys1
#define action_name   search_sacl_ip32b1
#define stage_num     2

#include "../include/lpm_32b_keys.h"

#undef table_name
#undef action_name
#undef stage_num

/* Stage 2 */
#define table_name    sacl_ipv4_data
#define action_name   search_sacl_ip32b_retrieve
#define res_handler   sacl_ipv4_handler
#define stage_num     3

#include "../include/lpm_32b_data.h"

action res_handler() {
    modify_field(sacl_metadata.p1_table_addr,
        p4_to_rxdma_header.sacl_base_addr + SACL_P1_TABLE_OFFSET +
        (((scratch_metadata.class_id10 |
           (sacl_metadata.sport_class_id << 10)) / 51) << 6));
    modify_field(sacl_metadata.ip_sport_class_id,
        (scratch_metadata.class_id10 | (sacl_metadata.sport_class_id << 10)));
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
