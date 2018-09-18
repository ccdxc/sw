#include "tyche.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct rx_rewrite0_k k;
struct rx_rewrite0_d d;
struct phv_ p;

%%

#include "rx_rewrite.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_rewrite0_error:
    nop.e
    nop
