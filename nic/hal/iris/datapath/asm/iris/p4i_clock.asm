#include "ingress.h"
#include "INGRESS_p.h"
#include "nw.h"

struct p4i_clock_d  d;
struct phv_         p;

%%

gettimeofday_ms:
    seq             c1, d.gettimeofday_ms_d.multiplier_ms, 0
    nop.c1.e
    slt             c1, r4, d.gettimeofday_ms_d.ticks
    bcf             [c1], ticks_rollover
    sub             r1, r4, d.gettimeofday_ms_d.ticks
    b               ticks_to_ms
ticks_rollover:
    add             r2, r0, 1, 49
    sub             r2, r2, 1
    add             r1, r1, r2
ticks_to_ms:
    mul             r2, r1[23:0], d.gettimeofday_ms_d.multiplier_ms[31:0]
    mul             r3, r1[47:24], d.gettimeofday_ms_d.multiplier_ms[31:0]
    add             r2, r2[63:41], r3[63:17]
    add.e           r1, r2, d.gettimeofday_ms_d.time_in_ms
    phvwr.f         p.control_metadata_current_time_in_ms, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4i_clock_error:
    nop.e
    nop
