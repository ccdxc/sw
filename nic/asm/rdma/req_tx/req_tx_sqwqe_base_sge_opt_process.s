#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

struct req_tx_phv_t p;
struct sqwqe_t d;
struct req_tx_s2_t0_k k;

#define SGE_TO_LKEY_T struct req_tx_sge_to_lkey_info_t
#define SGE_TO_LKEY_P t0_s2s_sge_to_lkey_info
#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info
#define SQCB_WRITE_BACK_SEND_WR_P t2_s2s_sqcb_write_back_info_send_wr
#define WQE_TO_SGE_P t0_s2s_wqe_to_sge_info

#define IN_P t0_s2s_sqcb_to_wqe_info
#define IN_TO_S_P to_s2_sqwqe_info

#define TO_S0_SQSGE_P to_s0_sqsge_info
#define TO_S3_SQSGE_P to_s3_sqsge_info
#define TO_S4_DCQCN_BIND_MW_P to_s4_dcqcn_bind_mw_info
#define TO_S5_SQCB_WB_ADD_HDR_P to_s5_sqcb_wb_add_hdr_info
#define TO_S7_STATS_INFO_P to_s7_stats_info

#define K_CURRENT_SGE_ID CAPRI_KEY_FIELD(IN_P, current_sge_id)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit7, current_sge_offset_sbit24_ebit31)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)
#define K_HEADER_TEMPLATE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, header_template_addr_sbit0_ebit7, header_template_addr_sbit24_ebit31)
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, fast_reg_rsvd_lkey_enable)
#define K_LOG_PAGE_SIZE  CAPRI_KEY_FIELD(IN_TO_S_P, log_page_size)
#define K_MSG_PSN CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit7, current_sge_offset_sbit24_ebit31)
#define K_LOG_PMTU CAPRI_KEY_FIELD(IN_P, log_pmtu)

%%
    .param    req_tx_sqlkey_process
    .param    req_tx_sqlkey_rsvd_lkey_process
    .param    req_tx_dcqcn_enforce_process
    .param    req_tx_sqsge_process

.align
req_tx_sqwqe_base_sge_opt_process:

    // total bytes transferred = msg_psn << log_pmtu
    sll            r2, K_MSG_PSN, K_LOG_PMTU

    // If total bytes transferred is greater than msg length then drop the speculations.
    sle            c1, d.send.length, r2
    bcf            [c1], spec_drop

    // clear to-stage fence bit for mid-last packets.
    seq            c1, CAPRI_KEY_FIELD(IN_P, in_progress), 1 // BD-slot
    phvwr.c1       CAPRI_PHV_FIELD(TO_S5_SQCB_WB_ADD_HDR_P, fence), 0

    seq            c2, d.base.wqe_format, SQWQE_FORMAT_DEFAULT //BD-slot
    bcf            [c2], decode_sge_default

    seq            c3, d.base.wqe_format, SQWQE_FORMAT_8x4 //BD-slot
    CAPRI_RESET_TABLE_0_ARG()  

    // cur_sge_id = 0
    add            r7, r0, r0
    // Get to length-encoding.
    add            r1, TXWQE_SGE_OFFSET_BITS, r0

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


    sub.c3            r1, r1, 1, LOG_SIZEOF_WQE_8x4_T_BITS
    sub.!c3           r1, r1, 1, LOG_SIZEOF_WQE_16x2_T_BITS

    CAPRI_TABLE_GET_FIELD_C(r4, r1, WQE_8x4_T, len, c3)
    CAPRI_TABLE_GET_FIELD_C(r4, r1, WQE_16x2_T, len, !c3)

    // if total bytes transferred is within first sge's length, then
    // current_sge_offset is equal to total bytes transferred
    blt            r2, r4, trigger_sqsge_process
    nop            // Branch Delay Slot

    // cur_sge_id += 1
    add            r7, r7, 1
    slt            c1, r7, d.base.num_sges
    bcf            [c1], decode_sge_len_encoding
    // current_sge_offset = current_sge_offset - sge_len
    // if not, go to next sge length
    sub            r2, r2, r4 //BD-slot
    bcf            [!c1], spec_drop // Not-expected !! 

