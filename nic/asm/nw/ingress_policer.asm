#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ingress_policer_d d;
struct phv_              p;
%%

execute_ingress_policer:
  seq         c1, d.execute_ingress_policer_d.entry_valid, TRUE
  seq         c2, d.execute_ingress_policer_d.tbkt[39], TRUE
  setcf       c3, [c1&c2]
  nop.!c3.e
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_INGRESS_POLICER], 1
  phvwr       p.capri_intrinsic_drop, TRUE
