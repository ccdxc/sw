#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct registered_macs_k k;
struct registered_macs_d d;
struct phv_              p;

%%

registered_macs_miss:
  seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  smeqb       c2, k.control_metadata_lif_filter, LIF_FILTER_ALL_MULTICAST, LIF_FILTER_ALL_MULTICAST
  bcf         [c1&c2], registered_macs_all_multicast
  seq         c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_BROADCAST
  smeqb       c2, k.control_metadata_lif_filter, LIF_FILTER_BROADCAST, LIF_FILTER_BROADCAST
  bcf         [c1&c2], registered_macs_broadcast
  smeqb       c2, k.control_metadata_lif_filter, LIF_FILTER_PROMISCUOUS, LIF_FILTER_PROMISCUOUS
  bcf         [c2], registered_macs_promiscuous
  phvwr.!c2.e p.capri_intrinsic_drop, TRUE
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
registered_macs_hit:
  phvwr.e     p.capri_intrinsic_lif, d.u.registered_macs_hit_d.dst_lif
  nop

registered_macs_all_multicast:
  phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
  phvwr       p.capri_intrinsic_tm_replicate_ptr, k.control_metadata_flow_miss_idx

registered_macs_broadcast:
  add         r1, k.control_metadata_flow_miss_idx, 1
  phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
  phvwr       p.capri_intrinsic_tm_replicate_ptr, r1

registered_macs_promiscuous:
  add         r1, k.control_metadata_flow_miss_idx, 2
  phvwr.e     p.capri_intrinsic_tm_replicate_en, TRUE
  phvwr       p.capri_intrinsic_tm_replicate_ptr, r1
