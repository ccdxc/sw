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
  phvwr       p.capri_intrinsic_tm_span_session, 0
  phvwr       p.control_metadata_dst_lport, d.u.local_span_d.dst_lport
  seq.e       c1, d.u.local_span_d.truncate_len, 0
  phvwr.!c1   p.capri_p4_intrinsic_frame_size, d.u.local_span_d.truncate_len

.align
remote_span:
  phvwr       p.capri_intrinsic_tm_span_session, 0
  phvwr       p.control_metadata_dst_lport, d.u.remote_span_d.dst_lport
  phvwr       p.tunnel_metadata_tunnel_originate, TRUE
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, d.u.remote_span_d.tunnel_rewrite_index
  phvwr       p.rewrite_metadata_tunnel_vnid, d.u.remote_span_d.vlan
  seq.e       c1, d.u.remote_span_d.truncate_len, 0
  phvwr.!c1   p.capri_p4_intrinsic_frame_size, d.u.remote_span_d.truncate_len

.align
erspan_mirror:
  phvwr       p.capri_intrinsic_tm_span_session, 0
  phvwr       p.control_metadata_dst_lport, d.u.erspan_mirror_d.dst_lport
  phvwr       p.tunnel_metadata_tunnel_originate, TRUE
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, d.u.erspan_mirror_d.tunnel_rewrite_index
  seq.e       c1, d.u.erspan_mirror_d.truncate_len, 0
  phvwr.!c1   p.capri_p4_intrinsic_frame_size, d.u.erspan_mirror_d.truncate_len

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
drop_mirror:
  phvwr.e     p.capri_intrinsic_drop, TRUE
  nop
