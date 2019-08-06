#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_nacl_stats_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct nacl_stats_k_ k;
struct nacl_stats_d d;
struct phv_         p;

%%

nop:
  nop.e
  nop

.align
nacl_stats:
  tbladd.e      d.u.nacl_stats_d.stats_packets, 1
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_stats_error:
  nop.e
  nop
