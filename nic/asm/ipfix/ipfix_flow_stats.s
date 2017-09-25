#include "ipfix/asm_out/INGRESS_p.h"
#include "iris/asm_out/ingress.h"

struct flow_stats_d d;
struct phv_         p;

%%

ipfix_flow_stats:
    phvwr       p.app_header_table0_valid, 0
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
