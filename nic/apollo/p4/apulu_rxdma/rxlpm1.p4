/*****************************************************************************/
/* General Purpose LPM lookup - 1                                            */
/*****************************************************************************/

/**    Global Definitions    **/
/* Select key-widths and define table field names for them */
#define key_field16b       scratch_metadata.field16
#define dat_field16b       scratch_metadata.field16
#define key_field32b       scratch_metadata.field32
#define dat_field32b       scratch_metadata.field16
#define key_field128b      scratch_metadata.field128
#define dat_field128b      scratch_metadata.field16

/* Define key fields */
#define key                lpm_metadata.lpm1_key
#define base_addr          lpm_metadata.lpm1_base_addr

/* Define PHV fields */
#define next_addr          lpm_metadata.lpm1_next_addr

// Define result field and handler function name
#define lpm_result16b      scratch_metadata.field10
#define lpm_result32b      scratch_metadata.field10
#define lpm_result128b     scratch_metadata.field10
#define result_handler16b  rxlpm1_res_handler
#define result_handler32b  rxlpm1_res_handler
#define result_handler128b rxlpm1_res_handler

/**    Per Stage Definitions    **/
/* Stage 0 */
#define stage_num          1
#define table_name         rxlpm1_0
#define action_keys16b     match1_0_16b
#define action_keys32b     match1_0_32b
#define action_keys128b    match1_0_128b
#define curr_addr          base_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys16b
#undef action_keys32b
#undef action_keys128b
#undef curr_addr

/* Stage 1 */
#define stage_num          2
#define table_name         rxlpm1_1
#define action_keys16b     match1_1_16b
#define action_keys32b     match1_1_32b
#define action_keys128b    match1_1_128b
#define action_data16b     match1_1_16b_retrieve
#define curr_addr          next_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys16b
#undef action_keys32b
#undef action_keys128b
#undef action_data16b

/* Stage 2 */
#define stage_num          3
#define table_name         rxlpm1_2
#define action_keys16b     match1_2_16b
#define action_keys32b     match1_2_32b
#define action_keys128b    match1_2_128b
#define action_data16b     match1_2_16b_retrieve
#define action_data32b     match1_2_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys16b
#undef action_keys32b
#undef action_keys128b
#undef action_data16b
#undef action_data32b

/* Stage 3 */
#define stage_num          4
#define table_name         rxlpm1_3
#define action_keys16b     match1_3_16b
#define action_keys32b     match1_3_32b
#define action_keys128b    match1_3_128b
#define action_data16b     match1_3_16b_retrieve
#define action_data32b     match1_3_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys16b
#undef action_keys32b
#undef action_keys128b
#undef action_data16b
#undef action_data32b

/* Stage 4 */
#define stage_num          5
#define table_name         rxlpm1_4
#define action_keys16b     match1_4_16b
#define action_keys32b     match1_4_32b
#define action_keys128b    match1_4_128b
#define action_data16b     match1_4_16b_retrieve
#define action_data32b     match1_4_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys16b
#undef action_keys32b
#undef action_keys128b
#undef action_data16b
#undef action_data32b

/* Stage 5 */
#define stage_num          6
#define table_name         rxlpm1
#define action_keys16b     match1_16b
#define action_keys32b     match1_32b
#define action_keys128b    match1_128b
#define action_data16b     match1_16b_retrieve
#define action_data32b     match1_32b_retrieve
#define action_data128b    match1_128b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys16b
#undef action_keys32b
#undef action_keys128b
#undef action_data16b
#undef action_data32b
#undef action_data128b

#undef key_field16b
#undef dat_field16b
#undef key_field32b
#undef dat_field32b
#undef key_field128b
#undef dat_field128b
#undef key
#undef base_addr
#undef next_addr
#undef curr_addr
#undef lpm_result16b
#undef lpm_result32b
#undef lpm_result128b
#undef result_handler16b
#undef result_handler32b
#undef result_handler128b

control rxlpm1 {
    apply(rxlpm1_0);
    apply(rxlpm1_1);
    apply(rxlpm1_2);
    apply(rxlpm1_3);
    apply(rxlpm1_4);
    apply(rxlpm1);
}
