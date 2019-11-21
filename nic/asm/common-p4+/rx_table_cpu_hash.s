
#include "ingress.h"
#include "../../p4/common/defines.h"

struct rx_table_cpu_hash_k k;

%%
    .param      cpu_hash_calculate_id
    .param      rawr_toeplitz_hash_calc_id
    
    .align

rx_table_cpu_hash:
    seq     c1, k.toeplitz_key2_data[3:0], P4PLUS_APPTYPE_CPU
    j.c1    cpu_hash_calculate_id
    seq     c1, k.toeplitz_key2_data[3:0], P4PLUS_APPTYPE_RAW_REDIR // delay slot
    j.c1    rawr_toeplitz_hash_calc_id
    nop.!c1.e
    nop

