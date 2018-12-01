#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqwqe_t d;
struct req_tx_s2_t0_k k;

#define WQE_TO_SGE_P t0_s2s_wqe_to_sge_info
#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info
#define SQCB_WRITE_BACK_RD_P t2_s2s_sqcb_write_back_info_rd
#define SQCB_WRITE_BACK_SEND_WR_P t2_s2s_sqcb_write_back_info_send_wr
#define WQE_TO_LKEY_T0 t0_s2s_sqwqe_to_lkey_inv_info
#define WQE_TO_LKEY_T1 t1_s2s_sqwqe_to_lkey_inv_info
#define WQE_TO_FRPMR_LKEY_T0 t0_s2s_sqwqe_to_lkey_frpmr_info
#define WQE_TO_FRPMR_LKEY_T1 t1_s2s_sqwqe_to_lkey_frpmr_info
#define SQWQE_TO_LKEY_MW_T0_P t0_s2s_sqwqe_to_lkey_mw_info
#define SQWQE_TO_LKEY_MW_T1_P t1_s2s_sqwqe_to_lkey_mw_info

#define IN_P t0_s2s_sqcb_to_wqe_info
#define IN_TO_S_P to_s2_sqwqe_info

#define TO_S4_DCQCN_BIND_MW_P   to_s4_dcqcn_bind_mw_info
#define TO_S4_FRPMR_LKEY_P      to_s4_frpmr_sqlkey_info
#define TO_S5_SQCB_WB_ADD_HDR_P to_s5_sqcb_wb_add_hdr_info
#define TO_S6_FRPMR_WB_P        to_s6_frpmr_sqcb_wb_info
#define TO_S6_SQCB_WB_ADD_HDR_P to_s6_sqcb_wb_add_hdr_info
#define TO_S7_STATS_P           to_s7_stats_info


#define K_LOG_PMTU CAPRI_KEY_FIELD(IN_P, log_pmtu)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit0, remaining_payload_bytes_sbit9_ebit15)
#define K_HEADER_TEMPLATE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, header_template_addr_sbit0_ebit7, header_template_addr_sbit24_ebit31)
#define K_PRIVILEGED_QKEY K_HEADER_TEMPLATE_ADDR
#define K_READ_REQ_ADJUST CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit0, current_sge_offset_sbit25_ebit31)
#define K_SPEC_CINDEX CAPRI_KEY_RANGE(IN_TO_S_P, spec_cindex_sbit0_ebit7, spec_cindex_sbit8_ebit15)
#define K_AH_BASE_ADDR_PAGE_ID CAPRI_KEY_RANGE(IN_TO_S_P, ah_base_addr_page_id_sbit0_ebit7, ah_base_addr_page_id_sbit16_ebit21)
#define K_FAST_REG_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, fast_reg_rsvd_lkey_enable)

%%
    .param    req_tx_sqsge_process
    .param    req_tx_dcqcn_enforce_process
    .param    req_tx_sqlkey_invalidate_process
    .param    req_tx_bind_mw_sqlkey_process
    .param    req_tx_load_ah_size_process
    .param    req_tx_load_hdr_template_process 
    .param    req_tx_frpmr_sqlkey_process

.align
req_tx_sqwqe_process:

    bbne           CAPRI_KEY_FIELD(IN_P, poll_in_progress), 1, skip_color_check
    //color check
    seq            c1, CAPRI_KEY_FIELD(IN_P, color), d.base.color  // BD-slot
    bcf            [!c1], clear_poll_in_progress

skip_color_check:
    bbeq           CAPRI_KEY_FIELD(IN_P, fence_done), 1, skip_fence_check
    add            r1, r0, d.base.op_type  //BD-slot
    phvwr          CAPRI_PHV_FIELD(TO_S5_SQCB_WB_ADD_HDR_P, fence), d.base.fence

