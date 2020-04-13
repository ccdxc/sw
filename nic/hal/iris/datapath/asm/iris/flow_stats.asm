#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_flow_stats_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "asic/cmn/asic_common.hpp"
#include "nw.h"

struct flow_stats_k_ k;
struct flow_stats_d d;
struct phv_         p;

%%

flow_stats:
  seq         c1, k.control_metadata_skip_flow_update, TRUE
  seq.!c1     c1, k.flow_info_metadata_flow_index, 0
  nop.c1.e

  add         r7, r0, k.capri_p4_intrinsic_packet_len
  smeqb       c1, k.flow_lkp_metadata_lkp_type, \
                FLOW_KEY_LOOKUP_TYPE_IP_MASK, FLOW_KEY_LOOKUP_TYPE_IP_MASK
  seq         c2, k.vlan_tag_valid, TRUE
  cmov        r6, c2, 18, 14
  sub.c1      r7, r7, r6

  seq         c1, k.capri_intrinsic_drop, TRUE
  bcf         [c1], flow_stats_dropped
#ifndef CAPRI_IGNORE_TIMESTAMP
  tblwr.!c1   d.flow_stats_d.last_seen_timestamp, r4[47:16]
#else
  tblwr.!c1   d.flow_stats_d.last_seen_timestamp, r0
#endif
  tbladd.e    d.flow_stats_d.permit_packets, 1
  tbladd.f    d.flow_stats_d.permit_bytes, r7

flow_stats_dropped:
  add         r1, r0, 1, DROP_FLOW_HIT
  seq         c1, k.control_metadata_drop_reason, r1
#ifndef CAPRI_IGNORE_TIMESTAMP
  tblwr.c1    d.flow_stats_d.last_seen_timestamp, r4[47:16]
#else
  tblwr.c1    d.flow_stats_d.last_seen_timestamp, r0
#endif
  tblor       d.flow_stats_d.drop_reason, k.control_metadata_drop_reason
  tbladd.e    d.flow_stats_d.drop_packets, 1
  tbladd.f    d.flow_stats_d.drop_bytes, r7

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_stats_error:
  nop.e
  nop
