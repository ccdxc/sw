#include "req_rx.h"
#include "sqcb.h"
#include "defines.h"

struct req_rx_phv_t p;
struct rrqwqe_t d;
struct req_rx_s1_t0_k k;

#define RRQWQE_TO_SGE_P t0_s2s_rrqwqe_to_sge_info
#define RRQSGE_TO_LKEY_T struct req_rx_rrqsge_to_lkey_info_t
#define SQCB1_WRITE_BACK_P t2_s2s_sqcb1_write_back_info
#define TO_S4_SQCB1_WRITE_BACK_P to_s4_sqcb1_wb_info

#define IN_P t0_s2s_sqcb1_to_rrqwqe_info
#define IN_TO_S_P to_s1_rrqwqe_info

#define K_CUR_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, cur_sge_offset_sbit0_ebit7, cur_sge_offset_sbit16_ebit31)
#define K_CUR_SGE_ID CAPRI_KEY_FIELD(IN_P, cur_sge_id)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_TO_S_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit13)
#define K_DMA_CMD_START_INDEX CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)
#define K_CQ_ID CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit7, cq_id_sbit16_ebit23)
#define K_LOG_PAGE_SIZE  CAPRI_KEY_RANGE(IN_TO_S_P, log_page_size_sbit0_ebit2, log_page_size_sbit3_ebit4)

#define TO_S1_RECIRC_P to_s1_recirc_info
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, priv_oper_enable)
#define TO_S7_P to_s7_stats_info

%%
    .param    req_rx_rrqlkey_process
    .param    req_rx_rrqlkey_rsvd_lkey_process
    .param    req_rx_sqcb1_write_back_process
    .param    req_rx_rrqsge_process

.align
req_rx_rrqwqe_base_sge_opt_process:
    seq            c1, d.num_sges, 0
    bcf            [c1], insufficient_sges

    // total bytes transferred till now, which is used to determine current_sge_offset
    sll            r2, r1, CAPRI_KEY_FIELD(IN_TO_S_P, log_pmtu) // BD Slot

    seq            c1, d.wqe_format, SQWQE_FORMAT_DEFAULT
    bcf            [c1], default_sge_fmt_opt_process
    // Use conditional flag to select between sge_index 0 and 1
    // sge_index = 0
    setcf          c7, [c0] // Branch Delay Slot

    // sge_len_p[0]
    seq            c1, d.wqe_format, SQWQE_FORMAT_8x4
    add            r6, RRQWQE_SGE_OFFSET_BITS, r0
    // cur_sge_id = 0
    add            r7, r0, r0 // Branch Delay Slot

decode_sge_len_encoding:

    // 8x4 WQE format
    /*
     *    lower 32-byte encoding
     *    0          32          63
     *    -------------------------
     *    |     len   |    len    |
     *    -------------------------
     *    |     len   |    len    |
     *    -------------------------
     *    |     len   |    len    |
     *    -------------------------
     *    |     len   |    len    |
     *    -------------------------
     */

    // 16x2 WQE format
    /*
     *    lower 32-byte encoding
     *    0    16    32    48    63
     *    -------------------------
     *    | len | len | len | len |
     *    -------------------------
     *    | len | len | len | len |
     *    -------------------------
     *    | len | len | len | len |
     *    -------------------------
     *    | len | len | len | len |
     *    -------------------------
     */

    sub.c1         r6, r6, 1, LOG_SIZEOF_WQE_8x4_T_BITS
    sub.!c1        r6, r6, 1, LOG_SIZEOF_WQE_16x2_T_BITS
    CAPRI_TABLE_GET_FIELD_C(r4, r6, WQE_8x4_T, len, c1)
    CAPRI_TABLE_GET_FIELD_C(r4, r6, WQE_16x2_T, len, !c1)
    // if total bytes transferred is within first sge's length, then
    // current_sge_offset is equal to total bytes transferred
    blt            r2, r4, trigger_rrqsge_process
    setcf          c2, [c0] // Branch Delay Slot

    // cur_sge_id += 1
    add            r7, r7, 1
    slt            c2, r7, d.num_sges
    bcf            [c2], decode_sge_len_encoding
    // if not, go to next sge length
    // current_sge_offset = current_sge_offset - sge_len
    sub            r2, r2, r4 // Branch Delay Slot

trigger_rrqsge_process:
    bcf            [!c2], insufficient_sges
    add            r3, d.read.wqe_sge_list_addr, TXWQE_SGE_LEN_ENC_SIZE // Branch Delay Slot
    //rrqsge_p = cur_sge_id << log_sizeof_sge_t
    add            r3, r3, r7, LOG_SIZEOF_SGE_T

    // if log_rq_page_size = 0, rq is in hbm and page boundary check is not needed
    phvwr           CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, end_of_page), 0

    seq             c3, K_LOG_PAGE_SIZE, 0
    sub.c3          r3, r3, 2, LOG_SIZEOF_SGE_T
    bcf             [c3], page_boundary_check_done

    srl             r5, r3, K_LOG_PAGE_SIZE //BD Slot
    add             r6, r3, (RRQWQE_SGE_TABLE_READ_SIZE - 1)
    srl             r6, r6, K_LOG_PAGE_SIZE
    sne             c3, r5, r6
    // move addr_to_load back by sizeof 2 SGE's
    sub.!c3         r3, r3, 2, LOG_SIZEOF_SGE_T
    // start addr and end addr are not in the same page, move addr_to_load back by sizeof 3 SGE's
    phvwr.c3        CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, end_of_page), 1
    sub.c3          r3, r3, 3, LOG_SIZEOF_SGE_T

