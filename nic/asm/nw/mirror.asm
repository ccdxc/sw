#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct mirror_k k;
struct mirror_d d;
struct phv_     p;

%%

nop:
  nop.e
  nop

.align
local_span:
  add         r1, r0, d.u.local_span_d.truncate_len
  phvwr       p.control_metadata_dst_lport, d.u.local_span_d.dst_lport
  phvwr       p.tunnel_metadata_tunnel_originate, FALSE
  bne         r1, r0, span_truncation
  phvwrpair   p.rewrite_metadata_tunnel_rewrite_index, 0, \
              p.control_metadata_dest_tm_oq[4:0], d.u.local_span_d.span_tm_oq
  phvwr.e     p.capri_intrinsic_tm_span_session, 0
  nop

.align
remote_span:
  add         r1, r0, d.u.remote_span_d.truncate_len
  phvwr       p.control_metadata_dst_lport, d.u.remote_span_d.dst_lport
  phvwr       p.tunnel_metadata_tunnel_originate, TRUE
  phvwrpair   p.rewrite_metadata_tunnel_rewrite_index[9:0], \
                d.u.remote_span_d.tunnel_rewrite_index, \
              p.control_metadata_dest_tm_oq[4:0], \
                d.u.remote_span_d.span_tm_oq
  bne         r1, r0, span_truncation
  phvwr       p.rewrite_metadata_tunnel_vnid, d.u.remote_span_d.vlan
  phvwr.e     p.capri_intrinsic_tm_span_session, 0
  nop

.align
erspan_mirror:
  add         r1, r0, d.u.erspan_mirror_d.truncate_len
  phvwr       p.control_metadata_dst_lport, d.u.erspan_mirror_d.dst_lport
  phvwr       p.tunnel_metadata_tunnel_originate, TRUE
  bne         r1, r0, span_truncation
  phvwrpair   p.rewrite_metadata_tunnel_rewrite_index[9:0], \
                d.u.erspan_mirror_d.tunnel_rewrite_index, \
              p.control_metadata_dest_tm_oq[4:0], \
                d.u.erspan_mirror_d.span_tm_oq
  phvwr.e     p.capri_intrinsic_tm_span_session, 0
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
drop_mirror:
  phvwr       p.control_metadata_dst_lport, 0
  phvwr.e     p.capri_intrinsic_drop, TRUE
  phvwr       p.capri_intrinsic_tm_span_session, 0

span_truncation:
  sub         r2, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                     capri_p4_intrinsic_packet_len_sbit6_ebit13}, 14
  slt         c1, r1, r2
  nop.!c1.e
  phvwr       p.capri_intrinsic_tm_span_session, 0
  add         r3, r1, 14
  phvwr       p.capri_p4_intrinsic_packet_len, r3
  phvwr.e     p.capri_deparser_len_trunc_pkt_len, r1
  phvwr       p.{capri_intrinsic_payload,capri_deparser_len_trunc}, 0x1
