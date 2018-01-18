#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_rrqsge_process_k_t k;

#define RRQSGE_TO_LKEY_T struct req_rx_rrqsge_to_lkey_info_t
#define SQCB1_WRITE_BACK_T struct req_rx_sqcb1_write_back_info_t

#define LOG_PAGE_SIZE  10

%%
    .param    req_rx_rrqlkey_process
    .param    req_rx_sqcb1_write_back_process

.align
req_rx_rrqsge_process:
    // Use conditional flag to select between sge_index 0 and 1
    // sge_index = 0
    setcf          c7, [c0]

    // sge_p[0]
    //add            r1, HBM_CACHE_LINE_SIZE_BITS, r0
    // Data structures are accessed from bottom to top in big-endian, hence go to
    // the bottom of the SGE_T
    // big-endian
    add            r1, r0, (HBM_NUM_SGES_PER_CACHELINE - 1), LOG_SIZEOF_SGE_T_BITS

    // r2 = k.args.current_sge_offset
    add            r2, r0, k.args.cur_sge_offset

    // r3 = k.args.remaining_payload_bytes
    add            r3, r0, k.args.remaining_payload_bytes

    // r5 = num_pages = 0
    add            r5, r0, r0

sge_loop:
    // sge_remaining_bytes = sge_p->len - current_sge_offset
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, len)
    sub            r4, r4, r2

    // transfer_bytes = min(sge_remaining_bytes, remaining_payload_bytes)
    slt            c1, r4, r3
    cmov           r4, c1, r4, r3

    // transfer_va = sge_p->va + current_sge_offset
    CAPRI_TABLE_GET_FIELD(r6, r1, SGE_T, va)
    add            r6, r6, r2

    // Get common.common_t[0]_s2s or common.common_t[1]_s2s... args based on sge_index
    // to invoke programs in multiple MPUs
    CAPRI_GET_TABLE_0_OR_1_ARG(req_rx_phv_t, r7, c7)

    // Fill stage 2 stage data in req_tx_sge_lkey_info_t for next stage
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, sge_va, r6)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, sge_bytes, r4)
    add            r5, r5, k.args.dma_cmd_start_index
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, dma_cmd_start_index, r5)

    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, log_page_size, LOG_PAGE_SIZE) // TODO page_size ???
    // To start with, set dma_cmd_eop to 0
    //CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, dma_cmd_eop, 0)

    // current_sge_offset += transfer_bytes
    add            r2, r2, r4

    // remaining_payload_bytes -= transfer_bytes
    sub            r3, r3, r4

    // if end of loop, set k[].dma_cmd_eop = TRUE;
    sle            c2, r3, r0
    slt            c3, 1, k.args.num_valid_sges
   
    setcf          c4, [!c2 & c3 & c7]
    // set dma_cmd_eop in last dma cmd for the pkt
    CAPRI_SET_FIELD_C(r7, RRQSGE_TO_LKEY_T, dma_cmd_eop, k.args.dma_cmd_eop, !c4)

    GET_NUM_PAGES(r6, r4, LOG_PAGE_SIZE, r5, r4)

    cmov           r6, c7, 0, 1
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, sge_index, r6)

    // r6 = hbm_addr_get(PHV_GLOBA_KT_BASE_ADDR_GET())
    KT_BASE_ADDR_GET(r6, r4)

    // r4 = sge_p->lkey
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, l_key)

    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey & KEY_INDEX_MASK) * sizeof(key_entry_t));
    KEY_ENTRY_ADDR_GET(r6, r6, r4)
    
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, is_atomic, k.args.is_atomic)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, cq_dma_cmd_index, REQ_RX_DMA_CMD_CQ)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, cq_id, k.args.cq_id)

    // r4 = sge_p->len
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, len)

    // if (current_sge_offset == sge_p->len)
    seq            c1, r2, r4

    // Get common.common_te[0]_phv_table_addr or common.common_te[1]_phv_table_Addr ... based on
    // sge_index to invoke program in multiple MPUs
    CAPRI_GET_TABLE_0_OR_1_K(req_rx_phv_t, r7, c7)
    // aligned_key_addr and key_id sent to next stage to load lkey
    CAPRI_NEXT_TABLE_I_READ_PC(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqlkey_process, r6)

    // big-endian - subtract sizeof(sge_t) as sges are read from bottom to top in big-endian format
    // sge_p[1]
    sub.c1         r1, r1, 1, LOG_SIZEOF_SGE_T_BITS

    // current_sge_offset = 0;
    add.c1         r2, r0, r0

    // while((remaining_payload_bytes > 0) &&
    //       (num_valid_sges > 1) &&
    //       (sge_index == 0)
    bcf            [c4], sge_loop
    // sge_index = 1
    setcf.c1       c7, [!c0] // branch delay slot

    srl            r1, r1, LOG_SIZEOF_SGE_T_BITS
    sub            r1, (HBM_NUM_SGES_PER_CACHELINE-1), r1
    seq            c1, r1, k.args.num_valid_sges
    add.!c1        r1, r1, k.args.cur_sge_id

    // TODO Set Phv recirc if remaining_payload_bytes is non-zero (cf - c2 set to False)

    //if (REQ_RX_FLAG_IS_SET(last) || (REQ_RX_FLAG_ONLY(only)))
    add             r7, r0, k.global.flags
    IS_ANY_FLAG_SET(c5, r7, REQ_RX_FLAG_LAST|REQ_RX_FLAG_ONLY)

    bcf            [!c5], set_arg
    // in_progress = FALSE
    cmov            r4, c5, 0, 1 // Branch Delay Slot
    // current_sge_id = 0
    add             r1, r0, r0
    // currrent_sge_offset = 0
    add             r2, r0, r0

    // RING_C_INDEX_INCREMENT(rrq_ring_id) TODO Need to do via DMA and Fence it
    SQCB0_ADDR_GET(r5)
    add            r6, r5, RRQ_C_INDEX_OFFSET
    memwr.hx       r6, k.args.rrq_cindex

set_arg:

    CAPRI_GET_TABLE_2_ARG(req_rx_phv_t, r7)

    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, in_progress, r4)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, cur_sge_id, r1)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, cur_sge_offset, r2)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, e_rsp_psn, k.args.e_rsp_psn)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, tbl_id, 2)

    SQCB1_ADDR_GET(r5)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_sqcb1_write_back_process, r5)
 
    nop.e
    nop
