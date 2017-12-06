#include "app_redir_common.h"

struct phv_                                 p;
struct proxyc_desc_enqueue_k                k;
struct proxyc_desc_enqueue_desc_enqueue_d   d;

#define r_pi                        r1
#define r_ci                        r2
#define r_qentry_addr               r3
#define r_qentry_size_shift         r4
#define r_db_addr_scratch           r5
#define r_db_data_scratch           r6
#define r_desc                      r7

%%

    .param      proxyc_s4_cleanup_discard
    .align
    
/*
 * All meta headers have been stripped and the current packet descriptor
 * can now be enqueued to the next service chain's TxQ.
 */
proxyc_s3_desc_enqueue:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * Ring is full when PI+1 == CI and, if so, we'll set a flag 
     * and exit (handling will be from a subsequent stage).
     */
    add         r_pi, r0, d.{pi_curr}.hx
    add         r_qentry_addr, r0, r_pi
    add         r_ci, r0, d.{ci_curr}.hx
    mincr       r_pi, k.common_phv_chain_txq_ring_size_shift, 1
    mincr       r_ci, k.common_phv_chain_txq_ring_size_shift, r0
    beq         r_pi, r_ci, _txq_ring_full_discard
    mincr       r_qentry_addr, k.common_phv_chain_txq_ring_size_shift, r0 // delay slot
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_chain_dma_cmd_type) / 16
    /*
     * Set up DMA to enqueue descriptor to next service chain.
     */
    add         r_qentry_size_shift, r0, k.common_phv_chain_txq_entry_size_shift
    sllv        r_qentry_addr, r_qentry_addr, r_qentry_size_shift
    add         r_qentry_addr, r_qentry_addr, k.{common_phv_chain_txq_base}.wx
    phvwr       p.dma_chain_dma_cmd_addr, r_qentry_addr

    phvwr       p.chain_txq_desc_addr_descr_addr, k.to_s3_desc
    phvwri      p.dma_chain_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(chain_txq_desc_addr_descr_addr)
    phvwri      p.dma_chain_dma_cmd_phv_end_addr,   \
                CAPRI_PHV_END_OFFSET(chain_txq_desc_addr_descr_addr)
    phvwri      p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    /*
     * Set up DMA to increment PI and ring doorbell
     */
    APP_REDIR_SETUP_DB_ADDR(DB_ADDR_BASE,
                            DB_INC_PINDEX,
                            DB_SCHED_WR_EVAL_RING,
                            k.{common_phv_chain_txq_lif_sbit0_ebit7...\
                               common_phv_chain_txq_lif_sbit8_ebit10},
                            k.common_phv_chain_txq_qtype,
                            r_db_addr_scratch);
    APP_REDIR_SETUP_DB_DATA(k.{common_phv_chain_txq_qid_sbit0_ebit1...\
                               common_phv_chain_txq_qid_sbit18_ebit23},
                            k.{common_phv_chain_txq_ring_sbit0_ebit1...\
                               common_phv_chain_txq_ring_sbit2_ebit2},
                            r0, // current PI is actually dontcare for DB_INC_PINDEX
                            r_db_data_scratch)
                        
    phvwr       p.chain_txq_db_data_data, r_db_data_scratch
    phvwr       p.dma_doorbell_dma_cmd_addr, r_db_addr_scratch
    phvwri      p.dma_doorbell_dma_cmd_phv_start_addr,  \
                CAPRI_PHV_START_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_phv_end_addr,    \
                CAPRI_PHV_END_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    
    CAPRI_DMA_CMD_STOP_FENCE(dma_doorbell_dma_cmd)
                                
    nop.e
    nop

    .align

/*
 * Prep for cleaning up and discarding desc/pages:
 * re-read the page addresses from the descriptor
 */
proxyc_s3_cleanup_discard_prep:

    add         r_desc, k.{to_s3_desc}.dx, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          proxyc_s4_cleanup_discard,
                          r_desc,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop


/*
 * TxQ ring full discard
 */                       
_txq_ring_full_discard:

    /*
     * TODO: add stats here
     */
    b           proxyc_s3_cleanup_discard_prep
    phvwri      p.common_phv_do_cleanup_discard, TRUE   // delay slot