skip_fence_check:
    // Populate wrid in phv to post error-completion for wqes or completion for non-packet-wqes.
    phvwr          p.rdma_feedback.completion.wrid, d.base.wrid
    add            r2, r0, K_GLOBAL_FLAGS

    .brbegin
    br             r1[3:0]
    IS_ANY_FLAG_SET(c7, r2, REQ_TX_FLAG_REXMIT) // Branch Delay Slot

    .brcase OP_TYPE_SEND
        b               send_or_write
        nop             //Branch Delay slot

    .brcase OP_TYPE_SEND_INV
        b               send_or_write
        nop             //Branch Delay slot
        
    .brcase OP_TYPE_SEND_IMM
        b               send_or_write
        nop             //Branch Delay slot

    .brcase OP_TYPE_READ
        b               read
        nop             //Branch Delay slot

    .brcase OP_TYPE_WRITE
        b               set_write_reth
        nop             //Branch Delay slot

    .brcase OP_TYPE_WRITE_IMM
        b               set_write_reth
        nop             //Branch Delay slot

    .brcase OP_TYPE_CMP_N_SWAP
        b               atomic
        nop             //Branch Delay slot

    .brcase OP_TYPE_FETCH_N_ADD
        b               atomic
        nop             //Branch Delay slot 

    .brcase OP_TYPE_FRPMR
        b.!c7           frpmr
        b.c7            skip_npg_wqe // Branch Delay Slot
        nop             //Branch Delay slot 

    .brcase OP_TYPE_LOCAL_INV
        b.!c7           local_inv
        b.c7            skip_npg_wqe // Branch Delay Slot
        nop             //Branch Delay slot

    .brcase OP_TYPE_BIND_MW
        b.!c7           bind_mw
        b.c7            skip_npg_wqe // Branch Delay Slot
        nop             //Branch Delay slot
      
    .brcase OP_TYPE_SEND_INV_IMM
        nop.e
        nop

    .brcase OP_TYPE_FRMR
        nop.e
        nop

    .brcase 13
        nop.e
        nop

    .brcase 14
        nop.e
        nop

    .brcase 15
        nop.e
        nop

    .brend

set_write_reth:
    phvwrpair RETH_VA, d.write.va, RETH_RKEY, d.write.r_key
    phvwr RETH_LEN, d.write.length

send_or_write:
    seq            c3, d.base.num_sges, 0
    // If UD, add DETH hdr
    seq            c1, CAPRI_KEY_FIELD(phv_global_common, _ud), 1
    bcf            [!c1], set_sge_arg
    seq            c2, d.base.inline_data_vld, 1 // Branch Delay Slot

    seq            c4, K_FAST_REG_ENABLE, 0x1
    seq            c5, d.ud_send.q_key[31:31], 0x1
    bcf            [c5 & !c4], ud_error
    seq            c6, K_PRIVILEGED_QKEY[31:31], 0x1  // BD slot
    bcf            [c5 & !c6], ud_error
    cmov           r4, c5, K_PRIVILEGED_QKEY, d.ud_send.q_key // BD slot

    phvwrpair DETH_Q_KEY, r4, DETH_SRC_QP, K_GLOBAL_QID
    phvwr BTH_DST_QP, d.ud_send.dst_qp

    // For UD, length should be less than pmtu
    sll            r4, 1,  K_LOG_PMTU
    add            r5, r0, d.ud_send.length
    add            r6, d.ud_send.ah_handle, r0
    muli           r2, r6, AT_ENTRY_SIZE_BYTES
    blt            r4, r5, ud_error
    add            r2, r2, K_AH_BASE_ADDR_PAGE_ID, HBM_PAGE_SIZE_SHIFT
    // AH_SIZE is the last byte in AH_ENTRY
    add            r6, r2, HDR_TEMPLATE_T_SIZE_BYTES
    srl            r2, r2, HDR_TEMP_ADDR_SHIFT

    CAPRI_RESET_TABLE_3_ARG()
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_load_ah_size_process, r6)

