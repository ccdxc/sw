#include "egress.h"
#include "EGRESS_p.h"
#include "nw.h"

struct clock_d  d;
struct phv_     p;

%%

gettimeofday:
    seq             c1, d.gettimeofday_d.multiplier_ns, 0
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
    mul             r2, r1[23:0], d.gettimeofday_d.multiplier_ns[31:0]
    mul             r3, r1[47:24], d.gettimeofday_d.multiplier_ns[31:0]
    mul             r4, r1[23:0], d.gettimeofday_d.multiplier_ns[33:32]
    mul             r5, r1[47:24], d.gettimeofday_d.multiplier_ns[33:32]
    add             r2, r2[63:31], r3[63:7]
    add             r4, r4, r5, 24
    sll             r4, r4, 1
    add             r2, r2, r4
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
