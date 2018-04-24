#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct rx_policer_k k;
struct rx_policer_d d;
struct phv_         p;

%%

execute_rx_policer:
  phvwr       p.policer_metadata_packet_len, \
              k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                 capri_p4_intrinsic_packet_len_sbit6_ebit13}
  seq         c1, d.execute_rx_policer_d.entry_valid, TRUE
  seq.c1      c1, d.execute_rx_policer_d.tbkt[39], TRUE
  nop.!c1.e
  phvwr.c1    p.policer_metadata_rx_policer_color, POLICER_COLOR_RED
  phvwr.e     p.capri_intrinsic_drop, TRUE
  phvwr       p.control_metadata_egress_drop_reason[EGRESS_DROP_POLICER], 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_policer_error:
  nop.e
  nop
