#include "raw_redir_common.h"

struct phv_                                         p;
struct rawc_chain_txq_ring_indices_k                k;
struct rawc_chain_txq_ring_indices_desc_enqueue_d   d;

#define PI_R                        r1
#define CI_R                        r2
#define QENTRY_ADDR_R               r3
#define QENTRY_SIZE_SHIFT_R         r4
#define DB_ADDR_SCRATCH_R           r5
#define DB_DATA_SCRATCH_R           r6

%%

    .align
    
/*
 * This is one of 2 mutually exclusive paths taken for sending the
 * incoming packet data to the next service chain. In this case, a
 * next service chain TxQ had been configured to which we will enqueue
 * the given packet descriptor.
 */
rawc_s2_chain_txq_desc_enqueue:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * Ring is full when PI+1 == CI and, if so, we'll set a flag 
     * and exit (handling will be from a subsequent stage).
     */
    add         PI_R, r0, d.{pi_curr}.hx
    add         CI_R, r0, d.{ci_curr}.hx
    mincr       PI_R, k.common_phv_chain_txq_ring_size_shift, 1
    mincr       CI_R, k.common_phv_chain_txq_ring_size_shift, r0
    seq         c1, PI_R, CI_R
    phvwri.c1.e p.common_phv_chain_txq_ring_full, TRUE
    
    add         QENTRY_ADDR_R, r0, PI_R     // delay slot
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(dma_chain_dma_cmd_type) / 16
        
    /*
     * Set up DMA to enqueue descriptor to next service chain.
     * NOTE: everything is enqueued as is, which means all meta headers will
     * still be available for the next service, including cpu_to_p4_header_t,
     * p4plus_to_p4_header_t, and L7 header.
     */
    add         QENTRY_SIZE_SHIFT_R, r0, k.common_phv_chain_txq_entry_size_shift
    sllv        QENTRY_ADDR_R, QENTRY_ADDR_R, QENTRY_SIZE_SHIFT_R
    add         QENTRY_ADDR_R, QENTRY_ADDR_R, k.{common_phv_chain_txq_base}.wx
    phvwr       p.dma_chain_dma_cmd_addr, QENTRY_ADDR_R

    phvwr       p.chain_txq_desc_addr_descr_addr, k.to_s2_my_txq_desc
    phvwri      p.dma_chain_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(chain_txq_desc_addr_descr_addr)
    phvwri      p.dma_chain_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(chain_txq_desc_addr_descr_addr)
    phvwri      p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    /*
     * Set up DMA to increment PI and ring doorbell
     */
    RAWR_SETUP_DB_ADDR(DB_ADDR_BASE,
                       DB_INC_PINDEX,
                       DB_SCHED_WR_EVAL_RING,
                       k.{common_phv_chain_txq_lif_sbit0_ebit7...\
                          common_phv_chain_txq_lif_sbit8_ebit10},
                       k.common_phv_chain_txq_qtype,
                       DB_ADDR_SCRATCH_R);
    RAWR_SETUP_DB_DATA(k.{common_phv_chain_txq_qid_sbit0_ebit1...\
                          common_phv_chain_txq_qid_sbit18_ebit23},
                       k.{common_phv_chain_txq_ring_sbit0_ebit1...\
                          common_phv_chain_txq_ring_sbit2_ebit2},
                       r0,      // current PI is actually dontcare for DB_INC_PINDEX
                       DB_DATA_SCRATCH_R)
                        
    phvwr       p.chain_txq_db_data_data, DB_DATA_SCRATCH_R
    phvwr       p.dma_doorbell_dma_cmd_addr, DB_ADDR_SCRATCH_R
    phvwri      p.dma_doorbell_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM;
    
    CAPRI_DMA_CMD_STOP_FENCE(dma_doorbell_dma_cmd)
                                
    nop.e
    nop

