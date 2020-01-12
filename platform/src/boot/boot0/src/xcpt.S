
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

        #include "defs.h"
        #include "iomem.h"

        .section .text, "ax", @progbits
        .align  11
        .globl  xcpt_vectors
xcpt_vectors:
        b       xcpt_take       // +0x000: Current EL, SPL_EL0, Synchronous
        .align  7
        b       xcpt_take       // +0x080: Current EL, SPL_EL0, IRQ
        .align  7
        b       xcpt_take       // +0x100: Current EL, SPL_EL0, FIQ
        .align  7
        b       xcpt_take       // +0x180: Current EL, SPL_EL0, SError
        .align  7
        b       xcpt_take       // +0x200: Current EL, SPL_ELx, Synchronous
        .align  7
        b       xcpt_take       // +0x280: Current EL, SPL_ELx, IRQ
        .align  7
        b       xcpt_take       // +0x300: Current EL, SPL_ELx, FIQ
        .align  7
        b       xcpt_take       // +0x380: Current EL, SPL_ELx, SError

xcpt_take:
        str     x0, [sp, #-16]!
        ldr     x0, =xcpt_regs
        stp     x29, x30, [x0, #0x100]
        stp     x27, x28, [x0, #0x0f0]
        stp     x25, x26, [x0, #0x0e0]
        stp     x23, x24, [x0, #0x0d0]
        stp     x21, x22, [x0, #0x0c0]
        stp     x19, x20, [x0, #0x0b0]
        stp     x17, x18, [x0, #0x0a0]
        stp     x15, x16, [x0, #0x090]
        stp     x13, x14, [x0, #0x080]
        stp     x11, x12, [x0, #0x070]
        stp     x9, x10,  [x0, #0x060]
        stp     x7, x8,   [x0, #0x050]
        stp     x5, x6,   [x0, #0x040]
        stp     x3, x4,   [x0, #0x030]
        stp     x1, x2,   [x0, #0x020]
        ldr     x1, [sp]
        str     x1, [x0, #0x18]
        mrs     x1, esr_el3
        str     x1, [x0, #0x8]
        mrs     x1, elr_el3
        str     x1, [x0]
        mov     x1, sp
        add     x1, x1, #16
        str     x1, [x0, #0x10]
        ldr     x1, =(SSRAM_BASE + SSRAM_SIZE)
        mov     sp, x1
        b       xcpt_panic

        .section .bss
        .align  4
        .globl  xcpt_regs
xcpt_regs:
        .space  ((31 + 3) * 8)

