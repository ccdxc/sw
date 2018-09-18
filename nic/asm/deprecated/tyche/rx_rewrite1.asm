#include "tyche.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct rx_rewrite1_k k;
struct rx_rewrite1_d d;
struct phv_ p;

%%

#include "rx_rewrite.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_rewrite1_error:
    nop.e
    nop
