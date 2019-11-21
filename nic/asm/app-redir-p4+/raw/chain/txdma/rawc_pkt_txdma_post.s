#include "app_redir_common.h"

struct phv_                     p;
struct s3_tbl1_k                k;
struct s3_tbl1_pkt_post_d       d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers
 * so ensure program stage ends after invoking it.
 */
#define r_cpu_flags                 r3
#define r_last_mem2pkt_ptr          r4  // last  DMA mem2pkt pointer set by rawc_pkt_txdma_prep

%%

    .param      rawc_desc_free
    
    .align

/*
 * This is the continuation of rawc_pkt_txdma_prep. We indicate the acquired
 * (original) src_lif in cap_phv_intr_global_t as the last step for the DMA
 * to P4.
 */
rawc_pkt_txdma_post:

    /*
     * Save cpu_to_p4plus_header_t flags which tell us whether to submit the
     * descriptor for freeing...
     */
    add         r_cpu_flags, d.{flags}.hx, r0

_desc_free_prep_check:

    smeqh       c1, r_cpu_flags, PEN_APP_REDIR_DONT_FREE_DESC,  \
                                 PEN_APP_REDIR_DONT_FREE_DESC
    sne         c2, RAWC_KIVEC0_NEXT_SERVICE_CHAIN_ACTION, r0
    sne         c3, RAWC_KIVEC0_DO_CLEANUP_DISCARD, r0
    
    /*
     * Never free the packet descriptor if  PEN_APP_REDIR_DONT_FREE_DESC was set.
     */
    bcf         [c1], _desc_free_skip
    CAPRI_CLEAR_TABLE1_VALID                            // delay slot
    
    bcf         [c3], _desc_free_prep
    nop
    
    /*
     * Skip desc free if desc already successfully enqueued to next service
     */
    bcf         [c2], _desc_free_skip
    nop
    
_desc_free_prep:

    bbeq        RAWC_KIVEC0_PKT_FREEQ_NOT_CFG, 0, _desc_free_launch
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, rawc_desc_free)
    b           _desc_free_skip
    nop

_desc_free_launch:
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          rawc_desc_free,
                          RAWC_KIVEC3_ASCQ_SEM_INF_ADDR,
                          TABLE_SIZE_64_BITS)
_desc_free_skip:

    /*
     * Early exit in the case of do_cleanup_discard
     */
    nop.c3.e
    add         r_last_mem2pkt_ptr, RAWC_KIVEC3_LAST_MEM2PKT_PTR, r0    // delay slot
    
    /*
     * Gather packet chain statistics
     */
    RAWC_METRICS_SET(chain_pkts)
    
    /*
     * Now, exit if packet descriptor already enqueued to next service
     */
    nop.c2.e

    phvwr       p.p4_intr_global_lif, d.{src_lif}.hx    // delay slot
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_intr_global_dma_cmd_type) / 16
    phvwrpair   p.p4_intr_global_tm_iport, TM_PORT_DMA,\
                p.p4_intr_global_tm_oport, TM_PORT_INGRESS
    /*
     * Note that DMA mem2pkt command descriptors (including the necessary pkteop
     * descriptor) have been set up by rawc_pkt_txdma_prep prior to coming here.
     *
     * If the packet descriptor were to be freed, there would be one more DMA
     * command (which would be filled in by rawc_desc_free) so don't set cmdeop if so.
     * NOTE: capri_dma_cmd_mem2pkt_t does not have a wr_fence bit.
     */
    phvwrpi.c1  r_last_mem2pkt_ptr, APP_REDIR_BIT_OFFS_DMA_MEM2PKT(cmdeop), \
                APP_REDIR_BIT_SIZE_DMA_MEM2PKT(cmdeop), TRUE
                
    /*
     * Set up DMA of cap_phv_intr_global_t header to packet domain
     */
    phvwrpair   p.dma_intr_global_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type), \
                p.dma_intr_global_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    phvwri      p.dma_intr_global_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    
    /*
     * Set up DMA of cap_phv_intr_txdma_t header to packet domain
     */
    phvwrpair.e p.dma_intr_txdma_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv), \
                p.dma_intr_txdma_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid)
    phvwri      p.dma_intr_txdma_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT     // delay slot
    
    .align
        
/*
 * Entered as a stage transition when cpu_to_p4plus_header_t flags
 * were not available to be read.
 */         
rawc_cpu_flags_not_read:
    b           _desc_free_prep_check
    add         r_cpu_flags, r0, r0             // delay slot

    


    
