#include "app_redir_common.h"

struct phv_                     p;
struct s4_tbl1_k                k;
struct s4_tbl1_desc_free_d      d;

/*
 * Registers usage
 */
#define r_ascq_entry                r5
#define r_last_mem2pkt_ptr          r6

%%
    
    .align

rawc_desc_free:

    bbeq        RAWC_KIVEC0_PKT_FREEQ_NOT_CFG, 1, _pkt_not_free
    CAPRI_CLEAR_TABLE1_VALID                    // delay slot
    
    /*
     * NOTE: to get here, the following condition have been been verified:
     *  - service_chain_action was false, i.e., not chaining to a TxQ
     *  - PEN_APP_REDIR_DONT_FREE_DESC flag was false
     */
    sne         c1, d.ascq_full, r0
    bcf         [c1], _handle_cmdeop
    RAWC_METRICS_SET_c(c1, pkt_free_errors)     // delay slot

    CPU_TX_ASCQ_ENQUEUE(r_ascq_entry,
                        RAWC_KIVEC3_DESC,
                        d.{ascq_pindex}.wx,
                        RAWC_KIVEC4_ASCQ_BASE,
                        ring_entry_descr_addr,
                        dma_chain_dma_cmd,
                        TRUE,   // eop
                        TRUE)   // fence
    /*
     * In the case of do_cleanup_discard, the above DMA command
     * should be the only command excuted so we make the
     * corresponding adjustment here.
     */
    sne.e       c2, RAWC_KIVEC0_DO_CLEANUP_DISCARD, r0
    phvwri.c2   p.p4_txdma_intr_dma_cmd_ptr,\
                CAPRI_PHV_START_OFFSET(dma_chain_dma_cmd_type) / 16
    
_pkt_not_free:

    RAWC_METRICS_SET(pkt_free_errors)

_handle_cmdeop:
    
    add         r_last_mem2pkt_ptr, RAWC_KIVEC3_LAST_MEM2PKT_PTR, r0
    sne         c3, r_last_mem2pkt_ptr, r0
    phvwrpi.c3  r_last_mem2pkt_ptr, APP_REDIR_BIT_OFFS_DMA_MEM2PKT(cmdeop), \
                APP_REDIR_BIT_SIZE_DMA_MEM2PKT(cmdeop), TRUE    // delay slot
    /*
     * We could have gotten to this point with RAWC_KIVEC0_PKT_FREEQ_NOT_CFG=1,
     * and if RAWC_KIVEC0_DO_CLEANUP_DISCARD=1, all DMA should be canceled.
     */
    sne.e       c2, RAWC_KIVEC0_DO_CLEANUP_DISCARD, r0
    phvwr.c2    p.p4_txdma_intr_dma_cmd_ptr, r0
                
