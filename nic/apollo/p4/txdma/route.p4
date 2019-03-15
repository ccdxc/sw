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

/* Stage 0 */
#define curr_addr     base_addr
#define table_name    route_ipv4_keys
#define action_name   search_routes32b
#define stage_num     2

#include "../include/lpm_32b_keys.h"

#undef curr_addr
#undef table_name
#undef action_name
#undef stage_num

/* Stage 1 */
#define curr_addr     next_addr
#define table_name    route_ipv4_keys1
#define action_name   search_routes32b1
#define stage_num     3

#include "../include/lpm_32b_keys.h"

#undef table_name
#undef action_name
#undef stage_num

/* Stage 2 */
#define table_name    route_ipv4_data
#define action_name   search_routes32b_retrieve
#define res_handler   route_ipv4_handler
#define stage_num     4

#include "../include/lpm_32b_data.h"

action res_handler() {
}

#undef table_name
#undef action_name
#undef stage_num

control route_ipv4_lookup {
    apply(route_ipv4_keys);
    apply(route_ipv4_keys1);
    apply(route_ipv4_data);
}

#undef key
#undef key_field
#undef dat_field
#undef base_addr
#undef curr_addr
#undef next_addr
#undef res_field
