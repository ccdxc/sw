
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "defs.h"
#include "bsm.h"

void
panic(const char *s)
{
    bsm_t bsm = { 0 };

    logf_printf("panic: %s\n", s);
    if (board_reset_on_panic()) {
        // Complete reset upon panic.
        bsm_save(&bsm);             // zero BSM state for full restart
        uart_wait_idle();           // wait for panic message to drain
        wdt_sys_reset();            // chip reset
    }
    for (;;) {
        asm volatile("wfi");
    }
}

/*
 * Fatal exception taken
 * esr: 0000000002000000
 * pc : 00000000004004fc,  lr: 00000000004004fc,  sp: 000000000041ffd0
 * x29: 000000000041ffd0, x28: 0000000000000000, x27: 0000000000000000
 * x26: 0000000000000000, x25: 0000000000000000, x24: 0000000000000000
 * x23: 0000000000000000, x22: 0000000000000000, x21: 0000000000000000
 * x20: 0000000000000000, x19: 0000000000000000, x18: 0000000000000000
 * x17: 0000000000000000, x16: 0000000000000000, x15: 0000000000000000
 * x14: 0000000000000000, x13: 0000000000000000, x12: 0000000000000000
 * x11: 0000000000000000, x10: 0000000000000000, x9 : 00000000004004d4
 * x8 : 0000000000000000, x7 : 0000000000000000, x6 : 0000000000000000
 * x5 : 0000000000000000, x4 : 0000000000000002, x3 : 000000009fa00001
 * x2 : 0000000000002410, x1 : 0000000000002400, x0 : 0000000080780081
 */
void
xcpt_panic(struct xcpt_regs *regs)
{
    static int in_panic = 0;
    int r;

    if (!in_panic) {
        in_panic = 1;
        uart_init();
        logf_printf("Fatal exception taken\n");
        logf_printf("esr: %016lx\n", regs->esr);
        logf_printf("pc : %016lx,  lr: %016lx,  sp: %016lx\n",
                regs->elr, regs->x[30], regs->sp);
        for (r = 29; r >= 0; r--) {
            logf_printf("x%-2d: %016lx%s", r,
                    regs->x[r], (r % 3) ? ", " : "\n");
        }
        panic("Fatal exception taken");
    }
    for (;;) {
        asm volatile("wfi");
    }
}

