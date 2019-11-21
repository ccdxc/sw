#include "app_redir_common.h"

struct phv_                     p;
struct s2_tbl1_k                k;
struct s2_tbl1_desc_enqueue_d   d;

/*
 * Registers usage
 */
#define r_pi                        r1
#define r_ci                        r2
#define r_qentry_addr               r3
#define r_rawccb_flags              r4
#define r_db_addr_scratch           r5
#define r_db_data_scratch           r6
#define r_desc                      r7

/*
 * Register reuse
 */

%%

    .align
    
/*
 * This is one of 2 mutually exclusive paths taken for sending the
 * incoming packet data to the next service chain. In this case, a
 * next service chain TxQ had been configured to which we will enqueue
 * the given packet descriptor.
 */
rawc_chain_txq_desc_enqueue:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * Ring is full when PI+1 == CI and, if so, we'll set a flag 
     * and exit (handling will be from a subsequent stage).
     */
    add         r_pi, r0, d.{pi_curr}.hx
    add         r_qentry_addr, r0, r_pi
    add         r_ci, r0, d.{ci_curr}.hx
    mincr       r_pi, RAWC_KIVEC0_CHAIN_TXQ_RING_SIZE_SHIFT, 1
    mincr       r_ci, RAWC_KIVEC0_CHAIN_TXQ_RING_SIZE_SHIFT, r0
    beq         r_pi, r_ci, _txq_ring_full_discard
    mincr       r_qentry_addr, RAWC_KIVEC0_CHAIN_TXQ_RING_SIZE_SHIFT, r0   // delay slot
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_chain_dma_cmd_type) / 16
    /*
     * Set up DMA to enqueue descriptor to next service chain.
     * NOTE: everything is enqueued as is, which means all meta headers will
     * still be available for the next service, including cpu_to_p4plus_header_t,
     * p4plus_to_p4_header_t, and L7 header.
     */
    sllv        r_qentry_addr, r_qentry_addr, RAWC_KIVEC0_CHAIN_TXQ_ENTRY_SIZE_SHIFT
    add         r_qentry_addr, r_qentry_addr, RAWC_KIVEC2_CHAIN_TXQ_BASE
    phvwrpair   p.dma_chain_dma_cmd_addr, r_qentry_addr, \
                p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    /*
     * Service chain's queue may be expecting to get a desc that has already
     * been adjusted to point to the beginning of the AOL area.
     */
    add         r_desc, RAWC_KIVEC3_DESC, r0
    add         r_rawccb_flags, RAWC_KIVEC0_RAWCCB_FLAGS, r0
    smeqh       c1, r_rawccb_flags, APP_REDIR_CHAIN_DESC_ADD_AOL_OFFSET, \
                                    APP_REDIR_CHAIN_DESC_ADD_AOL_OFFSET
    add.c1      r_desc, r_desc, NIC_DESC_ENTRY_0_OFFSET
    phvwr       p.ring_entry_descr_addr, r_desc
    phvwrpair   p.dma_chain_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(ring_entry_descr_addr), \
                p.dma_chain_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)

    /*
     * Set up DMA to increment PI and ring doorbell
     */
    APP_REDIR_SETUP_DB_ADDR(DB_ADDR_BASE,
                            DB_INC_PINDEX,
                            DB_SCHED_WR_EVAL_RING,
                            RAWC_KIVEC2_CHAIN_TXQ_LIF,
                            RAWC_KIVEC2_CHAIN_TXQ_QTYPE,
                            r_db_addr_scratch);
    APP_REDIR_SETUP_DB_DATA(RAWC_KIVEC2_CHAIN_TXQ_QID,
                            RAWC_KIVEC2_CHAIN_TXQ_RING,
                            r0, // current PI is actually dontcare for DB_INC_PINDEX
                            r_db_data_scratch)
                        
    phvwr       p.chain_txq_db_data_data, r_db_data_scratch.dx
    phvwrpair   p.dma_doorbell_dma_cmd_addr, r_db_addr_scratch, \
                p.dma_doorbell_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwrpair.e p.dma_doorbell_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(chain_txq_db_data_data), \
                p.dma_doorbell_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(chain_txq_db_data_data)
    phvwrpair   p.dma_doorbell_dma_cmd_wr_fence, TRUE, \
                p.dma_doorbell_dma_cmd_eop, TRUE        // delay slot
                

/*
 * TxQ ring full discard
 */                       
_txq_ring_full_discard:

    /*
     * Cleanup discard will be launched from a different stage
     */
    phvwri.e    p.rawc_kivec0_do_cleanup_discard, TRUE
    RAWC_METRICS_SET(txq_full_discards)                 // delay slot
