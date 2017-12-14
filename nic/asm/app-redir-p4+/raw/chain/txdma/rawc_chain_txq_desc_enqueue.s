#include "app_redir_common.h"

struct phv_                             p;
struct rawc_desc_enqueue_k              k;
struct rawc_desc_enqueue_desc_enqueue_d d;

/*
 * Registers usage
 */
#define r_pi                        r1
#define r_ci                        r2
#define r_qentry_addr               r3
#define r_qentry_size_shift         r4
#define r_db_addr_scratch           r5
#define r_db_data_scratch           r6
#define r_desc                      r7

/*
 * Register reuse
 */
#define r_rawccb_flags              r_qentry_addr

%%

    .align
    
/*
 * This is one of 2 mutually exclusive paths taken for sending the
 * incoming packet data to the next service chain. In this case, a
 * next service chain TxQ had been configured to which we will enqueue
 * the given packet descriptor.
 */
rawc_s2_chain_txq_desc_enqueue:

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
    mincr       r_qentry_addr, k.common_phv_chain_txq_ring_size_shift, r0   // delay slot
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_chain_dma_cmd_type) / 16
    /*
     * Set up DMA to enqueue descriptor to next service chain.
     * NOTE: everything is enqueued as is, which means all meta headers will
     * still be available for the next service, including cpu_to_p4plus_header_t,
     * p4plus_to_p4_header_t, and L7 header.
     */
    add         r_qentry_size_shift, r0, k.common_phv_chain_txq_entry_size_shift
    sllv        r_qentry_addr, r_qentry_addr, r_qentry_size_shift
    add         r_qentry_addr, r_qentry_addr, k.common_phv_chain_txq_base
    phvwr       p.dma_chain_dma_cmd_addr, r_qentry_addr

    /*
     * Service chain's queue may be expecting to get a desc that has already
     * been adjusted to point to the beginning of the AOL area.
     */
    add         r_rawccb_flags, r0, k.common_phv_rawccb_flags
    smeqh       c1, r_rawccb_flags, APP_REDIR_CHAIN_DESC_ADD_AOL_OFFSET,\
                                    APP_REDIR_CHAIN_DESC_ADD_AOL_OFFSET
    add.c1      r_desc, k.t0_s2s_desc, NIC_DESC_ENTRY_0_OFFSET
    add.!c1     r_desc, k.t0_s2s_desc, r0
    phvwr       p.chain_txq_desc_addr_descr_addr, r_desc
    phvwri      p.dma_chain_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(chain_txq_desc_addr_descr_addr)
    phvwri      p.dma_chain_dma_cmd_phv_end_addr, \
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
                        
    phvwr       p.chain_txq_db_data_data, r_db_data_scratch.dx
    phvwr       p.dma_doorbell_dma_cmd_addr, r_db_addr_scratch
    phvwri      p.dma_doorbell_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM;
    
    CAPRI_DMA_CMD_STOP_FENCE(dma_doorbell_dma_cmd)
                                
    nop.e
    nop


/*
 * TxQ ring full discard
 */                       
_txq_ring_full_discard:

    /*
     * TODO: add stats here
     */
    phvwri.e    p.common_phv_do_cleanup_discard, TRUE
    nop