trigger_sqsge_process:
    // sge_list_addr = wqe_addr + TX_SGE_OFFSET
    mfspr          r3, spr_tbladdr  //Branch Delay Slot
    add            r3, r3, TXWQE_SGE_OFFSET

   // for sge_len encoded wqe, sge start is sge_list_addr + 32 + (cur_sge_id << log_size_of_sge_t)
    add            r3, r3, TXWQE_SGE_LEN_ENC_SIZE
    add            r3, r3, r7, LOG_SIZEOF_SGE_T

    seq            c3, CAPRI_KEY_FIELD(IN_P, in_progress), 1
    cmov           r5, c3, 0, 1

    // num-valid-sges = num_sges - cur_sge_id
    sub            r4, d.base.num_sges, r7

    // populate stage-2-stage data req_tx_wqe_to_sge_info_t for next stage
    phvwrpair       CAPRI_PHV_FIELD(WQE_TO_SGE_P, current_sge_id), r7, \
                    CAPRI_PHV_FIELD(WQE_TO_SGE_P, current_sge_offset), r2
    phvwrpair       CAPRI_PHV_FIELD(WQE_TO_SGE_P, in_progress), CAPRI_KEY_FIELD(IN_P, in_progress), \
                    CAPRI_PHV_FIELD(WQE_TO_SGE_P, first), r5
    phvwrpair       CAPRI_PHV_FIELD(WQE_TO_SGE_P, num_valid_sges), r4, \
                    CAPRI_PHV_FIELD(WQE_TO_SGE_P, remaining_payload_bytes), K_REMAINING_PAYLOAD_BYTES
    phvwrpair       CAPRI_PHV_FIELD(WQE_TO_SGE_P, op_type), d.base.op_type, \
                    CAPRI_PHV_FIELD(WQE_TO_SGE_P, dma_cmd_start_index), REQ_TX_DMA_CMD_PYLD_BASE
    // imm_data and inv_key are union members
    phvwrpair       CAPRI_PHV_RANGE(WQE_TO_SGE_P, poll_in_progress, color), CAPRI_KEY_RANGE(IN_P, poll_in_progress, color), \
                    CAPRI_PHV_FIELD(WQE_TO_SGE_P, imm_data_or_inv_key), d.{base.imm_data}

    srl             r5, r3, K_LOG_PAGE_SIZE
    add             r6, r3, (SQWQE_SGE_TABLE_READ_SIZE - 1)
    srl             r6, r6, K_LOG_PAGE_SIZE
    sne             c3, r5, r6
    // move addr_to_load back by sizeof 2 SGE's
    sub.!c3         r3, r3, 2, LOG_SIZEOF_SGE_T
    // start addr and end addr are not in the same page, move addr_to_load back by sizeof 3 SGE's
    phvwr.c3        CAPRI_PHV_FIELD(WQE_TO_SGE_P, end_of_page), 1
    sub.c3          r3, r3, 3, LOG_SIZEOF_SGE_T

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqsge_process, r3)


