#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_hdr_transpositions1_k k;
struct tx_hdr_transpositions1_d d;
struct phv_ p;

%%

#include "tx_hdr_transpositions.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_hdr_transpositions1_error:
    nop.e
    nop
