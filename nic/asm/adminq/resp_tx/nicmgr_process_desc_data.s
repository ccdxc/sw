
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t1_k.h"

#include "../../asm/adminq/defines.h"

struct phv_ p;
struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_nicmgr_process_desc_data_d d;

%%

.align
nicmgr_process_desc_data:

  bcf             [c2 | c3 | c7], nicmgr_process_desc_data_error
  nop.e

  // Save adminq descriptor data to PHV
  phvwr.f         p.{adminq_comp_desc_status...adminq_comp_desc_rsvd2}, d[511:384]

nicmgr_process_desc_data_error:
  phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
  phvwri.f        p.p4_intr_global_drop, 1
