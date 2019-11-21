#include "app_redir_common.h"

struct phv_                     p;
struct s6_tbl_k                 k;
struct s6_tbl_ppage_free_d      d;

/*
 * Registers usage
 */
#define r_ascq_entry                r3

%%
    
    .align

rawr_ppage_free:

    CAPRI_CLEAR_TABLE0_VALID
    sne         c1, RAWR_KIVEC0_PKT_FREEQ_NOT_CFG, r0
    nop.c1.e
    RAWR_METRICS_SET_c(c1, pkt_free_errors)
    
    /*
     * ASCQ should never be full,
     */
    sne         c2, d.ascq_full, r0
    nop.c2.e
    RAWR_METRICS_SET_c(c2, pkt_free_errors)            // delay slot

    CPU_TX_ASCQ_ENQUEUE(r_ascq_entry,
                        RAWR_KIVEC1_PPAGE,
                        d.{ascq_pindex}.wx,
                        RAWR_KIVEC3_ASCQ_BASE,
                        ring_entry_descr_addr,
                        dma_desc_dma_cmd,
                        TRUE,   // eop
                        TRUE)   // fence
                       
    phvwri.e    p.p4_rxdma_intr_dma_cmd_ptr,\
                CAPRI_PHV_START_OFFSET(dma_desc_dma_cmd_type) / 16
    nop

