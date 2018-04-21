#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct copp_d d;
struct phv_   p;

%%

execute_copp:
  seq         c1, d.execute_copp_d.entry_valid, TRUE
  seq.c1      c1, d.execute_copp_d.tbkt[39], TRUE
  nop.!c1.e
  phvwr.c1    p.copp_metadata_policer_color, POLICER_COLOR_RED
  phvwr.e     p.capri_intrinsic_drop, TRUE
  phvwr       p.control_metadata_egress_drop_reason[EGRESS_DROP_COPP], 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
execute_copp_error:
  nop.e
  nop
