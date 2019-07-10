#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t1_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s3_t1_nvme_sessrsrcfree_rx_k_ k;
struct s3_t1_nvme_sessrsrcfree_rx_decr_refcnt_last_page_process_d d;

#define DMA_CMD_BASE r7
#define PAGE_METADATA_RF_REFCNT_OFFSET \
    FIELD_OFFSET(s3_t1_nvme_sessrsrcfree_rx_decr_refcnt_last_page_process_d, rf_refcnt)

%%

.align
nvme_sessrsrcfree_rx_decr_refcnt_last_page_process:

        // DMA command to increment rf_refcnt
        add             r1, d.rf_refcnt, r0
        mincr           r1, 16, 1

        phvwr           p.last_page_refcnt_index, r1
        mfspr           r2, spr_tbladdr
        add             r2, r2, PAGE_METADATA_RF_REFCNT_OFFSET
        DMA_CMD_BASE_GET(DMA_CMD_BASE, last_page_refcnt_dma)
        DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, last_page_refcnt_index, last_page_refcnt_index, r2)

        // set free-first-page in to-stage if refcnt=0 && more_pdus=0
        bbeq            d.more_pdus, 1, skip_free_page
        seq             c2, d.rq_refcnt, r1  //BD-slot
                
        phvwr.c2        p.to_s4_info_free_last_page, 1
        phvwr.c2        p.to_s6_info_free_last_page, 1    

skip_free_page:
        phvwr.e         p.app_header_table1_valid, 0
        nop //Exit-slot
