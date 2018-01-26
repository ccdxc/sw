#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_hdr_transpositions0_k k;
struct rx_hdr_transpositions0_d d;
struct phv_ p;

%%

#include "rx_hdr_transpositions.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_hdr_transpositions0_error:
    nop.e
    nop
