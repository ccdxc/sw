/*
 *	Read semaphore to index into GC TCP NMDR ring
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t1_tcp_tx_k.h"

struct phv_ p;
struct s4_t1_tcp_tx_k_ k;
struct s4_t1_tcp_tx_read_nmdr_gc_pi_d d;

%%
    .align
    .param          TNMDR_GC_TABLE_BASE
    .param          RNMDR_GC_TABLE_BASE
    .param          TCP_PROXY_STATS

tcp_tx_read_nmdr_gc_idx_start:
    add             r1, d.sw_pi, k.t1_s2s_num_pkts_freed
    and             r1, r1, ASIC_HBM_GC_PER_PRODUCER_RING_MASK
    seq             c1, r1, d.sw_ci
    b.c1            fatal_error
    /*
     * r2 = GC PI
     */
    add             r2, d.sw_pi, r0
    add             r1, r0, k.t1_s2s_num_pkts_freed
    tblmincr.f      d.sw_pi, ASIC_HBM_GC_PER_PRODUCER_RING_SHIFT, r1
    CAPRI_CLEAR_TABLE_VALID(1)

    phvwri          p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(ringentry_dma_dma_cmd_type) / 16

    seq             c1, k.common_phv_debug_dol_bypass_barco, 1
    /*
     * TODO : handle freeing of descriptors from CPU
     */
    seq             c2, k.common_phv_pending_asesq, 1
    bcf             [c1 & !c2], free_rnmdr
    nop
    bcf             [!c1 & !c2], free_tnmdr
    nop

    phvwri          p.p4_intr_global_drop, 1

    nop.e
    nop

/*
 * Pass PI in GC ring to copy address in r2
 */
free_rnmdr:
    /*
     * r1 = address in GC ring to DMA into
     * r2 = gc pi
     */
    addui           r1, r0, hiword(RNMDR_GC_TABLE_BASE)
    addi            r1, r1, loword(RNMDR_GC_TABLE_BASE)
    add             r1, r1, RNMDR_GC_PRODUCER_TCP, RNMDR_GC_PER_PRODUCER_SHIFT
    add             r1, r1, r2, RNMDR_TABLE_ENTRY_SIZE_SHFT

    // r4 = distance of gc_pi from end of ring
    sub             r4, RNMDR_GC_PER_PRODUCER_SIZE, r2
    slt             c1, r4, k.t1_s2s_num_pkts_freed
    b.c1            dma_free_rnmdr_two_dma_commands

dma_free_rnmdr_one_dma_command:
    // r6 = total len in bytes
    add             r6, r0, k.t1_s2s_num_pkts_freed, 3
    CAPRI_DMA_CMD_PHV2MEM_SETUP_WITH_LEN(ringentry_dma_dma_cmd, r1, ring_entry1_descr_addr, r6)

dma_free_rnmdr_doorbell:
    mincr	    r2, ASIC_HBM_GC_PER_PRODUCER_RING_SHIFT, k.t1_s2s_num_pkts_freed
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(doorbell_dma_dma_cmd, LIF_GC,
                    ASIC_HBM_GC_RNMDR_QTYPE,
                    ASIC_RNMDR_GC_TCP_RING_PRODUCER, 0,
                    r2, db_data_pid, db_data_index)
    CAPRI_DMA_CMD_STOP_FENCE(doorbell_dma_dma_cmd)
    nop.e
    nop

dma_free_rnmdr_two_dma_commands:
    /*
     * Setup DMA command 1
     */
    CAPRI_DMA_CMD_PHV2MEM_SETUP_NO_OFFSETS(ringentry_dma_dma_cmd, r1)
    phvwr           p.ringentry_dma_dma_cmd_phv_start_addr, \
                        CAPRI_PHV_START_OFFSET(ring_entry1_descr_addr)
    add             r6, CAPRI_PHV_START_OFFSET(ring_entry1_descr_addr), r4, 3
    sub             r7, r6, 1
    phvwr           p.ringentry_dma_dma_cmd_phv_end_addr, r7
    phvwr           p.ringentry2_dma_dma_cmd_phv_start_addr, r6

    // r4 is remaining entries
    sub             r4, k.t1_s2s_num_pkts_freed, r4
    add             r7, r7, r4, 3
    phvwr           p.ringentry2_dma_dma_cmd_phv_end_addr, r7

    /*
     * r1 = address in GC ring to DMA into
     */
    addui           r1, r0, hiword(RNMDR_GC_TABLE_BASE)
    addi            r1, r1, loword(RNMDR_GC_TABLE_BASE)
    add             r1, r1, RNMDR_GC_PRODUCER_TCP, RNMDR_GC_PER_PRODUCER_SHIFT

    /*
     * Setup DMA command 2
     */
    CAPRI_DMA_CMD_PHV2MEM_SETUP_NO_OFFSETS(ringentry2_dma_dma_cmd, r1)
    b dma_free_rnmdr_doorbell
    nop

