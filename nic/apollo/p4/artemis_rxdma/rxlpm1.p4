/*****************************************************************************/
/* General Purpose LPM lookup - 1                                            */
/*****************************************************************************/

#include "../include/lpm_defines.h"

/**    Global Definitions    **/
/* Select key-widths and define table field names for them */
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
#define lpm_result32b      scratch_metadata.field10
#define lpm_result128b     scratch_metadata.field10
#define result_handler32b  rxlpm1_res_handler
#define result_handler128b rxlpm1_res_handler

/**    Per Stage Definitions    **/
/* Stage 0 */
#define stage_num          0
#define table_name         rxlpm1_0
#define action_keys32b     match1_0_32b
#define action_keys128b    match1_0_128b
#define curr_addr          base_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef curr_addr

/* Stage 1 */
#define stage_num          1
#define table_name         rxlpm1_1
#define action_keys32b     match1_1_32b
#define action_keys128b    match1_1_128b
#define curr_addr          next_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b

/* Stage 2 */
#define stage_num          2
#define table_name         rxlpm1_2
#define action_keys32b     match1_2_32b
#define action_keys128b    match1_2_128b
#define action_data32b     match1_2_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b

/* Stage 3 */
#define stage_num          3
#define table_name         rxlpm1_3
#define action_keys32b     match1_3_32b
#define action_keys128b    match1_3_128b
#define action_data32b     match1_3_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b

/* Stage 4 */
#define stage_num          4
#define table_name         rxlpm1_4
#define action_keys32b     match1_4_32b
#define action_keys128b    match1_4_128b
#define action_data32b     match1_4_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b

/* Stage 5 */
#define stage_num          5
#define table_name         rxlpm1_5
#define action_keys32b     match1_5_32b
#define action_keys128b    match1_5_128b
#define action_data32b     match1_5_32b_retrieve
#define action_data128b    match1_5_128b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b
#undef action_data128b

/* Stage 6 */
#define stage_num          6
#define table_name         rxlpm1_6
#define action_keys32b     match1_6_32b
#define action_keys128b    match1_6_128b
#define action_data32b     match1_6_32b_retrieve
#define action_data128b    match1_6_128b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys128b
#undef action_data32b
#undef action_data128b

/* Stage 7 */
#define stage_num          7
#define table_name         rxlpm1
#define action_keys32b     match1_32b
#define action_keys128b    match1_128b
#define action_data32b     match1_32b_retrieve
#define action_data128b    match1_128b_retrieve

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
