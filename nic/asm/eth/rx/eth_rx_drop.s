
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_common_p4plus_stage0_app_header_table_k.h"

#include "defines.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_k k;
struct common_p4plus_stage0_app_header_table_eth_rx_app_header_d d;

%%

.align
eth_rx_drop:
  phvwr.e           p.p4_intr_global_drop, 1        // Drop this PHV
  nop
