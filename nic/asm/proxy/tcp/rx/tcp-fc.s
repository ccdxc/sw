/*
 *  Implements the CC stage of the RxDMA P4+ pipeline
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_tcp_rx_k.h"

struct phv_ p;
struct s5_t0_tcp_rx_k_ k;
struct s5_t0_tcp_rx_tcp_fc_d d;

%%
    .param          tcp_rx_dma_serq_stage_start
    .param          tcp_rx_dma_rx2tx_stage_start
    .param          tcp_rx_write_arq_stage_start
    .align
tcp_rx_fc_stage_start:
    seq         c1, k.common_phv_write_arq, 1
    bcf         [c1], tcp_cpu_rx
    nop

    // TODO : FC stage has to be implemented

    // launch table 1 next stage
    CAPRI_NEXT_TABLE_READ_OFFSET(1, TABLE_LOCK_EN,
                tcp_rx_dma_rx2tx_stage_start, k.common_phv_qstate_addr,
                TCP_TCB_RX_DMA_OFFSET, TABLE_SIZE_512_BITS)

flow_fc_process:
    seq         c2, k.common_phv_write_serq, 1
    bcf         [!c2], flow_fc_process_done
    nop

    /* Figure out how many entries are free in serq */
    add         r2, k.to_s5_serq_cidx, CAPRI_SERQ_RING_SLOTS
    sub         r2, r2, k.to_s5_serq_pidx
    and         r2, r2, CAPRI_SERQ_RING_SLOTS - 1

    /* r2 free entries in serq. Reserve 32 and use the remaining in
    * the window calculation below.
    */ 
    sub         r2, r2, 31
    sle         c3, r2, 0
    add.c3      r2, r0, r0

    /* Assume each entry can hold 128 bytes */
    sll         r4, r2, 7

    /* If the window calculated is smaller than the window
    * advertised previously reset to previous window to avoid
    * shrinking rcv window. See the fix for Bug627496 in Linux
    * kernel.
    */

    add         r3, d.rcv_wup, d.rcv_wnd
    sub         r3, r3, k.to_s5_rcv_nxt

    /* r3 is the current window. Add ( 2 ^ wscale ) - 1 to
    * current window to avoid shrinking the window when we
    * apply scale
    */
    sll         r2, 1, d.rcv_scale
    sub         r2, r2, 1
    add         r3, r3, r2

    sle         c3, r4, r3
    add.c3      r4, r0, r3
    tblwr.!c3   d.rcv_wnd, r4

flow_fc_process_done:
    /* Apply the scale factor SEG.WND = RCV.WND >> Rcv.Wind.Scale */
    srl         r4, d.rcv_wnd, d.rcv_scale

    /* If scaled widnow is > 0xFFFF we have an issue with the scale
    * factor passed from LKL. Reset to 0xFFFF to avoid overflow.
    */
    slt         c3, 0xffff, r4
    add.c3      r4, r0, 0xffff

    /* We have a valid window after applying the scale factor. */
    phvwr       p.rx2tx_extra_rcv_wnd, r4
    tblwr       d.rcv_wup, k.to_s5_rcv_nxt

    bbeq        k.common_phv_ooo_rcv, 1, flow_fc_ooq
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_DIS,
                tcp_rx_dma_serq_stage_start, k.common_phv_qstate_addr,
                TCP_TCB_RX_DMA_OFFSET, TABLE_SIZE_512_BITS)

    nop.e
    nop

flow_fc_ooq:
    // Tables launched from qbase load
    CAPRI_CLEAR_TABLE_VALID(0)

    nop.e
    nop

tcp_cpu_rx:
    CPU_ARQ_SEM_INF_ADDR(d.cpu_id, r3)
    phvwr       p.s6_t1_s2s_cpu_id, d.cpu_id

    CAPRI_NEXT_TABLE_READ(1,
                          TABLE_LOCK_DIS,
                          tcp_rx_write_arq_stage_start,
                          r3,
                          TABLE_SIZE_64_BITS)

    b           flow_fc_process
    nop
