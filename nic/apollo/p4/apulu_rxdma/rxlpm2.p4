/*****************************************************************************/
/* General Purpose LPM lookup - 2                                            */
/*****************************************************************************/

/**    Global Definitions    **/
/* Select key-widths and define table field names for them */
#define key_field32b       scratch_metadata.field32
#define dat_field32b       scratch_metadata.field16
#define key_field128b      scratch_metadata.field128
#define dat_field128b      scratch_metadata.field16

/* Define key fields */
#define key                lpm_metadata.lpm2_key
#define base_addr          lpm_metadata.lpm2_base_addr

/* Define PHV fields */
#define next_addr          lpm_metadata.lpm2_next_addr

// Define result field and handler function name
#define lpm_result32b      scratch_metadata.field10
#define lpm_result128b     scratch_metadata.field10
#define result_handler32b  rxlpm2_res_handler
#define result_handler128b rxlpm2_res_handler

/**    Per Stage Definitions    **/
/* Stage 0 */
#define stage_num          1
#define table_name         rxlpm2_0
#define action_keys32b     match2_0_32b
#define action_keys128b    match2_0_128b
#define curr_addr          base_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef curr_addr

/* Stage 1 */
#define stage_num          2
#define table_name         rxlpm2_1
#define action_keys32b     match2_1_32b
#define action_keys128b    match2_1_128b
#define curr_addr          next_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b

/* Stage 2 */
#define stage_num          3
#define table_name         rxlpm2_2
#define action_keys32b     match2_2_32b
#define action_keys128b    match2_2_128b
#define action_data32b     match2_2_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b

/* Stage 3 */
#define stage_num          4
#define table_name         rxlpm2_3
#define action_keys32b     match2_3_32b
#define action_keys128b    match2_3_128b
#define action_data32b     match2_3_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b

/* Stage 4 */
#define stage_num          5
#define table_name         rxlpm2_4
#define action_keys32b     match2_4_32b
#define action_keys128b    match2_4_128b
#define action_data32b     match2_4_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b

/* Stage 5 */
#define stage_num          6
#define table_name         rxlpm2
#define action_keys32b     match2_32b
#define action_keys128b    match2_128b
#define action_data32b     match2_32b_retrieve
#define action_data128b    match2_128b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b
#undef action_data128b

#undef key_field32b
#undef dat_field32b
#undef key_field128b
#undef dat_field128b
#undef key
#undef base_addr
#undef next_addr
#undef curr_addr
#undef lpm_result32b
#undef lpm_result128b
#undef result_handler32b
#undef result_handler128b

control rxlpm2 {
    apply(rxlpm2_0);
    apply(rxlpm2_1);
    apply(rxlpm2_2);
    apply(rxlpm2_3);
    apply(rxlpm2_4);
    apply(rxlpm2);
}
