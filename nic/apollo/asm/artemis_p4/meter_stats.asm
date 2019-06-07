#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_meter_stats_k.h"
#include "nic/apollo/p4/include/artemis_table_sizes.h"
#include "platform/capri/capri_common.hpp"

struct meter_stats_k_   k;
struct phv_ p;

%%

meter_stats:
    seq             c1, r5, r0
    nop.c1.e
    add             r1, k.rewrite_metadata_meter_idx, METER_STATS_TABLE_SIZE, 3
    seq             c1, k.control_metadata_direction, TX_FROM_HOST
    add.!c1         r5, r5, r1
    add             r7, r0, k.rewrite_metadata_meter_len
    or              r7, r7, r5[32:27], 58
    addi            r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    add.e           r6, r6, r5[26:0]
    memwr.dx        r6, r7

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
meter_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
