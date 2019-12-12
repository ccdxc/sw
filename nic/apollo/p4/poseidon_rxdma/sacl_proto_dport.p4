/*****************************************************************************/
/* Security ACL IPv4 LPM lookup                                              */
/*****************************************************************************/

#include "../include/lpm_defines.h"

/**    Global Definitions    **/
/* Select key-widths and define table field names for them */
#define key_field32b       scratch_metadata.proto_dport
#define dat_field32b       scratch_metadata.class_id16

/* Define key fields */
#define key                sacl_metadata.proto_dport
#define base_addr          sacl_metadata.proto_dport_table_addr

/* Define PHV fields */
#define next_addr          sacl_metadata.proto_dport_table_addr_next

// Define result field and handler function name
#define lpm_result32b      scratch_metadata.class_id16

/**    Per Stage Definitions    **/
/* Stage 0 */
#define stage_num          3
#define table_name         sacl_proto_dport_keys
#define action_keys32b     match_proto_dport
#define curr_addr          base_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef curr_addr

/* Stage 1 */
#define stage_num          4
#define table_name         sacl_proto_dport_keys1
#define action_keys32b     match1_proto_dport
#define curr_addr          next_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b

/* Stage 2 */
#define stage_num          5
#define table_name         sacl_proto_dport_data
#define action_data32b     match_proto_dport_retrieve
#define result_handler32b  sacl_proto_dport_handler

#include "../include/lpm.h"

action sacl_proto_dport_handler() {
    modify_field(sacl_metadata.p2_table_addr,
                 p4_to_rxdma_header.sacl_base_addr + SACL_P2_TABLE_OFFSET +
                 (sacl_metadata.p1_class_id << 6));
}

#undef stage_num
#undef table_name
#undef action_data32b
#undef result_handler32b

#undef key_field32b
#undef dat_field32b
#undef key
#undef base_addr
#undef next_addr
#undef curr_addr
#undef lpm_result32b
