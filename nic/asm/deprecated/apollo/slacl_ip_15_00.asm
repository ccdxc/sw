#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_ip_15_00_k k;
struct slacl_ip_15_00_d d;
struct phv_ p;

%%

slacl_ip_15_00:
    mod             r7, k.slacl_metadata_ip_15_00, 51
    add             r1, k.slacl_metadata_base_addr, k.key_metadata_dport
    addi            r1, r1, SLACL_DPORT_TABLE_OFFSET
    phvwr           p.slacl_metadata_addr2, r1
    mul             r7, r7, 10
    add             r1, k.slacl_metadata_base_addr, k.key_metadata_proto[7:1]
    addi            r1, r1, SLACL_PROTO_TABLE_OFFSET
    phvwr           p.slacl_metadata_addr3, r1
    tblrdp.e        r2, r7, 0, 10
    phvwr           p.slacl_metadata_class_id1, r2

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ip_15_00_error:
    nop.e
    nop
