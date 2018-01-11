#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"
#include "nw.h"

struct replica_k k;
struct phv_      p;

%%

set_replica_rewrites:
  K_DBG_WR(0xe0)
  phvwr       p.control_metadata_flow_miss_egress, k.control_metadata_flow_miss
  phvwrpair   p.tunnel_metadata_tunnel_terminate_egress, \
                k.tunnel_metadata_tunnel_terminate[0], \
              p.tunnel_metadata_tunnel_originate_egress, \
                k.tunnel_metadata_tunnel_originate[0]
  phvwr       p.control_metadata_src_tm_iq, k.capri_intrinsic_tm_iq
  phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
  seq         c1, k.capri_intrinsic_tm_instance_type, TM_INSTANCE_TYPE_CPU
  seq.!c1     c1, k.capri_intrinsic_tm_instance_type, TM_INSTANCE_TYPE_CPU_AND_DROP
  phvwr.c1    p.control_metadata_dst_lport, CPU_LPORT
  phvwr.c1    p.rewrite_metadata_tunnel_rewrite_index, 0
  phvwr.c1    p.rewrite_metadata_rewrite_index, 0

  seq         c1, k.tm_replication_data_valid, TRUE
  nop.!c1.e

  seq         c1, k.tm_replication_data_repl_type, TM_REPL_TYPE_DEFAULT
  seq         c2, k.tm_replication_data_repl_type, TM_REPL_TYPE_TO_CPU_REL_COPY
  bcf         ![c1 | c2], lb_replica_honor_ingress
  phvwr       p.tm_replication_data_valid, FALSE

  seq         c1, k.control_metadata_src_lport, k.{tm_replication_data_lport_sbit0_ebit4, \
                                                   tm_replication_data_lport_sbit5_ebit10}
  and         r1, k.tunnel_metadata_tunnel_terminate, k.tm_replication_data_is_tunnel
  seq.!c1     c1, r1[0], 1
  phvwr.c1.e  p.capri_intrinsic_drop, TRUE

  phvwr.!c1   p.control_metadata_dst_lport, k.{tm_replication_data_lport_sbit0_ebit4, \
                                               tm_replication_data_lport_sbit5_ebit10}
  phvwr       p.control_metadata_qtype, k.tm_replication_data_qtype
  phvwr       p.rewrite_metadata_rewrite_index, \
                  k.{tm_replication_data_rewrite_index_sbit0_ebit3, \
                     tm_replication_data_rewrite_index_sbit4_ebit11}
  seq         c1, k.tm_replication_data_is_qid, TRUE
  phvwr.c1    p.control_metadata_qid, k.tm_replication_data_qid_or_vnid
  phvwr.!c1   p.rewrite_metadata_tunnel_vnid, k.tm_replication_data_qid_or_vnid
  phvwr.e     p.tunnel_metadata_tunnel_originate_egress, k.tm_replication_data_is_tunnel
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, \
                  k.{tm_replication_data_tunnel_rewrite_index_sbit0_ebit1, \
                     tm_replication_data_tunnel_rewrite_index_sbit2_ebit9}

lb_replica_honor_ingress:
  nop.e
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
replica_error:
  nop.e
  nop
