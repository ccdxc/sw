#include "tyche.h"
#include "egress.h"
#include "EGRESS_p.h"

struct tx_rewrite1_k k;
struct tx_rewrite1_d d;
struct phv_ p;

%%

#include "tx_rewrite.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_rewrite1_error:
    nop.e
    nop
