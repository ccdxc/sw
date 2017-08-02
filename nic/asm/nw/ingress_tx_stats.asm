#include "ingress.h"
#include "INGRESS_p.h"

struct ingress_tx_stats_k k;
struct ingress_tx_stats_d d;
struct phv_               p;

%%

ingress_tx_stats:
  seq         c2, k.capri_intrinsic_drop, 0
  nop.c2.e
  add         r6, d.ingress_tx_stats_d.tx_ingress_drops, k.capri_intrinsic_drop
  beqi        r6, 0xFFFF, ingress_tx_stats_overflow
  add         r5, r5, k.control_metadata_src_lif, 3
  tblwr.e     d.ingress_tx_stats_d.tx_ingress_drops, r6
  nop

ingress_tx_stats_overflow:
  memwr.d.e   r5, r6
  tblwr       d.ingress_tx_stats_d.tx_ingress_drops, r0
