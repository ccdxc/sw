#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct copp_d d;
struct phv_   p;

%%

execute_copp:
  seq         c1, d.execute_copp_d.entry_valid, TRUE
  seq         c2, d.execute_copp_d.tbkt[39], TRUE
  setcf       c3, [c1&c2]
  nop.!c3.e
  phvwr.c3.e  p.copp_metadata_policer_color, POLICER_COLOR_RED
  phvwr       p.capri_intrinsic_drop, TRUE
