/*****************************************************************************/
/* Security ACL IPv4 LPM lookup                                              */
/*****************************************************************************/

#include "../include/lpm_defines.h"

/**    Global Definitions    **/
/* Select key-widths and define table field names for them */
#define key_field32b       scratch_metadata.field32
#define dat_field32b       scratch_metadata.class_id16
#define key_field128b      scratch_metadata.field128
#define dat_field128b      scratch_metadata.class_id16

/* Define key fields */
#define key                sacl_metadata.ip
#define base_addr          sacl_metadata.ip_table_addr

/* Define PHV fields */
#define next_addr          sacl_metadata.ip_table_addr_next

// Define result field and handler function name
#define lpm_result32b      scratch_metadata.class_id10
#define lpm_result128b     scratch_metadata.class_id10
#define result_handler32b  sacl_ip_res_handler
#define result_handler128b sacl_ip_res_handler

/**    Per Stage Definitions    **/
/* Stage 0 */
#define stage_num          1
#define table_name         sacl_ip1
#define action_keys32b     match1_ipv4
#define action_data32b     match1_ipv4_retrieve
#define action_keys128b    match1_ipv6
#define curr_addr          base_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_data32b
#undef action_keys128b
#undef curr_addr

/* Stage 1 */
#define stage_num          2
#define table_name         sacl_ip2
#define action_keys32b     match2_ipv4
#define action_data32b     match2_ipv4_retrieve
#define action_keys128b    match2_ipv6
#define curr_addr          next_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_data32b
#undef action_keys128b

/* Stage 2 */
#define stage_num          3
#define table_name         sacl_ip
#define action_keys32b     match_ipv4
#define action_data32b     match_ipv4_retrieve
#define action_keys128b    match_ipv6
#define action_data128b    match_ipv6_retrieve

#include "../include/lpm.h"

action sacl_ip_res_handler() {
    modify_field(sacl_metadata.p1_table_addr,
        p4_to_rxdma_header.sacl_base_addr + SACL_P1_TABLE_OFFSET +
        (((scratch_metadata.class_id10 |
           (sacl_metadata.sport_class_id << 10)) / 51) << 6));
    modify_field(sacl_metadata.ip_sport_class_id,
        (scratch_metadata.class_id10 |
         (sacl_metadata.sport_class_id << 10)));
}

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_data32b
#undef action_keys128b
#undef action_data128b

#undef key_field32b
#undef dat_field32b
#undef key
#undef base_addr
#undef next_addr
#undef curr_addr
#undef lpm_result32b
#undef lpm_result128b
#undef result_handler32b
#undef result_handler128b