set_sge_arg:
    bcf            [c2], inline_data
    // sge_list_addr = wqe_addr + TX_SGE_OFFSET
    mfspr          r3, spr_tbladdr //Branch Delay Slot
    bcf            [c3], zero_length
    add            r3, r3, TXWQE_SGE_OFFSET // Branch Delay Slot

    // populate stage-2-stage data req_tx_wqe_to_sge_info_t for next stage
    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(WQE_TO_SGE_P, in_progress), CAPRI_KEY_FIELD(IN_P, in_progress), CAPRI_PHV_FIELD(WQE_TO_SGE_P, first), 1
    phvwrpair CAPRI_PHV_FIELD(WQE_TO_SGE_P, num_valid_sges), d.base.num_sges, CAPRI_PHV_FIELD(WQE_TO_SGE_P, remaining_payload_bytes), K_REMAINING_PAYLOAD_BYTES
    phvwrpair CAPRI_PHV_FIELD(WQE_TO_SGE_P, op_type), d.base.op_type, CAPRI_PHV_FIELD(WQE_TO_SGE_P, dma_cmd_start_index), REQ_TX_DMA_CMD_PYLD_BASE
    //imm_data and inv_key are union members
    phvwrpair CAPRI_PHV_RANGE(WQE_TO_SGE_P, poll_in_progress, color), CAPRI_KEY_RANGE(IN_P, poll_in_progress, color), CAPRI_PHV_FIELD(WQE_TO_SGE_P, imm_data_or_inv_key), d.{base.imm_data}
    // if UD copy ah_handle
    phvwr.c1 CAPRI_PHV_FIELD(WQE_TO_SGE_P, ah_handle), r2

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqsge_process, r3)

    nop.e
    nop

read:
    // prepare atomic header
    #phvwr           RETH_VA_RKEY_LEN, d.{read.va...read.length}
    add            r4, d.read.va, K_READ_REQ_ADJUST
    sub            r5, d.read.length, K_READ_REQ_ADJUST
    phvwrpair      RETH_VA, r4, RETH_RKEY, d.read.r_key
    phvwr          RETH_LEN, r5

    // prepare RRQWQE descriptor
    phvwrpair      RRQWQE_READ_RSP_OR_ATOMIC, RRQ_OP_TYPE_READ, RRQWQE_NUM_SGES, d.base.num_sges
    mfspr          r3, spr_tbladdr
    add            r2, r3, TXWQE_SGE_OFFSET
    phvwrpair      RRQWQE_READ_LEN, d.read.length, RRQWQE_READ_WQE_SGE_LIST_ADDR, r2

    CAPRI_RESET_TABLE_2_ARG()
    //set first = 1, last_pkt = 1
    phvwrpair CAPRI_PHV_RANGE(SQCB_WRITE_BACK_P, first, last_pkt), 3, CAPRI_PHV_FIELD(SQCB_WRITE_BACK_RD_P, op_rd_log_pmtu), K_LOG_PMTU
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, op_type), r1, CAPRI_PHV_FIELD(SQCB_WRITE_BACK_RD_P, op_rd_read_len), r5
    // leave rest of variables to FALSE

    phvwr CAPRI_PHV_FIELD(TO_S5_SQCB_WB_ADD_HDR_P, read_req_adjust), K_READ_REQ_ADJUST

    add            r2, AH_ENTRY_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)
    CAPRI_SET_TABLE_0_1_VALID(0, 0);     

    nop.e
    nop

