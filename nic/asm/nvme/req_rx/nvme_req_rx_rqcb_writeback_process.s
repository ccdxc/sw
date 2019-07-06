#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_nvme_req_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

#define IN_TO_S_P       to_s5_info
#define OUT_P           t0_s2s_rqcb_writeback_to_incr_refcnt_info
#define DMA_CMD_BASE    r4
#define PDU_ID          r5
#define PDU_CTXT_P      r6
#define TMP_R           r7

struct phv_ p;
struct s5_t0_nvme_req_rx_k_ k;
struct s5_t0_nvme_req_rx_rqcb_writeback_process_d d;

%%

    .param  nvme_rx_pdu_context_base
    .param  nvme_req_rx_incr_refcnt_process

.align
nvme_req_rx_rqcb_writeback_process:

    mfspr           r1, spr_mpuid
    seq             c1, r1[4:2], STAGE_5
    bcf             [!c1], bubble_to_next_stage
    crestore        [c6, c5], CAPRI_KEY_RANGE(IN_TO_S_P, resourcecb_empty, sess_prod_dgst_full), 0x3 // BD Slot

    // this instruction helps avoid a stall
    // conditional flags used in crestore should not be used immediately
    CAPRI_SET_TABLE_0_VALID(0)

    // if resourcecb_empty, drop phv immediately
    bcf             [c6], drop_phv
    tblmincri       d.wb_r0_busy, 1, 1 // BD Slot

    // update pduid before checking if sess_prod_dgst_full
    seq             c1, d.resource_alloc_done, 0
    cmov            PDU_ID, c1, CAPRI_KEY_FIELD(IN_TO_S_P, pduid), d.pduid
    tblwr.c1        d.pduid, CAPRI_KEY_FIELD(IN_TO_S_P, pduid)
    tblwr.c1        d.resource_alloc_done, 1

    // if sess_prod_dgst_full, drop phv
    bcf             [c5], drop_phv
    // DMA commands for pdu_ctxt1_dma
    addui           PDU_CTXT_P, r0, hiword(nvme_rx_pdu_context_base) // BD Slot
    addi            PDU_CTXT_P, PDU_CTXT_P, loword(nvme_rx_pdu_context_base)
    add             PDU_CTXT_P, PDU_CTXT_P, PDU_ID, LOG_PDU_CTXT_SIZE
    add             PDU_CTXT_P, PDU_CTXT_P, d.pdu_offset

    DMA_CMD_BASE_GET(DMA_CMD_BASE, pdu_ctxt1_dma)
    DMA_HBM_PHV2MEM_START_LEN_SETUP(DMA_CMD_BASE, TMP_R, pdu_ctxt1_pdu_type, CAPRI_KEY_FIELD(IN_TO_S_P, dma_len), PDU_CTXT_P)

    // TODO DMA commands for page_ptr

    bbeq            CAPRI_KEY_FIELD(IN_TO_S_P, incr_refcnt), 0, skip_incr_refcnt
    CAPRI_SET_FIELD2(OUT_P, more_pdus, CAPRI_KEY_FIELD(IN_TO_S_P, more_pdus)) // BD Slot

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN,
                              CAPRI_TABLE_SIZE_512_BITS,
                              nvme_req_rx_incr_refcnt_process,
                              CAPRI_KEY_FIELD(IN_TO_S_P, page_ptr))

skip_incr_refcnt:
    bbeq            CAPRI_KEY_FIELD(IN_TO_S_P, partial_hdr), 0, header_done
    nop // BD Slot

    // update pdu_offset
    tbladd          d.pdu_offset, CAPRI_KEY_FIELD(IN_TO_S_P, dma_len)
    // set seg_offset to 0
    tblwr           d.segment_offset, 0
    // incr c_index
    tblmincri       RQ_C_INDEX, d.log_num_entries, 1

    // TODO tblwr curr_PLEN

    nop.e
    nop // Exit Slot

header_done:
    seq             c2, CAPRI_KEY_FIELD(IN_TO_S_P, more_pdus), 1

    // update seg_offset
    tbladd.c2       d.segment_offset, CAPRI_KEY_FIELD(IN_TO_S_P, dma_len)
    tblwr.!c2       d.segment_offset, 0
    // update c_index
    tblmincri.!c2   RQ_C_INDEX, d.log_num_entries, 1
    // reset curr_PLEN
    tblwr           d.{pdu_offset...curr_plen}, 0
    // update pdu_id
    tblwr           d.pduid, 0
    // update resource_alloc_done
    tblwr           d.resource_alloc_done, 0

    nop.e
    nop // Exit Slot

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_4
    bcf         [!c1], exit
    nop         //BD Slot

    nop.e
    CAPRI_NEXT_TABLE0_SET_SIZE(CAPRI_TABLE_LOCK_EN,
                               CAPRI_TABLE_SIZE_512_BITS) // Exit Slot

drop_phv:
    phvwr.e     p.p4_intr_global_drop, 1
    nop // Exit Slot

exit:
    nop.e
    nop // Exit Slot
