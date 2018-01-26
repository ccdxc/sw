#include "app_redir_common.h"

struct phv_                     p;
struct rawc_pkt_post_k          k;
struct rawc_pkt_post_pkt_post_d d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers
 * so ensure program stage ends after invoking it.
 */
#define r_cpu_flags                 r3
#define r_qstate_addr               r4

%%

    .param      rawc_s4_cleanup_discard
    .param      rawc_normal_stats_inc
    
    .align

/*
 * This is the continuation of rawc_s2_pkt_txdma_prep. We indicate the acquired
 * (original) src_lif in cap_phv_intr_global_t as the last step for the DMA
 * to P4.
 */
rawc_s3_pkt_txdma_post:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * Save cpu_to_p4plus_header_t flags which tell us how to free the AOL
     * pages in the current descriptor, that is, if we need to do so
     * as indicated in k.to_s3_do_cleanup_discard.
     */
    add         r_cpu_flags, r0, d.{flags}.hx
    smeqh       c1, r_cpu_flags, PEN_APP_REDIR_A0_RNMPR_SMALL,  \
                                 PEN_APP_REDIR_A0_RNMPR_SMALL
    phvwri.c1   p.t1_s2s_aol_A0_small, TRUE
    smeqh       c1, r_cpu_flags, PEN_APP_REDIR_A1_RNMPR_SMALL,  \
                                 PEN_APP_REDIR_A1_RNMPR_SMALL
    phvwri.c1   p.t1_s2s_aol_A1_small, TRUE
    smeqh       c1, r_cpu_flags, PEN_APP_REDIR_A2_RNMPR_SMALL,  \
                                 PEN_APP_REDIR_A2_RNMPR_SMALL
    phvwri.c1   p.t1_s2s_aol_A2_small, TRUE

_cleanup_discard_check:
    
    sne         c1, k.common_phv_do_cleanup_discard, r0
    bcf         [c1], _cleanup_discard_prep
    
    /*
     * Gather packet chain statistics
     */
    RAWCCB_NORMAL_STAT_INC_LAUNCH(3, r_qstate_addr, 
                                  k.{common_phv_qstate_addr_sbit0_ebit5...\
                                     common_phv_qstate_addr_sbit30_ebit33},
                                  p.t3_s2s_inc_stat_pkts_chain)
    /*
     * If discard was not taken, we are done if service chaining was in effect
     */
    sne         c1, k.common_phv_next_service_chain_action, r0
    nop.c1.e
    
    /*
     * Note that DMA mem2pkt command descriptors (including the necessary EOP
     * descriptor) have been set up by rawc_s2_pkt_txdma_prep prior to coming here.
     */
    phvwr       p.p4_intr_global_lif, d.{src_lif}.hx    // delay slot
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_intr_global_dma_cmd_type) / 16
    phvwri      p.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri      p.p4_intr_global_tm_oport, TM_PORT_INGRESS
    
    /*
     * Set up DMA of cap_phv_intr_global_t header to packet domain
     */
    phvwri      p.dma_intr_global_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    phvwri      p.dma_intr_global_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)
    phvwri      p.dma_intr_global_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    
    /*
     * Set up DMA of cap_phv_intr_txdma_t header to packet domain
     */
    phvwri      p.dma_intr_txdma_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid)
    phvwri.e    p.dma_intr_txdma_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv)
    phvwri      p.dma_intr_txdma_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT // delay slot


/*
 * Prep for cleaning up and discarding desc/pages.
 */         
_cleanup_discard_prep:

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, rawc_s4_cleanup_discard)
    nop.e
    nop

    
    .align
        
/*
 * Entered as a stage transition when cpu_to_p4plus_header_t flags
 * were not available to be read.
 */         
rawc_s3_cpu_flags_not_read:

    CAPRI_CLEAR_TABLE1_VALID
    b           _cleanup_discard_check
    nop

    


    
