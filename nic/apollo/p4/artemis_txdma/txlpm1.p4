/*****************************************************************************/
/* General Purpose LPM lookup - 1                                            */
/*****************************************************************************/

#include "../include/lpm_defines.h"

/**    Global Definitions    **/
/* Select key-widths and define table field names for them */
#define key_field32b       scratch_metadata.field32
#define dat_field32b       scratch_metadata.field16
#define key_field64b       scratch_metadata.field64
#define dat_field64b       scratch_metadata.field16

/* Define key fields */
#define key                txdma_control.lpm1_key
#define base_addr          txdma_control.lpm1_base_addr

/* Define PHV fields */
#define next_addr          txdma_control.lpm1_next_addr

// Define result field and handler function names
#define lpm_result32b      scratch_metadata.field16
#define lpm_result64b      scratch_metadata.field16
#define result_handler32b  route_res_handler
#define result_handler64b  route_res_handler

/**    Per Stage Definitions    **/
/* Stage 0 */
#define stage_num          2
#define table_name         txlpm1_2
#define action_keys32b     match1_2_32b
#define action_keys64b     match1_2_64b
#define curr_addr          base_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef curr_addr

/* Stage 1 */
#define stage_num          3
#define table_name         txlpm1_3
#define action_keys32b     match1_3_32b
#define action_keys64b     match1_3_64b
#define curr_addr          next_addr

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b

/* Stage 2 */
#define stage_num          4
#define table_name         txlpm1_4
#define action_keys32b     match1_4_32b
#define action_keys64b     match1_4_64b
#define action_data32b     match1_4_32b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b

/* Stage 3 */
#define stage_num          5
#define table_name         txlpm1_5
#define action_keys32b     match1_5_32b
#define action_keys64b     match1_5_64b
#define action_data32b     match1_5_32b_retrieve
#define action_data64b     match1_5_64b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

/* Stage 4 */
#define stage_num          6
#define table_name         txlpm1_6
#define action_keys32b     match1_6_32b
#define action_keys64b     match1_6_64b
#define action_data32b     match1_6_32b_retrieve
#define action_data64b     match1_6_64b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

/* Stage 5 */
#define stage_num          7
#define table_name         txlpm1
#define action_keys32b     match1_32b
#define action_keys64b     match1_64b
#define action_data32b     match1_32b_retrieve
#define action_data64b     match1_64b_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

#undef key_field32b
#undef dat_field32b
#undef key_field64b
#undef dat_field64b
#undef key
#undef base_addr
#undef next_addr
#undef curr_addr
#undef lpm_result32b
#undef lpm_result64b
#undef result_handler32b
#undef result_handler64b
