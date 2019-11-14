#include "egress.h"
#include "EGRESS_p.h"
#include "nw.h"

struct clock_d  d;
struct phv_     p;

%%

gettimeofday:
    seq             c1, d.gettimeofday_d.multiplier, 0
    nop.c1.e
    slt             c1, r4, d.gettimeofday_d.ticks
    bcf             [c1], ticks_rollover
    sub             r1, r4, d.gettimeofday_d.ticks
    b               ticks_to_ns
ticks_rollover:
    add             r2, r0, 1, 49
    sub             r2, r2, 1
    add             r1, r1, r2
ticks_to_ns:
    mul             r2, r1[23:0], d.gettimeofday_d.multiplier
    mul             r3, r1[47:24], d.gettimeofday_d.multiplier
    add             r2, r2, r3, 24
    srl             r2, r2, d.gettimeofday_d.shift
    add.e           r1, r2, d.gettimeofday_d.time_in_ns
    phvwr.f         p.control_metadata_current_time_in_ns, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
clock_error:
    nop.e
    nop
