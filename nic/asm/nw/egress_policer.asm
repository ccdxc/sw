#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct egress_policer_d d;
struct phv_             p;

%%

execute_egress_policer:
  seq         c1, d.execute_egress_policer_d.entry_valid, TRUE
  seq.c1      c1, d.execute_egress_policer_d.tbkt[39], TRUE
  nop.!c1.e
  phvwr.c1.e  p.policer_metadata_egress_policer_color, POLICER_COLOR_RED
  phvwr       p.capri_intrinsic_drop, TRUE
