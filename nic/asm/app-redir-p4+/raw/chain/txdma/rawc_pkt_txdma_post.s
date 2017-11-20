#include "app_redir_common.h"

struct phv_                                 p;
struct rawc_meta_header_k                   k;
struct rawc_meta_header_pkt_txdma_post_d    d;

%%
    .align

/*
 * This is the continuation of rawc_s2_pkt_txdma_prep. We indicate the acquired
 * (original) src_lif in cap_phv_intr_global_t as the last step for the DMA
 * to P4.
 */
rawc_s3_pkt_txdma_post:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * For service chaining, simply check its result and take any necessary
     * cleanup action.
     */
    sne         c1, k.common_phv_next_service_chain_action, r0
    bcf         [c1], service_chain_result_check
    nop
    
    /*
     * Note that DMA mem2pkt command descriptors (including the necessary EOP
     * descriptor) have been set up by rawc_s2_pkt_txdma_prep prior to coming here.
     */
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(dma_intr_global_dma_cmd_type) / 16
    phvwri      p.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri      p.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwri      p.p4_intr_global_tm_oq, 0
    phvwr       p.p4_intr_global_lif, d.{src_lif}.hx
    
    /*
     * Set up DMA of cap_phv_intr_global_t header to packet domain
     */
    phvwri      p.dma_intr_global_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    phvwri      p.dma_intr_global_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)
    phvwri      p.dma_intr_global_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    
    /*
     * Set up DMA of cap_phv_intr_txdma_t header to packet domain
     */
    phvwri      p.dma_intr_txdma_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid)
    phvwri      p.dma_intr_txdma_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv)
    phvwri      p.dma_intr_txdma_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT

sched_pages_free:

    /*
     * TODO: schedule freeing of desc and pages
     */
     
    nop.e
    nop
    
service_chain_result_check:

    /*
     * Check that service chaining did not encounter a ring full. 
     */

    seq         c1, k.common_phv_chain_txq_ring_full, r0
    nop.c1.e
    bcf         [!c1], sched_pages_free
    nop
        