page_boundary_check_done:
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, \
                              req_rx_rrqsge_process, r3)

    CAPRI_RESET_TABLE_0_ARG()
    //phvwr CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, is_atomic), 0
    phvwrpair CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, remaining_payload_bytes), K_REMAINING_PAYLOAD_BYTES, \
              CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, cur_sge_id), r7
    phvwr     CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, cur_sge_offset), r2
    sub       r3, d.num_sges, r7
    phvwrpair.e CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, num_valid_sges), r3, \
                CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_start_index), K_DMA_CMD_START_INDEX
    // c5 is set in req_rx_rrqwqe_process if its last or only read resp packet
    phvwr.!c5  CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_eop), 1

default_sge_fmt_opt_process:
sge1_offset:
    // sge_p[0]
    add            r1, r0, (RRQWQE_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))

    // if total bytes transferred is within first sge's length, then
    // current_sge_offset is equal to total bytes transferred
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, len)
    blt            r2, r4, sge_loop
    // r3 = K_REMAINING_PAYLOAD_BYTES
    add            r3, r0, K_REMAINING_PAYLOAD_BYTES // Branch Delay Slot

sge2_offset:
    // sge_p[0]
    sub            r1, r1, 1, LOG_SIZEOF_SGE_T_BITS

    // if total bytes transferred is greater than first sge's length, then
    // current_sge_offset in second sge is (total_bytes - first_sge_length)
    sub            r2, r2, r4
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, len)

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

    // To start with, set dma_cmd_eop to 0
    //CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, dma_cmd_eop, 0)

    // current_sge_offset += transfer_bytes
    add            r2, r2, r4

    // remaining_payload_bytes -= transfer_bytes
    sub            r3, r3, r4

    // if end of loop, set k[].dma_cmd_eop = TRUE;
    sle            c2, r3, r0
    slt            c3, 1, d.num_sges

    setcf          c4, [!c2 & c3 & c7]
    // set dma_cmd_eop in last dma cmd for the pkt
    cmov            r4, c5, 0, 1
    CAPRI_SET_FIELD_C(r7, RRQSGE_TO_LKEY_T, dma_cmd_eop, r4, c2)

    add            r4, r0, K_DMA_CMD_START_INDEX
    add.!c7        r4, r4, MAX_PYLD_DMA_CMDS_PER_SGE
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, dma_cmd_start_index, r4)

    cmov           r6, c7, 0, 1
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, sge_index, r6)

    // r6 = hbm_addr_get(PHV_GLOBA_KT_BASE_ADDR_GET())
    KT_BASE_ADDR_GET2(r6, r4)

    // r4 = sge_p->lkey
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, l_key)

    seq            c6, r4, RDMA_RESERVED_LKEY_ID
    CAPRI_SET_FIELD_C(r7, RRQSGE_TO_LKEY_T, rsvd_key_err, 1, c6)
    crestore.c6    [c6], K_PRIV_OPER_ENABLE, 0x1

    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey & KEY_INDEX_MASK) * sizeof(key_entry_t));
    KEY_ENTRY_ADDR_GET(r6, r6, r4)

    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, is_atomic, 0)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, cq_dma_cmd_index, REQ_RX_DMA_CMD_CQ)
    //CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, cq_id, K_CQ_ID)

    // r4 = sge_p->len
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, len)

    // if (current_sge_offset == sge_p->len)
    seq            c1, r2, r4

    // Get common.common_te[0]_phv_table_addr or common.common_te[1]_phv_table_Addr ... based on
    // sge_index to invoke program in multiple MPUs
    CAPRI_GET_TABLE_0_OR_1_K(req_rx_phv_t, r7, c7)
    // aligned_key_addr and key_id sent to next stage to load lkey
    CAPRI_NEXT_TABLE_I_READ_PC_C(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_rrqlkey_rsvd_lkey_process, req_rx_rrqlkey_process, r6, c6)

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

    bcf            [!c2], insufficient_sges
    nop            // Branch Delay Slot

set_arg:
    phvwr.c5  CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, last_pkt), 1
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwr     CAPRI_PHV_FIELD(TO_S4_SQCB1_WRITE_BACK_P, sge_opt), 1

    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

insufficient_sges:
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rrqsge_insuff_sges), 1 //BD Slot
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1
    phvwrpair      p.cqe.status, CQ_STATUS_LOCAL_LEN_ERR, p.cqe.error, 1
    phvwr          CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp), 1
    b              set_arg
    CAPRI_SET_TABLE_0_VALID(0) // Branch Delay Slot

table_error:
    // set err_dis_qp
    phvwr          CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp), 1
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1

    phvwrpair      p.cqe.status, CQ_STATUS_LOCAL_QP_OPER_ERR, p.cqe.error, 1
    phvwr          CAPRI_PHV_RANGE(TO_S7_P, lif_cqe_error_id_vld, lif_error_id), \
                    ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_REMOTE_OPER_ERR_OFFSET))

    // update stats
    phvwr          CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1
    phvwr.c2       CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_table_error), 1
    phvwr.c3       CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_phv_intrinsic_error), 1
    phvwr.c7       CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_table_resp_error), 1

    b              set_arg
    CAPRI_SET_TABLE_0_VALID(0)
