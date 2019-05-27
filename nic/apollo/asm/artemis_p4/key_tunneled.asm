#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_tunneled_k.h"

struct key_tunneled_k_ k;
struct key_tunneled_d  d;
struct phv_ p;

%%

nop:
set_tep2_dst:
tunneled_ipv4_packet:
tunneled_ipv6_packet:
tunneled_nonip_packet:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
