#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct replica_k k;
struct phv_      p;

%%

set_replica_rewrites:
  phvwr       p.control_metadata_flow_miss_egress, k.control_metadata_flow_miss
  phvwr       p.tunnel_metadata_tunnel_originate_egress, k.tunnel_metadata_tunnel_originate
  phvwr       p.tunnel_metadata_tunnel_terminate_egress, k.tunnel_metadata_tunnel_terminate
  seq         c1, k.capri_intrinsic_tm_instance_type, TM_INSTANCE_TYPE_CPU
  seq.!c1     c1, k.capri_intrinsic_tm_instance_type, TM_INSTANCE_TYPE_CPU_AND_DROP
  phvwr.c1    p.control_metadata_dst_lport, CPU_LPORT
  phvwr.c1    p.rewrite_metadata_tunnel_rewrite_index, 0
  phvwr.c1    p.rewrite_metadata_rewrite_index, 0

  seq         c1, k.tm_replication_data_valid, TRUE
  nop.!c1.e
  seq         c1, k.control_metadata_src_lport, k.{tm_replication_data_lport_sbit0_ebit4, \
                                                   tm_replication_data_lport_sbit5_ebit10}
  phvwr.c1.e  p.capri_intrinsic_drop, TRUE
  phvwr       p.tm_replication_data_valid, FALSE

  phvwr       p.control_metadata_dst_lport, k.{tm_replication_data_lport_sbit0_ebit4, \
                                               tm_replication_data_lport_sbit5_ebit10}
  phvwr       p.capri_rxdma_intrinsic_qtype, k.tm_replication_data_qtype
  seq         c1, k.{tm_replication_data_tunnel_rewrite_index_sbit0_ebit1, \
                     tm_replication_data_tunnel_rewrite_index_sbit2_ebit9}, 0
  phvwr.c1    p.capri_rxdma_intrinsic_qid, k.tm_replication_data_qid_or_vnid
  phvwr.!c1   p.rewrite_metadata_tunnel_rewrite_index, \
                  k.{tm_replication_data_tunnel_rewrite_index_sbit0_ebit1, \
                     tm_replication_data_tunnel_rewrite_index_sbit2_ebit9}
  phvwr.!c1   p.rewrite_metadata_tunnel_vnid, k.tm_replication_data_qid_or_vnid
  phvwr.e     p.rewrite_metadata_rewrite_index, \
                  k.{tm_replication_data_rewrite_index_sbit0_ebit3, \
                     tm_replication_data_rewrite_index_sbit4_ebit11}
  nop
