/*****************************************************************************/
/* IPv4 Route LPM lookup                                                     */
/*****************************************************************************/

#include "../include/lpm_defines.h"

/* Global definitions */
#define key            p4_to_txdma_header.lpm_dst
#define key_field32b   scratch_metadata.field32
#define dat_field32b   scratch_metadata.field16
#define key_field64b   scratch_metadata.field64
#define dat_field64b   scratch_metadata.field16
#define base_addr      p4_to_txdma_header.lpm_addr
#define next_addr      txdma_control.lpm_addr
#define res_field      txdma_to_p4e_header.nexthop_index
#define ctrl_field     app_header.table0_valid
#define res_handler    route_res_handler

/* Stage 0 */
#define stage_num      2
#define table_name     route
#define curr_addr      base_addr
#define action_keys32b search_ipv4
#define action_keys64b search_ipv6
#define action_data32b search_ipv4_retrieve
#define action_data64b search_ipv6_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef curr_addr
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

/* Stage 1 */
#define stage_num      3
#define table_name     route1
#define curr_addr      next_addr
#define action_keys32b search1_ipv4
#define action_keys64b search1_ipv6
#define action_data32b search1_ipv4_retrieve
#define action_data64b search1_ipv6_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

/* Stage 2 */
#define stage_num      4
#define table_name     route2
#define action_keys32b search2_ipv4
#define action_keys64b search2_ipv6
#define action_data32b search2_ipv4_retrieve
#define action_data64b search2_ipv6_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

/* Stage 3 */
#define stage_num      5
#define table_name     route3
#define action_keys32b search3_ipv4
#define action_keys64b search3_ipv6
#define action_data32b search3_ipv4_retrieve
#define action_data64b search3_ipv6_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

/* Stage 4 */
#define stage_num      6
#define table_name     route4
#define action_keys32b search4_ipv4
#define action_keys64b search4_ipv6
#define action_data32b search4_ipv4_retrieve
#define action_data64b search4_ipv6_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

/* Stage 5 */
#define stage_num      7
#define table_name     route5
#define action_keys32b search5_ipv4
#define action_keys64b search5_ipv6
#define action_data32b search5_ipv4_retrieve
#define action_data64b search5_ipv6_retrieve

#include "../include/lpm.h"

#undef stage_num
#undef table_name
#undef action_keys32b
#undef action_keys64b
#undef action_data32b
#undef action_data64b

action res_handler() {
}

control route_lookup {
    apply(route);
    apply(route1);
    apply(route2);
    apply(route3);
    apply(route4);
    apply(route5);
}

#undef key
#undef key_field32b
#undef key_field64b
#undef dat_field32b
#undef dat_field64b
#undef base_addr
#undef curr_addr
#undef next_addr
#undef res_field
#undef res_handler
