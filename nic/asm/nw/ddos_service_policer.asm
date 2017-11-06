#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ddos_service_policer_d   d;
struct phv_                     p;

%%

execute_ddos_service_policer:
  seq           c1, d.execute_ddos_service_policer_d.tbkt[39], 0
  phvwr.c1.e    p.ddos_metadata_ddos_service_policer_color, POLICER_COLOR_GREEN
  seq           c2, d.execute_ddos_service_policer_d.tbkt2[39], 0
  phvwr.c2.e    p.ddos_metadata_ddos_service_policer_color, POLICER_COLOR_YELLOW
  phvwr.!c2.e   p.ddos_metadata_ddos_service_policer_color, POLICER_COLOR_RED
  nop
