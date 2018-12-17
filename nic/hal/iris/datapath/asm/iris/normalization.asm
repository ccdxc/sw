#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct normalization_k k;
struct phv_ p;

%%

normalization:
    seq         c1, k.l4_metadata_ip_normalization_en, TRUE
    balcf       r7, [c1], ip_normalization
    nop
    bbeq        k.l4_metadata_tcp_normalization_en, TRUE, \
                    tcp_stateless_normalization
    bbeq        k.l4_metadata_icmp_normalization_en, TRUE, icmp_normalization
    nop
    nop.e
    nop

#include "ip_normalization.h"
#include "icmp_normalization.h"
#include "tcp_stateless_normalization.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
normalization_error:
    nop.e
    nop
