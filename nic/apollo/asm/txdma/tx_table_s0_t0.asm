#include "apollo_txdma.h"
#include "INGRESS_p.h"

struct phv_ p;

%%
    .param      read_qstate_info

tx_table_s0_t0:
    j           read_qstate_info
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_table_s0_t0_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
