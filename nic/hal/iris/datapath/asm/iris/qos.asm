#include "nw.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct qos_k k;
struct qos_d d;
struct phv_  p;

%%
qos:
  add       r1, (LIF_STATS_TX_UCAST_BYTES_OFFSET / 64), \
                k.control_metadata_src_lif, 4
  phvwr     p.control_metadata_lif_tx_stats_idx, r1
  /* copy the oq to iq, needed by PBC */
  sne       c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
  phvwr.c1  p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
  phvwr     p.capri_intrinsic_tm_oq, d.qos_d.egress_tm_oq

  /* qos info */
  phvwr     p.qos_metadata_cos_en, d.qos_d.cos_en
  phvwr     p.{qos_metadata_cos, qos_metadata_dscp_en}, \
               d.{qos_d.cos, qos_d.dscp_en}
  phvwr.e   p.qos_metadata_dscp, d.qos_d.dscp
  // phvwr.e   p.{qos_metadata_cos,qos_metadata_dscp_en,qos_metadata_dscp}, \
  //               d.{qos_d.cos,qos_d.dscp_en,qos_d.dscp}

  /* output queue selection */
  phvwr     p.control_metadata_dest_tm_oq, d.qos_d.dest_tm_oq

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
qos_error:
  phvwr       p.capri_intrinsic_drop, TRUE
  phvwr       p.control_metadata_drop_reason[DROP_HARDWARE_ERR], TRUE
  sne.e       c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
  phvwr.c1    p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
