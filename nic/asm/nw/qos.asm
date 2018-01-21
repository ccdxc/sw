#include "nw.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct qos_k k;
struct qos_d d;
struct phv_  p;

%%
qos:
  /* copy the oq to iq, needed by PBC */
  seq       c1, k.control_metadata_recirc_reason, RECIRC_NONE
  phvwr.c1  p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
  phvwr     p.capri_intrinsic_tm_oq, d.qos_d.egress_tm_oq

  /* qos info */
  ASSERT_PHVWR(p, qos_metadata_cos_en, qos_metadata_dscp,
               d, qos_d.cos_en, qos_d.dscp)
  phvwr.e   p.{qos_metadata_cos_en, \
               qos_metadata_cos, \
               qos_metadata_dscp_en, \
               qos_metadata_dscp}, \
            d.{qos_d.cos_en, \
               qos_d.cos, \
               qos_d.dscp_en, \
               qos_d.dscp}

  /* output queue selection */
  phvwr     p.control_metadata_dest_tm_oq, d.qos_d.dest_tm_oq

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
qos_error:
  seq           c1, k.control_metadata_recirc_reason, RECIRC_NONE
  phvwr.c1.e    p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
  nop
