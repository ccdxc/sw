#include "../../p4/include/lpm_defines.h"

#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct txlpm1_5_k          k;
struct txlpm1_5_d          d;

// Define Table Name and Action Names
#define table_name         txlpm1_5
#define action_keys32b     match1_5_32b
#define action_keys64b     match1_5_64b
#define action_data32b     match1_5_32b_retrieve
#define action_data64b     match1_5_64b_retrieve

// Define table field names for the selected key-widths
#define keys32b(a)         d.u.match1_5_32b_d.key ## a
#define keys64b(a)         d.u.match1_5_64b_d.key ## a
#define keys32b_(a)        d.u.match1_5_32b_retrieve_d.key ## a
#define keys64b_(a)        d.u.match1_5_64b_retrieve_d.key ## a
#define data32b(a)         d.u.match1_5_32b_retrieve_d.data ## a
#define data64b(a)         d.u.match1_5_64b_retrieve_d.data ## a

// Define key field names
#define key                k.txdma_control_lpm1_key
#define base_addr          k.{txdma_control_lpm1_base_addr_sbit0_ebit1...\
                              txdma_control_lpm1_base_addr_sbit2_ebit33}
#define curr_addr          k.{txdma_control_lpm1_next_addr_sbit0_ebit1...\
                              txdma_control_lpm1_next_addr_sbit2_ebit33}

// Define PHV field names
#define next_addr          p.txdma_control_lpm1_next_addr

// Define result register and handler function name
#define res_reg            r7
#define result_handler32b  route_result_handler
#define result_handler64b  route_result_handler

%%

#include "../include/lpm.h"
#include "route_result.h"
