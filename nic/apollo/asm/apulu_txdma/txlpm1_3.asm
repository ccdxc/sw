#include "../../p4/include/lpm_defines.h"

#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct txlpm1_3_k          k;
struct txlpm1_3_d          d;

// Define Table Name and Action Names
#define table_name         txlpm1_3
#define action_keys32b     match1_3_32b
#define action_keys64b     match1_3_64b

// Define table field names for the selected key-widths
#define keys32b(a)         d.u.match1_3_32b_d.key ## a
#define keys64b(a)         d.u.match1_3_64b_d.key ## a

// Define key field names
#define key                k.txdma_control_lpm1_key
#define base_addr          k.txdma_control_lpm1_base_addr
#define curr_addr          k.txdma_control_lpm1_next_addr

// Define PHV field names
#define next_addr          p.txdma_control_lpm1_next_addr

%%

#include "../include/lpm.h"
