#include "ingress.h"
#include "INGRESS_p.h"
#include "../../include/capri_common.h"
#include "../../p4/nw/include/defines.h"

struct ingress_tx_stats_k k;
struct ingress_tx_stats_d d;
struct phv_               p;

%%

ingress_tx_stats:
  phvwr       p.capri_p4_intrinsic_valid, TRUE
  seq         c2, k.capri_intrinsic_drop, 0
  b.c2        tcp_options_fixup
  add         r7, d.ingress_tx_stats_d.tx_ingress_drops, k.capri_intrinsic_drop
  beqi        r7, 0xFFFF, ingress_tx_stats_overflow
  add         r5, r5, k.control_metadata_src_lif, 3
  b           tcp_options_fixup
  tblwr       d.ingress_tx_stats_d.tx_ingress_drops, r6

ingress_tx_stats_overflow:
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]
  or          r7, r7, r5[31:27], 58
  memwr.dx    r6, r7
  b           tcp_options_fixup
  tblwr       d.ingress_tx_stats_d.tx_ingress_drops, r0

/*
 * stats allocation in the atomic add region:
 * 8B drop packet count
 * total per lif index = 8B
 */

#include "tcp_options_fixup.asm"
