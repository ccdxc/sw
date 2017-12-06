#include "p4pt.h"

struct p4pt_update_write_latency_distribution_k k;
struct p4pt_update_write_latency_distribution_d d;
struct phv_ p;

%%

    .param  p4pt_tcb_iscsi_write_latency_distribution_base
/*
 * - stage 5: write p4pt_tcb_iscsi_write_latency_distribution
 *    - k vector: p4pt_global, p4pt_s2s
 *    - d vector: p4pt_tcb_iscsi_write_latency_distribution
 *    - if p4pt_s2s.write:
 *         return
 *      if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE15_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range15++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE14_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range14++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE13_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range13++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE12_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range12++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION1RANGE11_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range11++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE10_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range10++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE9_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range9++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE8_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range8++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE7_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range7++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE6_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range6++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE5_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range5++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE4_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range4++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE3_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range3++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE2_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range2++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE1_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range1++
 *      else if p4pt_global.latency > P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE0_LOW
 *         p4pt_tcb_iscsi_write_latency_distribution.range0++
 *      write p4pt_tcb_iscsi_write_latency_distribution
 *      - lookup launch p4pt_tcb_iscsi_write_latency_distribution
 *         lookup addr = P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_BASE_ADDR + \
 *                       p4pt_global.p4pt_idx * P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_SIZE
 *
 */

p4pt_update_write_latency_distribution_start:
    P4PT_CHECK_EXIT

    seq      c1, k.p4pt_s2s_read, 1
    b.c1     p4pt_update_write_latency_distribution_return

    add      r1, r0, k.{p4pt_global_latency_sbit0_ebit3...p4pt_global_latency_sbit28_ebit31}
    addi     r3, r0, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE15_LOW
    sle      c1, r1, r3
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range15
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    addi     r3, r0, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE14_LOW
    sle      c1, r1, r3
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range14
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    addi     r3, r0, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE13_LOW
    sle      c1, r1, r3
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range13
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    addi     r3, r0, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE12_LOW
    sle      c1, r1, r3
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range12
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE11_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range11
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE10_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range10
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE9_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range9
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE8_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range8
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE7_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range7
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE6_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range6
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE5_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range5
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE4_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range4
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE3_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range3
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE2_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range2
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE1_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range1
    add.c1   r2, r0, 1
    b.c1     p4pt_update_write_latency_distribution_return

    sle      c1, r1, P4PT_ISCSI_LATENCY_DISTRIBUTION_RANGE0_LOW
    add      r2, r0, d.u.p4pt_update_write_latency_distribution_d.range0
    add.c1   r2, r0, 1

    P4PT_CLEAR_ALL_TABLES
    illegal

p4pt_update_write_latency_distribution_return:
    nop.e
    nop
