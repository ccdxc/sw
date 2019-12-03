#include "req_rx.h"
#include "sqcb.h"
#include "defines.h"

struct req_rx_phv_t p;
struct req_rx_s2_t0_k k;

#define RRQSGE_TO_LKEY_T struct req_rx_rrqsge_to_lkey_info_t
#define SQCB1_WRITE_BACK_P t2_s2s_sqcb1_write_back_info

#define IN_P t0_s2s_rrqwqe_to_sge_info
#define IN_TO_S_P to_s2_rrqsge_info

#define K_CUR_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, cur_sge_offset_sbit0_ebit15, cur_sge_offset_sbit16_ebit31)
#define K_CUR_SGE_ID CAPRI_KEY_FIELD(IN_P, cur_sge_id)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit13)
#define K_DMA_CMD_START_INDEX CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)
#define K_CQ_ID CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit7, cq_id_sbit16_ebit23)
#define K_NUM_VALID_SGES CAPRI_KEY_FIELD(IN_P, num_valid_sges)

#define TO_S1_RECIRC_P to_s1_recirc_info
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, priv_oper_enable)
#define TO_S7_P to_s7_stats_info

%%
    .param    req_rx_rrqlkey_process
    .param    req_rx_rrqlkey_rsvd_lkey_process
    .param    req_rx_sqcb1_write_back_process
    .param    req_rx_recirc_mpu_only_process

.align
req_rx_rrqsge_process:
    bcf            [c2 | c3 | c7], table_error
    // c5: near the end of page
    seq            c5, CAPRI_KEY_FIELD(IN_P, end_of_page), 1 // BD Slot

    // Use conditional flag to select between sge_index 0 and 1
    // sge_index = 0
    setcf          c7, [c0] // BD Slot

    // sge_p[0]
    //add            r1, HBM_CACHE_LINE_SIZE_BITS, r0
    // Data structures are accessed from bottom to top in big-endian, hence go to
    // the bottom of the SGE_T
    // big-endian
    add.!c5        r1, r0, (RRQWQE_OPT_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))
    add.c5         r1, r0, (RRQWQE_OPT_LAST_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))

    // r2 = k.args.current_sge_offset
    add            r2, r0, K_CUR_SGE_OFFSET

    // r3 = K_REMAINING_PAYLOAD_BYTES
    add            r3, r0, K_REMAINING_PAYLOAD_BYTES

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
    slt            c3, 1, K_NUM_VALID_SGES
   
    setcf          c4, [!c2 & c3 & c7]
    // set dma_cmd_eop in last dma cmd for the pkt
    CAPRI_SET_FIELD_C(r7, RRQSGE_TO_LKEY_T, dma_cmd_eop, CAPRI_KEY_FIELD(IN_P, dma_cmd_eop), c2)

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
    
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, is_atomic, CAPRI_KEY_FIELD(IN_P, is_atomic))
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
    CAPRI_NEXT_TABLE_I_READ_PC_C(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqlkey_rsvd_lkey_process, req_rx_rrqlkey_process, r6, c6)

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

    // new logic is to read from the bottom of 64bytes, move point back to get the right index
    // new logic is to read from the bottom of 64bytes, move point back to get the right index
    add.!c5        r1, r1, (HBM_NUM_SGES_PER_CACHELINE - 2), LOG_SIZEOF_SGE_T_BITS
    add.c5         r1, r1, (HBM_NUM_SGES_PER_CACHELINE - 1), LOG_SIZEOF_SGE_T_BITS
    srl            r1, r1, LOG_SIZEOF_SGE_T_BITS
    sub            r1, (HBM_NUM_SGES_PER_CACHELINE-1), r1
    seq            c1, r1, K_NUM_VALID_SGES
    add.!c1        r1, r1, K_CUR_SGE_ID

    // TODO Set Phv recirc if remaining_payload_bytes is non-zero (cf - c2 set to False)

    //if (REQ_RX_FLAG_IS_SET(last) || (REQ_RX_FLAG_ONLY(only)))
    add             r7, r0, K_GLOBAL_FLAGS

    bcf             [!c2], recirc
    IS_ANY_FLAG_SET(c5, r7, REQ_RX_FLAG_LAST|REQ_RX_FLAG_ONLY)

    bcf            [!c5], set_arg
    nop            // Branch Delay Slot
    // current_sge_id = 0
    add             r1, r0, r0
    // currrent_sge_offset = 0
    add             r2, r0, r0

set_arg:
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_id), r1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_offset), r2, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwr.c5  CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, last_pkt), 1

    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

recirc:
    sle     c3, K_NUM_VALID_SGES, 2
    bcf     [c3], insufficient_sges

    // increment dma_cmd_start_index by num of DMA cmds consumed for 2 SGES per pass
    add            r4, K_DMA_CMD_START_INDEX, (MAX_PYLD_DMA_CMDS_PER_SGE * 2) // Branch Delay Slot
    // Error if there are no dma cmd space to compose
    beqi           r4, REQ_RX_RDMA_PAYLOAD_DMA_CMDS_END, err_no_dma_cmds

    phvwrpair CAPRI_PHV_FIELD(TO_S1_RECIRC_P, cur_sge_offset), r2, \
              CAPRI_PHV_FIELD(TO_S1_RECIRC_P, cur_sge_id), r1
    sub       r5, K_NUM_VALID_SGES, 2
    phvwrpair CAPRI_PHV_FIELD(TO_S1_RECIRC_P, num_valid_sges), r5, \
              CAPRI_PHV_FIELD(TO_S1_RECIRC_P, remaining_payload_bytes), r3
    phvwr     CAPRI_PHV_FIELD(TO_S1_RECIRC_P, dma_cmd_eop), CAPRI_KEY_FIELD(IN_P, dma_cmd_eop)

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_recirc_mpu_only_process, r0)

    phvwr.e p.common.p4_intr_recirc, 1
    phvwr   p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_SGE_WORK_PENDING

insufficient_sges:
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rrqsge_insuff_sges), 1 //BD Slot
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1
    phvwrpair      p.cqe.status, CQ_STATUS_LOCAL_LEN_ERR, p.cqe.error, 1
    phvwr          CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp), 1
    b              set_arg
    CAPRI_SET_TABLE_0_VALID(0) // Branch Delay Slot

err_no_dma_cmds:
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rrqsge_insuff_dma_cmds), 1
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1
    phvwrpair      p.cqe.status, CQ_STATUS_LOCAL_QP_OPER_ERR, p.cqe.error, 1
    // Error disable QP
    phvwr         CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp), 1
    b              set_arg
    CAPRI_SET_TABLE_0_VALID(0)

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

