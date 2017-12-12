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

struct phv_ p;
struct s4_t1_tcp_tx_k k;
struct s4_t1_tcp_tx_read_nmdr_gc_pi_d d;

%%
    .align
    .param          TNMDR_GC_TABLE_BASE
    .param          RNMDR_GC_TABLE_BASE

tcp_tx_read_nmdr_gc_idx_start:
    CAPRI_CLEAR_TABLE_VALID(1)

    /*
     * We need to free k.t1_s2s_free_desc_addr. Store the address to be DMAed
     * into the GC ring later below
     */
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(ringentry_dma_dma_cmd_type) / 16
    phvwr           p.ring_entry_descr_addr, k.t1_s2s_free_desc_addr

    /*
     * r2 = GC PI
     */
    and             r2, d.{nmdr_gc_pi}.wx, CAPRI_HBM_GC_PER_PRODUCER_RING_MASK

    seq             c1, k.common_phv_debug_dol_bypass_barco, 1
    /*
     * TODO : handle freeing of descriptors from CPU
     */
    seq             c2, k.common_phv_pending_asesq, 1
    bcf             [c1 & !c2], free_rnmdr
    nop
    bcf             [!c1 & !c2], free_tnmdr
    nop

    nop.e
    nop

/*
 * Pass PI in GC ring to copy address in r2
 */
free_rnmdr:
    addui           r1, r0, hiword(RNMDR_GC_TABLE_BASE)
    addi            r1, r1, loword(RNMDR_GC_TABLE_BASE)
    add             r1, r1, RNMDR_GC_PRODUCER_TCP, RNMDR_GC_PER_PRODUCER_SHIFT
    add             r1, r1, r2, RNMDR_TABLE_ENTRY_SIZE_SHFT
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry_dma_dma_cmd, r1, ring_entry_descr_addr, ring_entry_descr_addr)
    CAPRI_DMA_CMD_RING_DOORBELL2(doorbell_dma_dma_cmd, LIF_GC, 0,
                    CAPRI_HBM_GC_RNMDR_QID, CAPRI_RNMDR_GC_TCP_RING_PRODUCER,
                    0, db_data_pid, db_data_index)
    CAPRI_DMA_CMD_STOP_FENCE(doorbell_dma_dma_cmd)
    nop.e
    nop

/*
 * Pass PI in GC ring to copy address in r2
 */
free_tnmdr:
    addui           r1, r0, hiword(TNMDR_GC_TABLE_BASE)
    addi            r1, r1, loword(TNMDR_GC_TABLE_BASE)
    add             r1, r1, TNMDR_GC_PRODUCER_TCP, TNMDR_GC_PER_PRODUCER_SHIFT
    add             r1, r1, r2, TNMDR_TABLE_ENTRY_SIZE_SHFT
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry_dma_dma_cmd, r1, ring_entry_descr_addr, ring_entry_descr_addr)
    CAPRI_DMA_CMD_RING_DOORBELL2(doorbell_dma_dma_cmd, LIF_GC, 0,
                    CAPRI_HBM_GC_TNMDR_QID, CAPRI_TNMDR_GC_TCP_RING_PRODUCER,
                    0, db_data_pid, db_data_index)
    CAPRI_DMA_CMD_STOP_FENCE(doorbell_dma_dma_cmd)
    nop.e
    nop
