#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_stats_k k;
struct flow_stats_d d;
struct phv_         p;

%%

flow_stats:
  seq         c1, k.{flow_info_metadata_flow_index_sbit0_ebit3, \
                  flow_info_metadata_flow_index_sbit4_ebit19}, 0
  b.c1.e      flow_stats_index_zero
  seq         c1, k.capri_intrinsic_drop, TRUE
  bcf         [c1], flow_stats_dropped
  tblwr       d.flow_stats_d.last_seen_timestamp, r6
  seq         c1, d.flow_stats_d.permit_packets, 0xF
  bcf         [c1], flow_stats_permitted_overflow
  tbladd      d.flow_stats_d.permit_packets, 1
  tbladd.e    d.flow_stats_d.permit_bytes, k.control_metadata_packet_len
  nop

flow_stats_permitted_overflow:
  add         r7, d.flow_stats_d.permit_bytes, k.control_metadata_packet_len
  or          r7, r7, 0xF, 32
  or          r7, r7, 1, 56
  add         r5, r5, k.{flow_info_metadata_flow_index_sbit0_ebit3, \
                         flow_info_metadata_flow_index_sbit4_ebit19}, 5
  memwr.d     r5, r7
  tblwr.e     d.flow_stats_d.permit_packets, 0
  tblwr       d.flow_stats_d.permit_bytes, 0

flow_stats_dropped:
  tblor       d.flow_stats_d.drop_reason, k.control_metadata_drop_reason
  seq         c1, d.flow_stats_d.drop_packets, 0xF
  bcf         [c1], flow_stats_dropped_overflow
  tbladd      d.flow_stats_d.drop_packets, 1
  tbladd.e    d.flow_stats_d.drop_bytes, k.control_metadata_packet_len
  nop

flow_stats_dropped_overflow:
  add         r7, d.flow_stats_d.drop_bytes, k.control_metadata_packet_len
  or          r7, r7, 0xF, 32
  or          r7, r7, 1, 56
  add         r5, r5, k.{flow_info_metadata_flow_index_sbit0_ebit3, \
                         flow_info_metadata_flow_index_sbit4_ebit19}, 5
  add         r5, r5, 16
  memwr.d     r5, r7
  tblwr.e     d.flow_stats_d.drop_packets, 0
  tblwr       d.flow_stats_d.drop_bytes, 0

flow_stats_index_zero:
  nop

/*
 * stats allocation in the atomic add region:
 * 8B permit bytes, 8B permit packets, 8B deny bytes, 8B deby packets
 * total per policer index = 32B
 */
