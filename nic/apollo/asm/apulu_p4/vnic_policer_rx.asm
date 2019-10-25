#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"

struct vnic_policer_rx_d    d;
struct phv_                 p;

%%

vnic_policer_rx:
  seq         c1, d.vnic_policer_rx_d.entry_valid, TRUE
  seq.c1      c1, d.vnic_policer_rx_d.tbkt[39], TRUE
  nop.!c1.e
  phvwr.c1.e  p.control_metadata_p4e_drop_reason[P4E_DROP_VNIC_POLICER_RX], 1
  phvwr.f     p.capri_intrinsic_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_policer_rx_error:
  nop.e
  nop
