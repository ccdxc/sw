#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_flow_stats_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "platform/capri/capri_common.hpp"
#include "nw.h"

struct flow_stats_k_ k;
struct flow_stats_d d;
struct phv_         p;

%%

flow_stats:
  K_DBG_WR(0xb0)
  DBG_WR(0xb8, 0xb8)
  seq         c1, k.flow_info_metadata_flow_index, 0
  b.c1.e      flow_stats_index_zero
  seq         c1, k.capri_intrinsic_drop, TRUE
  bcf         [c1], flow_stats_dropped
#ifndef CAPRI_IGNORE_TIMESTAMP
  tblwr       d.flow_stats_d.last_seen_timestamp, r4[47:16]
#else
  tblwr       d.flow_stats_d.last_seen_timestamp, r0
#endif
  tbladd.e    d.flow_stats_d.permit_packets, 1
  tbladd.f    d.flow_stats_d.permit_bytes, k.capri_p4_intrinsic_packet_len

flow_stats_dropped:
  tblor       d.flow_stats_d.drop_reason, k.control_metadata_drop_reason
  tbladd.e    d.flow_stats_d.drop_packets, 1
  tbladd.f    d.flow_stats_d.drop_bytes, k.capri_p4_intrinsic_packet_len

flow_stats_index_zero:
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_stats_error:
  nop.e
  nop
