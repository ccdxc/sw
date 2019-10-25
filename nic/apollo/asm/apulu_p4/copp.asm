#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"

struct copp_d d;
struct phv_   p;

%%

copp:
  seq         c1, d.copp_d.entry_valid, TRUE
  seq.c1      c1, d.copp_d.tbkt[39], TRUE
  nop.!c1.e
  phvwr.c1.e  p.control_metadata_p4e_drop_reason[P4E_DROP_COPP], 1
  phvwr.f     p.capri_intrinsic_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
copp_error:
  nop.e
  nop
