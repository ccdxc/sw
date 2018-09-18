#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_p3_k k;
struct slacl_p3_d d;
struct phv_ p;

%%

slacl_action:
    phvwr.e         p.slacl_metadata_stats_index, d.slacl_action_d.rule_id
    phvwr           p.slacl_metadata_drop, d.slacl_action_d.drop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p3_error:
    nop.e
    nop
