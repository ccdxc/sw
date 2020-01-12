
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

        #include "defs.h"
        #include "iomem.h"
        #include "version.h"

        .section .text.startup, "ax", @progbits
        .globl  _start
_start:
        b       0f
        .word   UBOOT_SIZE_MAGIC
        .word   0               // will be filled in with the file size
        .word   BOOT0_MAGIC     // identify this as boot0 rather than uboot
        .word   UBOOT_CRC32_MAGIC;
        .word   0               // will be filled in with image crc32
        .word   BOOT0_VERSION

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

        // Relocate down to SSRAM
        adr     x0, _start              // src: current PC-rel address in flash
        ldr     x1, =_start             // dst: symbol address in SSRAM
        ldr     x2, =_end               // dst: symbol address in SSRAM
0:
        ldp     x3, x4, [x0], #16
        stp     x3, x4, [x1], #16
        cmp     x1, x2
        bne     0b

        // Zero BSS
        ldr     x0, =__bss_start__
        ldr     x1, =__bss_end__
        b       1f
0:
        str     xzr, [x0], #8
1:
        cmp     x0, x1
        bne     0b

        // Setup exception vectors to catch pre-u-boot errors
        ldr     x0, =xcpt_vectors
        msr     vbar_el3, x0

        // Continue to main
        isb
        ldr     x9, =main
        br      x9
