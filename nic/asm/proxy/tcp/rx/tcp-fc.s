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


flow_fc_process_done:
    phvwr       p.rx2tx_extra_rcv_wnd, d.rcv_wnd
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

    b           flow_fc_process_done
    nop