atomic:
    // prepare atomic header
    phvwrpair      ATOMIC_VA, d.atomic.va, ATOMIC_R_KEY, d.atomic.r_key
    phvwrpair      ATOMIC_SWAP_OR_ADD_DATA, d.atomic.swap_or_add_data, ATOMIC_CMP_DATA, d.atomic.cmp_data

    // prepare RRQWQE descriptor
    phvwrpair      RRQWQE_READ_RSP_OR_ATOMIC, RRQ_OP_TYPE_ATOMIC, RRQWQE_NUM_SGES, 1
    phvwr          p.{rrqwqe.atomic.sge.va, rrqwqe.atomic.sge.len, rrqwqe.atomic.sge.l_key}, d.{atomic.sge.va, atomic.sge.len, atomic.sge.l_key}
 
    CAPRI_RESET_TABLE_2_ARG()
    //set first = 1, last_pkt = 1
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, op_type), r1, CAPRI_PHV_RANGE(SQCB_WRITE_BACK_P, first, last_pkt), 3
    // leave rest of variables to FALSE

    add            r2, AH_ENTRY_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)
    CAPRI_SET_TABLE_0_1_VALID(0, 0)

    nop.e
    nop

inline_data:
    phvwr          p.inline_data, d.inline_data
    phvwr          CAPRI_PHV_FIELD(phv_global_common, _inline),  1
    DMA_CMD_STATIC_BASE_GET(r4, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_PYLD_BASE)
    DMA_PHV2PKT_START_LEN_SETUP(r4, r5, inline_data, d.send.length)
    // NOTE: it should be noted that invoke_add_headers will directly invoke
    // add_headers phase without any sge process as the data is inline.
    // The length of data is populated in length argument. All the 'length'
    // parameter values for various union cases such as read/write/send are located
    // at same offset. So, though argument passing code is passing read.length, 
    // it should work for inline data as well.

zero_length:
    phvwr CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, packet_len), d.send.length

    CAPRI_RESET_TABLE_2_ARG()
    //set first = 1, last_pkt = 1
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, op_type), r1, CAPRI_PHV_RANGE(SQCB_WRITE_BACK_P, first, last_pkt), 3
    phvwrpair.!c1 CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, hdr_template_inline), 1, \
                  CAPRI_PHV_FIELD(SQCB_WRITE_BACK_SEND_WR_P, op_send_wr_imm_data_or_inv_key), d.{base.imm_data}
    phvwrpair.c1 CAPRI_PHV_FIELD(SQCB_WRITE_BACK_SEND_WR_P, op_send_wr_imm_data_or_inv_key), d.{base.imm_data}, \
                 CAPRI_PHV_FIELD(SQCB_WRITE_BACK_SEND_WR_P, op_send_wr_ah_handle), r2
    phvwr CAPRI_PHV_RANGE(SQCB_WRITE_BACK_P, poll_in_progress, color), CAPRI_KEY_RANGE(IN_P, poll_in_progress, color)
    // leave rest of variables to FALSE

    add            r2, AH_ENTRY_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)

    bcf            [c1], ud_exit // Do not optimize header load for UD
    CAPRI_RESET_TABLE_0_ARG()    // BD slot

    // Load MPU only hdr_template_process. Actual hdr-template-address will be loaded in stage 5
    // table0 is free for inline/zero_length packets in stages 3,4 and 5 since sge processing doesn't happen.
    sll            r2, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT //BD slot
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_load_hdr_template_process, r2)

    nop.e
    nop

ud_exit:
    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop

local_inv:
    /*
     * Fetch lkey address and load dummy-sqlkey MPU-only program in stage3. 
     * Actual lkey address will be loaded in stage4 for invalidation.
     */

    // r6 = hbm_addr_get(PHV_GLOBA_KT_BASE_ADDR_GET())
    KT_BASE_ADDR_GET2(r6, r4)

    // r4 = lkey
    add            r4, r0, d.base.key

    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+ ((lkey & KEY_INDEX_MASK) * sizeof(key_entry_t));
    KEY_ENTRY_ADDR_GET(r6, r6, r4)
    //set first = 1, last_pkt = 1
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, op_type), r1, CAPRI_PHV_RANGE(SQCB_WRITE_BACK_P, first, last_pkt), 3
    bbeq           CAPRI_KEY_FIELD(IN_P, fence_done), 1, skip_li_fence
    nop // BD-slot
    phvwr CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, set_li_fence), d.base.fence
    phvwr CAPRI_PHV_FIELD(WQE_TO_LKEY_T0, set_li_fence), d.base.fence
    phvwr CAPRI_PHV_FIELD(WQE_TO_LKEY_T1, set_li_fence), d.base.fence