/*
 * Pass PI in GC ring to copy address in r2
 */
free_tnmdr:
    /*
     * r1 = address in GC ring to DMA into
     * r2 = gc pi
     */
    addui           r1, r0, hiword(TNMDR_GC_TABLE_BASE)
    addi            r1, r1, loword(TNMDR_GC_TABLE_BASE)
    add             r1, r1, TNMDR_GC_PRODUCER_TCP, TNMDR_GC_PER_PRODUCER_SHIFT
    add             r1, r1, r2, TNMDR_TABLE_ENTRY_SIZE_SHFT

    // r4 = distance of gc_pi from end of ring
    sub             r4, TNMDR_GC_PER_PRODUCER_SIZE, r2
    slt             c1, r4, k.t1_s2s_num_pkts_freed
    b.c1            dma_free_tnmdr_two_dma_commands

dma_free_tnmdr_one_dma_command:
    // r6 = total len in bytes
    add             r6, r0, k.t1_s2s_num_pkts_freed, 3
    CAPRI_DMA_CMD_PHV2MEM_SETUP_WITH_LEN(ringentry_dma_dma_cmd, r1, ring_entry1_descr_addr, r6)

dma_free_tnmdr_doorbell:
    mincr	    r2, ASIC_HBM_GC_PER_PRODUCER_RING_SHIFT, k.t1_s2s_num_pkts_freed
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(doorbell_dma_dma_cmd, LIF_GC,
                    ASIC_HBM_GC_TNMDR_QTYPE,
                    ASIC_TNMDR_GC_TCP_RING_PRODUCER, 0,
                    r2, db_data_pid, db_data_index)
    CAPRI_DMA_CMD_STOP_FENCE(doorbell_dma_dma_cmd)
    nop.e
    nop

dma_free_tnmdr_two_dma_commands:
    /*
     * Setup DMA command 1
     */
    CAPRI_DMA_CMD_PHV2MEM_SETUP_NO_OFFSETS(ringentry_dma_dma_cmd, r1)
    phvwr           p.ringentry_dma_dma_cmd_phv_start_addr, \
                        CAPRI_PHV_START_OFFSET(ring_entry1_descr_addr)
    add             r6, CAPRI_PHV_START_OFFSET(ring_entry1_descr_addr), r4, 3
    sub             r7, r6, 1
    phvwr           p.ringentry_dma_dma_cmd_phv_end_addr, r7
    phvwr           p.ringentry2_dma_dma_cmd_phv_start_addr, r6

    // r4 is remaining entries
    sub             r4, k.t1_s2s_num_pkts_freed, r4
    add             r7, r7, r4, 3
    phvwr           p.ringentry2_dma_dma_cmd_phv_end_addr, r7

    /*
     * r1 = address in GC ring to DMA into
     */
    addui           r1, r0, hiword(TNMDR_GC_TABLE_BASE)
    addi            r1, r1, loword(TNMDR_GC_TABLE_BASE)
    add             r1, r1, TNMDR_GC_PRODUCER_TCP, TNMDR_GC_PER_PRODUCER_SHIFT

    /*
     * Setup DMA command 2
     */
    CAPRI_DMA_CMD_PHV2MEM_SETUP_NO_OFFSETS(ringentry2_dma_dma_cmd, r1)
    b dma_free_tnmdr_doorbell
    nop

fatal_error:
    addui           r2, r0, hiword(TCP_PROXY_STATS)
    addi            r2, r2, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r2, TCP_PROXY_STATS_GC_FULL, 1)
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    nop.e
    nop