decode_sge_default:
    // Skip wqe-base and point to sge-encoding.
    add             r1, r0, (HBM_NUM_SGES_PER_CACHELINE - 3), LOG_SIZEOF_SGE_T_BITS

    // Use conditional flag to select between sge_index 0 and 1
    // sge_index = 0
    setcf          c7, [c0]

    // if total bytes transferred is within first sge's length, then current_sge_offset is equal to total bytes transferred
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, len)
    // Store current_sge_offset in r2 and remaining-payload-bytes in r3 for sge_opt_process
    add             r3, r0, K_REMAINING_PAYLOAD_BYTES
    blt             r2, r4, sge_loop
    // r5 = 0. It will be used for storing packet-len.
    add             r5, r0, r0 // BD-slot.

    // if total bytes transferred is greater than first sge's length, then
    // current_sge_offset in second sge is (total_bytes - first_sge_length)
    sub             r2, r2, r4
    // sge_p[1]
    sub             r1, r1, 1, LOG_SIZEOF_SGE_T_BITS
    setcf          c7, [!c0]
    // Reset table0 valid since lkey stage will only be loaded in table1
    CAPRI_SET_TABLE_0_VALID(0)

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
    CAPRI_GET_TABLE_0_OR_1_ARG(req_tx_phv_t, r7, c7)

    // Fill stage 2 stage data in req_tx_sge_lkey_info_t for next stage
    CAPRI_SET_FIELD(r7, SGE_TO_LKEY_T, sge_va, r6)
    CAPRI_SET_FIELD(r7, SGE_TO_LKEY_T, sge_bytes, r4)

    // current_sge_offset += transfer_bytes
    add            r2, r2, r4

    // remaining_payload_bytes -= transfer_bytes
    sub            r3, r3, r4

    sle            c2, r3, r0
    slt            c3, 1, d.base.num_sges

    setcf          c4, [!c2 & c3 & c7]

    // packet_len += transfer_bytes
    add            r5, r5, r4

    cmov           r6, c7, 0, 1
    CAPRI_SET_FIELD(r7, SGE_TO_LKEY_T, sge_index, r6)

    add            r4, r0, REQ_TX_DMA_CMD_PYLD_BASE
    add.!c7        r4, r4, MAX_PYLD_DMA_CMDS_PER_SGE
    CAPRI_SET_FIELD(r7, SGE_TO_LKEY_T, dma_cmd_start_index, r4)

    // r6 = hbm_addr_get(PHV_GLOBA_KT_BASE_ADDR_GET())
    KT_BASE_ADDR_GET2(r6, r4)

    // r4 = sge_p->lkey
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, l_key)

    seq            c6, r4, RDMA_RESERVED_LKEY_ID
    CAPRI_SET_FIELD_C(r7, SGE_TO_LKEY_T, rsvd_key_err, 1, c6)
    crestore.c6    [c6], K_PRIV_OPER_ENABLE, 0x1

    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey >> KEY_INDEX_MASK) * sizeof(key_entry_t));
    KEY_ENTRY_ADDR_GET(r6, r6, r4)

    // r4 = sge_p->len
    CAPRI_TABLE_GET_FIELD(r4, r1, SGE_T, len)

    // if (current_sge_offset == sge_p->len)
    seq            c1, r2, r4

    // Get common.common_te[0]_phv_table_addr or common.common_te[1]_phv_table_Addr ... based on
    // sge_index to invoke program in multiple MPUs
    CAPRI_GET_TABLE_0_OR_1_K(req_tx_phv_t, r7, c7)
    // aligned_key_addr and key_id sent to next stage to load lkey
    CAPRI_NEXT_TABLE_I_READ_PC_C(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_sqlkey_rsvd_lkey_process, req_tx_sqlkey_process, r6, c6)

    // sge_p[2]
    sub.c1         r1, r1, 1, LOG_SIZEOF_SGE_T_BITS

    // current_sge_offset = 0;
    add.c1         r2, r0, r0

    // while((remaining_payload_bytes > 0) &&
    //       (num_valid_sges > 1) &&
    //       (sge_index == 0)
    bcf            [c4], sge_loop
    // sge_index = 1, if looping
    setcf.c4       c7, [!c0] // branch delay slot

    // Pass packet_len to dcqcn_enforce and to add_headers_2 for padding and to stats_process
    phvwr          CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, packet_len), r5
    phvwr          CAPRI_PHV_FIELD(TO_S5_SQCB_WB_ADD_HDR_P, packet_len), r5
    phvwr          p.common.p4_intr_packet_len, r5
    phvwr          CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, pyld_bytes), r5

    // if (index == num_valid_sges)
    srl            r1, r1, LOG_SIZEOF_SGE_T_BITS
    sub            r1, (HBM_NUM_SGES_PER_CACHELINE - 3), r1
    seq            c1, r1[1:0], d.base.num_sges

    // in_progress = TRUE
    cmov           r4, c1, 0, 1

    // first = TRUE
    seq            c3, CAPRI_KEY_FIELD(IN_P, in_progress), 1
    cmov           r5, c3, 0, 1

    // Get Table 0/1 arg base pointer as it was modified to 0/1 K base
    CAPRI_GET_TABLE_0_OR_1_ARG_NO_RESET(req_tx_phv_t, r7, c7)

    // if (index == num_valid_sges) last = TRUE else last = FALSE;
    cmov           r3, c1, 1, 0

    CAPRI_RESET_TABLE_2_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, in_progress), r4, \
              CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, op_type), d.base.op_type
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, first), r5, \
              CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, last_pkt), r3
    phvwr     CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, current_sge_offset), K_MSG_PSN
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_SEND_WR_P, op_send_wr_imm_data_or_inv_key), d.base.imm_data,\
              CAPRI_PHV_FIELD(SQCB_WRITE_BACK_SEND_WR_P, op_send_wr_ah_handle), d.ud_send.ah_handle
    // rest of the fields are initialized to default

trigger_dcqcn:
    add           r1, AH_ENTRY_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r1)

spec_drop:
    phvwr.e        p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)