skip_li_fence:
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_sqlkey_invalidate_process, r6)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_sqlkey_invalidate_process, r6)

    nop.e
    nop
    
bind_mw:
    KT_BASE_ADDR_GET2(r2, r3)
    add            r3, d.bind_mw.l_key, r0
    KEY_ENTRY_ADDR_GET(r2, r2, r3)

    // Pass l_key in to_stage to stage 4 so that bind_mw_rkey can store lkey in rkey
    // for reference to MR to which it is bound to.
    phvwr          CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, mr_l_key), d.bind_mw.l_key

    // bind_mw_sqlkey_process should be invoked in stage4 T0 and T1.
    CAPRI_RESET_TABLE_0_ARG()
    phvwr          CAPRI_PHV_RANGE(SQWQE_TO_LKEY_MW_T0_P, va, len), d.{bind_mw.va, bind_mw.len}
    phvwrpair      CAPRI_PHV_FIELD(SQWQE_TO_LKEY_MW_T0_P, r_key), d.base.key, CAPRI_PHV_FIELD(SQWQE_TO_LKEY_MW_T0_P, new_r_key_key), d.base.new_r_key_key
    phvwr          CAPRI_PHV_RANGE(SQWQE_TO_LKEY_MW_T0_P, acc_ctrl, zbva), d.{bind_mw.access_ctrl, bind_mw.mw_type, bind_mw.zbva}

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_bind_mw_sqlkey_process, r2)

    // local_inv wqe followed by bind_mw should be performed in order. Any rkey
    // invalidated should reflect the state as invalid for type2 bind_mw
    // otherwise bind_mw will error disable the QP. To do this, recirc bind_mw
    // phv after bind_mw_sqlkey_process and do bind_mw_rkey_process in same
    // stage as local_inv_process so that local_inv update to key_entry is
    // committed to P4 cache (if not in bypass cache) and read in
    // bind_mw_rkey_process will reflect the updated key_entry

    // Note that dcqcn_enforce_process is in same stage as bind_mw_rkey_process
    // so any error in this program cannot be conveyed to dcqcn_enforce. This
    // will result in dcqcn_cb state to be updated by phvs in pipeline until
    // write_back updates CB with error state and further new phvs are not
    // processed in stage0. write_back and sqcb2_write_back are triggered in
    // second pass in stage after bind_mw_rkey_process so the corresponding CB
    // state will be frozen and cannot be updated by subsequent phvs in the
    // pipeline

    CAPRI_RESET_TABLE_2_ARG()
    phvwrpair  CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, op_type), r1, \
               CAPRI_PHV_RANGE(SQCB_WRITE_BACK_P, first, last_pkt), 3
    phvwr      CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, non_packet_wqe), 1
    phvwr      p.common.p4_intr_recirc, 1
    phvwr      p.common.rdma_recirc_recirc_reason, REQ_TX_RECIRC_REASON_BIND_MW
   
    nop.e
    nop

