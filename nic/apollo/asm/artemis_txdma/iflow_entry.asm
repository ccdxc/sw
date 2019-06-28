#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_iflow_entry_k.h"

struct iflow_entry_k_       k;
struct iflow_entry_d        d;
struct phv_                 p;

%%
    .param          invalid_flow_base

iflow_entry:
    addi            r1, r0, loword(invalid_flow_base)
    seq             c1, k.txdma_control_pktdesc_addr, r1
    b.c1            iflow_entry_done

    add             r1, 0, TXDMA_IFLOW_PARENT_FLIT * 512
    phvwrp.e        r1, 0, 512, d.iflow_entry_d.flow
    nop
iflow_entry_done:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
iflow_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
