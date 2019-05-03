
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
        #include "iomem.h"

#define BOOT0_SIZE_MAGIC     0xd5c7a235

        .section .text, "ax", @progbits
        .globl  _start
_start:
        b       0f
        .word   BOOT0_SIZE_MAGIC
        .word   0       // will be filled in with the file size

0:
        // Set bit 8 in L2ACTLR_EL1 to disable DVM message broadcast
        // as these are not supported by the NOC.  Also disabled cache
        // maintainence operation broadcast.
        mrs     x0, S3_1_c15_c0_0
        orr     x0, x0, #0x100
        msr     S3_1_c15_c0_0, x0

        // Enable FP for varargs
        msr     cptr_el3, xzr
        isb

        // Set up the stack pointer
        ldr     x1, =(SSRAM_BASE + SSRAM_SIZE)
        mov     sp, x1

        // Zero BSS
        ldr     x0, =__bss_start__
        ldr     x1, =__bss_end__
        mov     x3, xzr
        mov     x4, xzr
        mov     x5, xzr
        mov     x6, xzr
        mov     x7, xzr
        mov     x8, xzr
        mov     x9, xzr
        mov     x10, xzr
        b       1f
0:
        stp     x3, x4, [x0, #0]
        stp     x5, x6, [x0, #16]
        stp     x7, x8, [x0, #32]
        stp     x9, x10, [x0, #48]
        add     x0, x0, #64
1:
        cmp     x0, x1
        b.ne    0b

        // Continue to main()
        b       main
