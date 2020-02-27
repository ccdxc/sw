#include "egress.h"
#include "EGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct twice_nat_k k;
struct twice_nat_d d;
struct phv_        p;

%%
nop:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
twice_nat_rewrite_info:
  phvwr.e     p.nat_metadata_twice_nat_ip, d.u.twice_nat_rewrite_info_d.ip
  phvwr       p.nat_metadata_twice_nat_l4_port, d.u.twice_nat_rewrite_info_d.l4_port

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
twice_nat_error:
  nop.e
  nop
