#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_replica_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct replica_k_ k;
struct phv_       p;

%%

set_replica_rewrites:
  // or          r1, k.tunnel_metadata_tunnel_originate[0], \
  //               k.control_metadata_i2e_flags[P4_I2E_FLAGS_NIC_MODE], 1
  // or          r1, r1, k.tunnel_metadata_tunnel_terminate[0], 2
  // phvwr       p.{tunnel_metadata_tunnel_terminate_egress, \
  //               control_metadata_nic_mode_e, \
  //               tunnel_metadata_tunnel_originate_egress}, r1
  or          r1, k.tunnel_metadata_tunnel_originate[0], \
                  k.tunnel_metadata_tunnel_terminate[0], 1
  phvwr       p.{tunnel_metadata_tunnel_terminate_egress, \
                 tunnel_metadata_tunnel_originate_egress}, r1
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

  seq         c1, k.control_metadata_src_lport, k.tm_replication_data_lport
  and         r1, k.tunnel_metadata_tunnel_terminate, k.tm_replication_data_is_tunnel
  seq.!c1     c1, r1[0], 1
  phvwr.c1    p.control_metadata_same_if_check_failed, TRUE

  phvwr       p.control_metadata_dst_lport, k.tm_replication_data_lport
  phvwr       p.control_metadata_qtype, k.tm_replication_data_qtype
  phvwr       p.rewrite_metadata_rewrite_index, \
                  k.tm_replication_data_rewrite_index
  seq         c1, k.tm_replication_data_is_qid, TRUE
  phvwr.c1    p.control_metadata_qid, k.tm_replication_data_qid_or_vnid
  phvwr.!c1   p.rewrite_metadata_tunnel_vnid, k.tm_replication_data_qid_or_vnid
  phvwr.e     p.tunnel_metadata_tunnel_originate_egress, k.tm_replication_data_is_tunnel
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, \
                  k.tm_replication_data_tunnel_rewrite_index

lb_replica_honor_ingress:
  nop.e
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
replica_error:
  phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
  phvwr.e     p.capri_intrinsic_drop, TRUE
  phvwr       p.control_metadata_egress_drop_reason[EGRESS_DROP_HARDWARE_ERR], TRUE
