#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"
#include "nw.h"

struct twice_nat_k k;
struct twice_nat_d d;
struct phv_        p;

%%
nop:
  K_DBG_WR(0xf0)
  DBG_WR(0xf8, 0xf8)
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
twice_nat_rewrite_info:
  K_DBG_WR(0xf0)
  DBG_WR(0xf9, 0xf9)
  phvwr.e     p.nat_metadata_twice_nat_ip, d.u.twice_nat_rewrite_info_d.ip
  phvwr       p.nat_metadata_twice_nat_l4_port, d.u.twice_nat_rewrite_info_d.l4_port
