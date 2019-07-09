#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct local_46_mapping_txdma_k  k;
struct local_46_mapping_txdma_d  d;
struct phv_       p;

%%

local_46_mapping_txdma:
    // Pass 46 prefix for rflow derivation
    // This is index table, it will be hit even for zero index and expecting zero
    // prefix to be programmed therem. Also we dont check if the prefix is non-zero
    // or not
    phvwr.e     p.txdma_control_tx_local_46_ip, d.local_46_info_txdma_d.prefix
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_46_mapping_txdma_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
