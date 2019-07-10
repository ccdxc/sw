#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s3_t0_nvme_sessrsrcfree_rx_k_ k;
struct s3_t0_nvme_sessrsrcfree_rx_decr_refcnt_first_page_process_d d;

#define DMA_CMD_BASE r7

#define PAGE_METADATA_RF_REFCNT_OFFSET \
    FIELD_OFFSET(s3_t0_nvme_sessrsrcfree_rx_decr_refcnt_first_page_process_d, rf_refcnt)

%%
    .param  nvme_nmdpr_resourcecb_addr
    .param  nvme_sessrsrcfree_rx_nmdpr_resourcecb_process

.align
nvme_sessrsrcfree_rx_decr_refcnt_first_page_process:

        // Increment rf_refcnt and DMA it to page-metadata.
        add             r1, d.rf_refcnt, r0
        mincr           r1, 16, 1

        phvwr           p.first_page_refcnt_index, r1
        mfspr           r2, spr_tbladdr
        add             r2, r2, PAGE_METADATA_RF_REFCNT_OFFSET
        DMA_CMD_BASE_GET(DMA_CMD_BASE, first_page_refcnt_dma)
        DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, first_page_refcnt_index, first_page_refcnt_index, r2)


        // set free-first-page in to-stage if refcnt=0 && more_pdus=0
        bbeq            d.more_pdus, 1, skip_free_page
        seq             c2, d.rq_refcnt, r1  //BD-slot
                
        phvwr.c2        p.to_s4_info_free_first_page, 1
        phvwr.c2        p.to_s6_info_free_first_page, 1    

skip_free_page:
        // load nmdpr_resourcecb
        addui           r6, r0, hiword(nvme_nmdpr_resourcecb_addr) 
        addi            r6, r6, loword(nvme_nmdpr_resourcecb_addr)

        CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                    CAPRI_TABLE_SIZE_512_BITS,
                                    nvme_sessrsrcfree_rx_nmdpr_resourcecb_process,
                                    r6)