frpmr:
    /*
     * Fetch lkey address and load frpmr-sqlkey MPU-only program in stage3. 
     * Actual lkey address will be loaded in stage4.
     */

    // r6 = hbm_addr_get(PHV_GLOBA_KT_BASE_ADDR_GET())
    KT_BASE_ADDR_GET2(r6, r4)

    // r4 = lkey
    add            r4, r0, d.base.key

    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+ ((lkey & KEY_INDEX_MASK) * sizeof(key_entry_t));
    KEY_ENTRY_ADDR_GET(r6, r6, r4)

    // Send all relevant FRPMR fields to stage4.
    CAPRI_RESET_TABLE_0_ARG()

    // If second pass - skip filling s2s data.
    bbeq           CAPRI_KEY_FIELD(IN_P, frpmr_lkey_state_upd), 1, frpmr_second_pass
    CAPRI_RESET_TABLE_1_ARG() //BD-slot

    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, fast_reg_rsvd_lkey_enable), K_FAST_REG_ENABLE, \
                   CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, base_va), d.frpmr.base_va
    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, zbva), d.frpmr.zbva, \
                   CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, new_user_key), d.{base.new_user_key}
    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, sge_index), 0, \
                   CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, pt_start_offset), d.frpmr.pt_start_offset
    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, mw_en), d.frpmr.mw_en, \
                   CAPRI_PHV_RANGE(WQE_TO_FRPMR_LKEY_T0, num_pt_entries, acc_ctrl), d.{frpmr.num_pt_entries, frpmr.access_ctrl}
    phvwr          CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, log_page_size), d.frpmr.log_page_size

    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, fast_reg_rsvd_lkey_enable), K_FAST_REG_ENABLE, \
                   CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, base_va), d.frpmr.base_va
    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, zbva), d.frpmr.zbva, \
                   CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, new_user_key), d.{base.new_user_key}
    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, sge_index), 0, \
                   CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, pt_start_offset), d.frpmr.pt_start_offset
    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, mw_en), d.frpmr.mw_en, \
                   CAPRI_PHV_RANGE(WQE_TO_FRPMR_LKEY_T1, num_pt_entries, acc_ctrl), d.{frpmr.num_pt_entries, frpmr.access_ctrl}
    phvwr          CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, log_page_size), d.frpmr.log_page_size


    phvwr          CAPRI_PHV_FIELD(TO_S4_FRPMR_LKEY_P, len), d.frpmr.len

    // Send DMA info to stage6.
    phvwrpair      CAPRI_PHV_FIELD(TO_S6_FRPMR_WB_P, wqe_addr), k.common_te0_phv_table_addr, \
                   CAPRI_PHV_FIELD(TO_S6_FRPMR_WB_P, frpmr_dma_src_addr), d.frpmr.dma_src_address

load_frpmr_sqlkey:
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_frpmr_sqlkey_process, r6)
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_frpmr_sqlkey_process, r6)

frpmr_second_pass:
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_STATS_P, npg), 1, \
                   CAPRI_PHV_FIELD(TO_S7_STATS_P, npg_frpmr), 1

    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, sge_index), 0, CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T0, lkey_state_update), 1
    b              load_frpmr_sqlkey             
    phvwrpair      CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, sge_index), 1, CAPRI_PHV_FIELD(WQE_TO_FRPMR_LKEY_T1, lkey_state_update), 1 //BD-slot

exit:
ud_error:
    //For UD we can silently drop
    phvwr.e   p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)    

clear_poll_in_progress:
    CAPRI_RESET_TABLE_2_ARG()
    phvwr CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, poll_failed), 1

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)
    CAPRI_SET_TABLE_0_1_VALID(0, 0)
    nop.e
    nop

skip_npg_wqe:
    CAPRI_RESET_TABLE_2_ARG()
    // Clear fence bit if SQ is in the retransmission mode
    phvwr      CAPRI_PHV_FIELD(TO_S5_SQCB_WB_ADD_HDR_P, fence), 0
    phvwrpair  CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, op_type), r1, \
               CAPRI_PHV_RANGE(SQCB_WRITE_BACK_P, first, last_pkt), 3
    phvwr      CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, non_packet_wqe), 1
    // Drop the phv as there is no completion to be posted for NPG
    // wqes on retransmission
    phvwr      p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_1_VALID(0, 0)
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)
