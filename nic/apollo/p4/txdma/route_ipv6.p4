/*****************************************************************************/
/* IPv6 Route LPM lookup                                                          */
/*****************************************************************************/

/* Global definitions */
#define LPM_TBL_SIZE  (64)
#define key           p4_to_txdma_header.lpm_dst
#define key_field     scratch_metadata.field64
#define base_addr     p4_to_txdma_header.lpm_addr
#define next_addr     txdma_control.lpm_addr
#define res_field     txdma_to_p4e_header.nexthop_index

/* Stage 0 */
#define curr_addr     base_addr
#define table_name    route_ipv6_keys
#define action_name   search
#define stage_num     2

#include "../include/lpm_64b_keys.h"

#undef curr_addr
#undef table_name
#undef action_name
#undef stage_num

/* Stage 1 */
#define curr_addr     next_addr
#define table_name    route_ipv6_keys1
#define action_name   search1
#define stage_num     3

#include "../include/lpm_64b_keys.h"

#undef table_name
#undef action_name
#undef stage_num

/* Stage 2 */
#define table_name    route_ipv6_keys2
#define action_name   search2
#define stage_num     4

#include "../include/lpm_64b_keys.h"

#undef table_name
#undef action_name
#undef stage_num

/* Stage 3 */
#define table_name        route_ipv6_data
#define action_name       search_retrieve
#define action_name_ext   search_retrieve_ext
#define stage_num         5

#include "../include/lpm_64b_data.h"

#undef table_name
#undef action_name
#undef action_name_ext
#undef stage_num

control route_ipv6_lookup {
    apply(route_ipv6_keys);
    apply(route_ipv6_keys1);
    apply(route_ipv6_keys2);
    apply(route_ipv6_data);
}

#undef key
#undef key_field
#undef base_addr
#undef curr_addr
#undef next_addr
#undef result
