#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct read_control_k       k;
struct read_control_d       d;

%%

read_control_info:
    // TODO: Check endianness. Is it 510:x or y:1 
    phvwr.e     p.{predicate_header_pad0...p4_to_txdma_header_vcn_id}, \
                    d.read_control_info_d.data[510: 510 - \
                        (offsetof(p, predicate_header_pad0) + sizeof(p.predicate_header_pad0) - 1 - \
                         offsetof(p, p4_to_txdma_header_vcn_id))]
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_control_error:
    nop.e
    nop
