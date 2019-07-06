#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_nvme_req_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_nvme_req_rx_k_ k;
struct s2_t0_nvme_req_rx_pdu_hdr_process_d d;

#define IN_P                t0_s2s_rqe_to_pdu_hdr_info
#define IN_TO_S_P           to_s2_info
#define TO_S_WB_P           to_s5_info
#define K_SEG_OFFSET        CAPRI_KEY_FIELD(IN_P, segment_offset)
#define K_PDU_OFFSET        CAPRI_KEY_FIELD(IN_P, pdu_offset)
#define K_RQE_LEN           CAPRI_KEY_FIELD(IN_P, rqe_len)
#define K_SESSION_ID        CAPRI_KEY_FIELD(IN_TO_S_P, session_id)

#define CURR_PDU_LEN        r1
#define PAGE_REM_LEN        r1
#define PLEN_S_OFFSET       r2
#define PLEN_REM_LEN        r3
#define PDU_REM_LEN         r5
#define DMA_CMD_BASE        r7
#define HDR_PLEN_S_OFFSET   4
#define HDR_PLEN_E_OFFSET   8
#define HDR_PLEN_LEN        4
#define PDU_TYPE_RESP_CAP   5
#define PDU_TYPE_C2H_DATA   7
#define RX_HDR_LEN          28

%%
    .param  nvme_rxsessprodcb_base
    .param  nvme_req_rx_sessprodcb_process
    .param  nvme_req_rx_rqcb_writeback_process

.align
nvme_req_rx_pdu_hdr_process:

    /* This is a page with any number of PDU’s, including a partial PDU
       If we have already received the PDU header completely, we make sure we have PLEN from the header. 
       We received this either completely in one pass, or in multiple passes. 
       If we received it in multiple passes, we need to construct PLEN here. 

       If PLEN > 0, we are processing C2H data, so load c2h_data_process
       Else, we are processing a capsule response, so load sess_prod_rx

       If we have NOT received the PDU header completely, we copy the PLEN specific bytes 
       in the page into the corresponding offset in rqcb, and eventually load writeback
     */

    mfspr           r1, spr_mpuid
    seq             c1, r1[4:2], STAGE_2
    bcf             [!c1], bubble_to_next_stage
    seq             c2, CAPRI_KEY_FIELD(IN_TO_S_P, resourcecb_empty), 1 // BD Slot

    bcf             [c2], resourcecb_empty
    sub             CURR_PDU_LEN, K_RQE_LEN, K_SEG_OFFSET // BD Slot
    add             CURR_PDU_LEN, CURR_PDU_LEN, K_PDU_OFFSET

    /*
        pdu_len = len - seg_offset + pdu_offset;
        if (pdu_len > 4) {

            if (pdu_offset <= 4) {
                plen_offset = 4 - pdu_offset;
                rem_len = plen_size;
            }
            else if (pdu_offset > 4 && pdu_offset < 8) {
                plen_offset = 0;
                rem_len = 8 - pdu_offset;
            }

            rem_len = ((pdu_len < rem_len) ? pdu_len : rem_len);
        }

    sle             c3, CURR_PDU_LEN, HDR_PLEN_S_OFFSET
    bcf             [c3], partial_hdr

    sle             c3, K_PDU_OFFSET, HDR_PLEN_S_OFFSET // BD Slot
    bcf             [c3], plen_start

    slt             c3, K_PDU_OFFSET, HDR_PLEN_E_OFFSET // BD Slot
    bcf             [!c3], plen_done
    add             PLEN_S_OFFSET, r0, r0 // BD Slot

    sub             PLEN_REM_LEN, HDR_PLEN_E_OFFSET, K_PDU_OFFSET
    b               update_len
    nop // BD Slot

plen_start:
    sub             PLEN_S_OFFSET, HDR_PLEN_S_OFFSET, K_PDU_OFFSET
    add             PLEN_REM_LEN, r0, HDR_PLEN_LEN

update_len:
    slt             c3, CURR_PDU_LEN, PLEN_REM_LEN
    add.c3          PLEN_REM_LEN, r0, CURR_PDU_LEN
    // PLEN_S_OFFSET: PLEN's start offset in this page,pdu
    // PLEN_REM_LEN: number of bytes of PLEN

    // TODO copy the relevant bytes of PLEN into rqcb
     */

plen_done:

    // read the first 28 bytes of the page
    tblrdp          r6, r0, 0, RX_HDR_LEN
    add             r1, r0, offsetof(struct phv_, pdu_ctxt1_pdu_type)
    phvwrp          r1, 0, RX_HDR_LEN, r6

    slt             c3, CURR_PDU_LEN, RX_HDR_LEN
    bcf             [c3], partial_hdr
    
    addui           r6, r0, hiword(nvme_rxsessprodcb_base) // BD Slot
    addi            r6, r6, loword(nvme_rxsessprodcb_base)

    add             r6, r6, K_SESSION_ID, LOG_RXSESSPRODCB_SIZE

    // pdu_rem_len = 28 - pdu_offset
    sub             PDU_REM_LEN, RX_HDR_LEN, K_PDU_OFFSET
    phvwrpair       CAPRI_PHV_FIELD(TO_S_WB_P, dma_len), PDU_REM_LEN, \
                    CAPRI_PHV_FIELD(TO_S_WB_P, incr_refcnt), 1

    // page_rem_len = rqe_len - seg_offset (remaining bytes in page, starting from this PDU)
    sub             PAGE_REM_LEN, K_RQE_LEN, K_SEG_OFFSET
    slt             c2, PDU_REM_LEN, PAGE_REM_LEN
    CAPRI_SET_FIELD2_C(TO_S_WB_P, more_pdus, 1, c2)

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_0_BITS,
                                nvme_req_rx_sessprodcb_process,
                                r6)

partial_hdr:

    // page_rem_len = rqe_len - seg_offset (remaining bytes in page, starting from this PDU)
    sub             PAGE_REM_LEN, K_RQE_LEN, K_SEG_OFFSET

    phvwrpair       CAPRI_PHV_FIELD(TO_S_WB_P, dma_len), PAGE_REM_LEN, \
                    CAPRI_PHV_FIELD(TO_S_WB_P, partial_hdr), 1                
    CAPRI_SET_FIELD2(TO_S_WB_P, incr_refcnt, 1)

load_writeback:
    add             r6, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_0_BITS,
                                nvme_req_rx_rqcb_writeback_process,
                                r6) // Exit Slot

process_c2h_data:
    nop.e
    CAPRI_SET_TABLE_0_VALID(0) // Exit Slot

resourcecb_empty:
    b               load_writeback
    CAPRI_SET_FIELD2(TO_S_WB_P, resourcecb_empty, 1) // BD Slot

bubble_to_next_stage:
    seq             c1, r1[4:2], STAGE_1
    bcf             [!c1], exit
    nop // BD Slot

    nop.e
    CAPRI_NEXT_TABLE0_SET_SIZE(CAPRI_TABLE_LOCK_DIS,
                               CAPRI_TABLE_SIZE_512_BITS) // Exit Slot

exit:
    nop.e
    nop // Exit Slot
