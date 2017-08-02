#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ddos_service_policer_action_k     k;
struct ddos_service_policer_action_d     d;
struct phv_             p;

%%

nop:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ddos_service_policer_action:
  sle           c1, d.u.ddos_service_policer_action_d.ddos_service_policer_saved_color, k.ddos_metadata_ddos_service_policer_color     /* Check: Pkt_color  > entry's */
  tblwr.!c1     d.u.ddos_service_policer_action_d.ddos_service_policer_saved_color, k.ddos_metadata_ddos_service_policer_color         /* If, Pkt_color > entry's, update entry */
  seq           c2, d.u.ddos_service_policer_action_d.ddos_service_policer_saved_color, POLICER_COLOR_RED                             /* Check: entry_color == RED, then DROP */
  bcf           [c2], set_drop  /* Execs next inst. before branching */
  seq           c3, d.u.ddos_service_policer_action_d.ddos_service_policer_saved_color, POLICER_COLOR_YELLOW                          /* Check: entry_color == YELLOW, then Random DROP */
  bcf           [c3], set_drop                                                                                                      /* TODO: Have to do Random Drop */
  nop.!c3.e
  nop

set_drop:
  tbladd.e      d.u.ddos_service_policer_action_d.ddos_service_policer_dropped_packets, 1
  phvwr         p.control_metadata_egress_ddos_service_policer_drop, TRUE
