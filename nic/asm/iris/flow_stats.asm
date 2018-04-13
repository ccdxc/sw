#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"
#include "../../include/capri_common.h"
#include "nw.h"

struct flow_stats_k k;
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
  tblwr       d.flow_stats_d.last_seen_timestamp, r6
#else
  tblwr       d.flow_stats_d.last_seen_timestamp, r0
#endif
  seq         c1, d.flow_stats_d.permit_packets, 0xF
  bcf         [c1], flow_stats_permitted_overflow
  tbladd      d.flow_stats_d.permit_packets, 1
  tbladd.e    d.flow_stats_d.permit_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  nop

flow_stats_permitted_overflow:
  add         r7, d.flow_stats_d.permit_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  or          r7, r7, 0xF, 32
  or          r7, r7, 1, 56
  or          r7, r7, r5[31:27], 58

  add         r5, r5, k.flow_info_metadata_flow_index, 5
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]

  memwr.dx    r6, r7
  tblwr.e     d.flow_stats_d.permit_packets, 0
  tblwr       d.flow_stats_d.permit_bytes, 0

flow_stats_dropped:
  tblor       d.flow_stats_d.drop_reason, k.control_metadata_drop_reason
  seq         c1, d.flow_stats_d.drop_packets, 0xF
  bcf         [c1], flow_stats_dropped_overflow
  tbladd      d.flow_stats_d.drop_packets, 1
  tbladd.e    d.flow_stats_d.drop_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  nop

flow_stats_dropped_overflow:
  add         r7, d.flow_stats_d.drop_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  or          r7, r7, 0xF, 32
  or          r7, r7, 1, 56
  or          r7, r7, r5[31:27], 58

  add         r5, r5, k.flow_info_metadata_flow_index, 5
  add         r5, r5, 16
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]

  memwr.dx    r6, r7
  tblwr.e     d.flow_stats_d.drop_packets, 0
  tblwr       d.flow_stats_d.drop_bytes, 0

flow_stats_index_zero:
  nop

/*
 * stats allocation in the atomic add region:
 * 8B permit bytes, 8B permit packets, 8B deny bytes, 8B deny packets
 * total per flow index = 32B
 */

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_stats_error:
  nop.e
  nop
