#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct registered_macs_k k;
struct registered_macs_d d;
struct phv_              p;

%%

registered_macs:
  bcf         [c1], registered_macs_hit
  phvwr       p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  bcf         [c1], registered_macs_all_multicast
  seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_BROADCAST
  bcf         [c1], registered_macs_broadcast
registered_macs_promiscuous:
  add.!c1     r1, k.control_metadata_flow_miss_idx, 2
  phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
  phvwr       p.capri_intrinsic_tm_replicate_ptr, r1

registered_macs_hit:
  seq         c1, d.registered_macs_d.multicast_en, 1
  phvwr.c1.e  p.capri_intrinsic_tm_replicate_en, 1
  phvwr.c1    p.capri_intrinsic_tm_replicate_ptr, d.registered_macs_d.dst_lport
  phvwr.!c1.e p.control_metadata_dst_lport, d.registered_macs_d.dst_lport
  nop

registered_macs_broadcast:
  phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
  phvwr       p.capri_intrinsic_tm_replicate_ptr, k.control_metadata_flow_miss_idx

registered_macs_all_multicast:
  add         r1, k.control_metadata_flow_miss_idx, 1
  phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
  phvwr       p.capri_intrinsic_tm_replicate_ptr, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
registered_macs_error:
  nop.e
  nop
