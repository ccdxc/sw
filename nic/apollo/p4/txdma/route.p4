/*****************************************************************************/
/* IPv4 Route LPM lookup                                                     */
/*****************************************************************************/

#include "../include/lpm_defines.h"

/* Global definitions */
#define key           p4_to_txdma_header.lpm_dst
#define key_field     scratch_metadata.field32
#define dat_field     scratch_metadata.field16
#define base_addr     p4_to_txdma_header.lpm_addr
#define next_addr     txdma_control.lpm_addr
#define res_field     txdma_to_p4e_header.nexthop_index
#define res_handler   route_ipv4_handler

/* Stage 0 */
#define curr_addr     base_addr
#define table_name    route_ipv4
#define action_keys   search_routes32b
#define action_data   search_routes32b_retrieve
#define stage_num     2

#include "../include/lpm_32b.h"

#undef curr_addr
#undef table_name
#undef action_keys
#undef action_data
#undef stage_num

/* Stage 1 */
#define curr_addr     next_addr
#define table_name    route_ipv4_1
#define action_keys   search_routes32b1
#define action_data   search_routes32b1_retrieve
#define stage_num     3

#include "../include/lpm_32b.h"

#undef table_name
#undef action_keys
#undef action_data
#undef stage_num

/* Stage 2 */
#define table_name    route_ipv4_2
#define action_keys   search_routes32b2
#define action_data   search_routes32b2_retrieve
#define stage_num     4

#include "../include/lpm_32b.h"

action res_handler() {
}

#undef table_name
#undef action_keys
#undef action_data
#undef stage_num

control route_ipv4_lookup {
    apply(route_ipv4);
    apply(route_ipv4_1);
    apply(route_ipv4_2);
}

#undef key
#undef key_field
#undef dat_field
#undef base_addr
#undef curr_addr
#undef next_addr
#undef res_field
#undef res_handler
