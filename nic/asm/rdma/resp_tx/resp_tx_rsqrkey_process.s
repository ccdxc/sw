#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s3_t0_k k;
struct key_entry_aligned_t d;

#define KEY_P r7
#define MY_PT_BASE_ADDR r2
#define LOG_PT_SEG_SIZE r1
#define PT_SEG_P r5
#define PT_OFFSET r3
#define RQCB1_ADDR r5
#define DMA_CMD_BASE r1
#define IN_PROGRESS r2
#define KT_BASE_ADDR r6
#define KEY_ADDR r6
#define GLOBAL_FLAGS r6
#define ABS_VA r4

#define PTSEG_INFO_P    t0_s2s_rkey_to_ptseg_info
#define RQCB0_WB_INFO_P t1_s2s_rqcb0_write_back_info

#define IN_P    t0_s2s_rsqwqe_to_rkey_info
#define IN_TO_S_P to_s3_dcqcn_info
#define TO_S4_P  to_s4_dcqcn_info
#define TO_S5_P  to_s5_rqcb1_wb_info
#define TO_S7_P  to_s7_stats_info
#define INFO_RKEY_COOKIE_P t3_s2s_rsqrkey_to_rkey_cookie_info

#define K_XFER_VA CAPRI_KEY_RANGE(IN_P, transfer_va_sbit0_ebit7, transfer_va_sbit48_ebit63)
#define K_XFER_BYTES CAPRI_KEY_RANGE(IN_P, transfer_bytes_sbit0_ebit7, transfer_bytes_sbit8_ebit15)
#define K_HDR_TMP CAPRI_KEY_RANGE(IN_P, header_template_addr_sbit0_ebit7, header_template_addr_sbit24_ebit31)
#define K_HDR_TMP_SZ CAPRI_KEY_FIELD(IN_P, header_template_size)
#define K_DCQCN_CB_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, dcqcn_cb_addr_sbit0_ebit15, dcqcn_cb_addr_sbit32_ebit33)
#define K_PD CAPRI_KEY_RANGE(IN_TO_S_P, pd_sbit0_ebit15, pd_sbit24_ebit31)

%%
    .param      resp_tx_rsqptseg_process
    .param      resp_tx_dcqcn_enforce_process
    .param      resp_tx_rsqrkey_mr_cookie_process

resp_tx_rsqrkey_process:

    CAPRI_SET_TABLE_0_VALID(0)
    bbeq        CAPRI_KEY_FIELD(IN_P, skip_rkey), 1, add_headers
    
    // lkey_p = lkey_p + lkey_info_p->key_id;
    //big-endian
    sub         KEY_P, (HBM_NUM_KEY_ENTRIES_PER_CACHE_LINE - 1), CAPRI_KEY_FIELD(IN_P, key_id) //BD Slot
    add         KEY_P, r0, KEY_P, LOG_SIZEOF_KEY_ENTRY_T_BITS

    // if (!(lkey_p->acc_ctrl & ACC_CTRL_LOCAL_WRITE)) {
    ARE_ALL_FLAGS_SET_B(c1, d.acc_ctrl, ACC_CTRL_REMOTE_READ)
    bcf         [!c1], rkey_acc_ctrl_err

    add         ABS_VA, K_XFER_VA, r0 // BD Slot
    IS_ANY_FLAG_SET_B(c1, d.acc_ctrl, ACC_CTRL_ZERO_BASED)
    // if ZBVA, add key table's base_va to k_va
    add.c1      ABS_VA, ABS_VA, d.base_va

    //  if ((lkey_info_p->sge_va < lkey_p->base_va) ||
    //  ((lkey_info_p->sge_va + lkey_info_p->sge_bytes) > (lkey_p->base_va + lkey_p->len))) {
    slt         c1, ABS_VA, d.base_va // BD Slot
    add         r1, d.base_va, d.len
    sslt        c2, r1, ABS_VA, K_XFER_BYTES
    bcf         [c1 | c2], rkey_va_err
    nop         //BD slot

    // check if state is valid (same for MR and MW)
    // if MR or MW and type 1, check PD
    // if MW and type 2A, check QP

    // access is allowed only in valid state
    seq         c1, d.state, KEY_STATE_VALID // BD Slot
    bcf         [!c1], rkey_state_err

    // check PD if MR
    seq         c5, d.type, MR_TYPE_MR // BD Slot
    // check PD if MW type 1
    seq         c6, d.type, MR_TYPE_MW_TYPE_1
    bcf         [c5 | c6], check_pd
    // c5: MR, c6: MW type 1

    // neither MW type 1, nor MR, must be MW type 2A, check QP
    seq         c3, d.qp, K_GLOBAL_QID // BD Slot
    bcf         [c3], cookie_check
    nop // BD Slot

    b           rkey_qp_err

check_pd:
    seq         c4, d.pd, K_PD // BD slot
    bcf         [!c4], rkey_pd_err
    nop         // BD Slot

cookie_check:
    // if MR, skip cookie check
    bcf         [c5], skip_mr_cookie_check
    CAPRI_SET_FIELD2(INFO_RKEY_COOKIE_P, mw_cookie, d.mr_cookie) // BD Slot

    KT_BASE_ADDR_GET2(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, d.mr_l_key)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rsqrkey_mr_cookie_process, KEY_ADDR)

skip_mr_cookie_check:
    // my_pt_base_addr = (void *)
    //     (hbm_addr_get(PHV_GLOBAL_PT_BASE_ADDR_GET()) +
    //         (lkey_p->pt_base * sizeof(u64)));

    seq         c4, d.is_phy_addr, 1  
    bcf         [c4], contig_mr

    PT_BASE_ADDR_GET2(r2)
    add         MY_PT_BASE_ADDR, r2, d.pt_base, CAPRI_LOG_SIZEOF_U64
    // now r2 has my_pt_base_addr
    
    // pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE * phv_p->page_size;
    // i.e., log_pt_seg_size = LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE + log_page_size
    add         LOG_PT_SEG_SIZE, LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE, d.log_page_size
    // now r1 has log_pt_seg_size
    
    
    // transfer_offset = lkey_info_p->sge_va - lkey_p->base_va + lkey_p->base_va % pt_seg_size;
    add         r3, r0, d.base_va
    // base_va % pt_seg_size
    mincr       r3, LOG_PT_SEG_SIZE, r0
    // add sge_va
    add         r3, r3, ABS_VA
    // subtract base_va
    sub         r3, r3, d.base_va
    // now r3 has transfer_offset

    // transfer_offset % pt_seg_size
    add         r7, r0, r3
    mincr       r7, LOG_PT_SEG_SIZE, r0
    // (transfer_offset % pt_seg_size) + transfer_bytes
    add         r7, r7, K_XFER_BYTES
    // get absolute pt_seg_size
    sllv        r6, 1, LOG_PT_SEG_SIZE
    // pt_seg_size <= ((transfer_offset % pt_seg_size) + transfer_bytes)
    sle         c1, r6, r7
    bcf         [!c1], aligned_pt
    nop

unaligned_pt:
    // pt_offset = transfer_offset % lkey_info_p->page_size;
    // pt_seg_p = (u64 *)my_pt_base_addr + (transfer_offset / lkey_info_p->page_size);

    // x = transfer_offset/log_page_size
    srlv        r5, r3, d.log_page_size
    // transfer_offset%log_page_size 
    //add         r6, r0, r3
    mincr       PT_OFFSET, d.log_page_size, r0
    // now r6 has pt_offset
    add         PT_SEG_P, MY_PT_BASE_ADDR, r5, CAPRI_LOG_SIZEOF_U64
    // now r5 has pt_seg_p
    b           invoke_pt
    nop
aligned_pt:
    // pt_offset = transfer_offset % pt_seg_size;
    // pt_seg_p = (u64 *)my_pt_base_addr + ((transfer_offset / phv_p->page_size) / HBM_NUM_PT_ENTRIES_PER_CACHE_LINE);
    srlv        r5, r3, LOG_PT_SEG_SIZE
    //add         r6, r0, r3
    mincr       PT_OFFSET, LOG_PT_SEG_SIZE, r0
    add         PT_SEG_P, MY_PT_BASE_ADDR, r5, (CAPRI_LOG_SIZEOF_U64 + LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE)

invoke_pt:

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rsqptseg_process, PT_SEG_P)
    
    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD2(PTSEG_INFO_P, pt_seg_offset, PT_OFFSET)
    CAPRI_SET_FIELD2(PTSEG_INFO_P, pt_seg_bytes, K_XFER_BYTES)

    CAPRI_SET_FIELD2(PTSEG_INFO_P, dma_cmd_start_index, RESP_TX_DMA_CMD_PYLD_BASE)
    CAPRI_SET_FIELD2(PTSEG_INFO_P, log_page_size, d.log_page_size)
    CAPRI_SET_FIELD2(PTSEG_INFO_P, host_addr, d.host_addr)

add_headers:

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_HDR_TEMPLATE)
    sll r4, K_HDR_TMP, HDR_TEMP_ADDR_SHIFT
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, K_HDR_TMP_SZ, r4)
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)

    bbne        CAPRI_KEY_FIELD(IN_P, send_aeth), 1, invoke_dcqcn
    CAPRI_SET_FIELD2(TO_S7_P, pyld_bytes, K_XFER_BYTES) //BD Slot

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_AETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, aeth, aeth)

invoke_dcqcn:
    phvwr          p.common.p4_intr_packet_len, K_XFER_BYTES
    // Note: Next stage(DCQCN) does not use stage-to-stage keys. So this will be passed to write-back stage untouched!
    CAPRI_RESET_TABLE_1_ARG()
    seq         c1, CAPRI_KEY_FIELD(IN_P, last_or_only), 1
    cmov        IN_PROGRESS, c1, 0, 1
    phvwrpair   CAPRI_PHV_FIELD(RQCB0_WB_INFO_P, curr_read_rsp_psn), \
                CAPRI_KEY_RANGE(IN_P, curr_read_rsp_psn_sbit0_ebit7, curr_read_rsp_psn_sbit16_ebit23), \
                CAPRI_PHV_FIELD(RQCB0_WB_INFO_P, read_rsp_in_progress), \
                IN_PROGRESS

    seq            c3, CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_type), 1
    bcf            [c3], dcqcn
    add            r3,  r0, K_DCQCN_CB_ADDR // BD slot

dcqcn_mpu_only:
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_enforce_process, r3)

dcqcn:
    CAPRI_SET_FIELD2(TO_S4_P, packet_len, K_XFER_BYTES)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_dcqcn_enforce_process, r3)

exit:
    nop.e
    nop

rsvd_rkey_err:
    b           error_completion
    phvwrpair   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rsvd_rkey_err), 1 // BD Slot

rkey_pd_err:
    b           error_completion
    phvwrpair   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rkey_pd_mismatch), 1 // BD Slot

rkey_qp_err:
    b           error_completion
    phvwrpair   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_type2a_mw_qp_mismatch), 1 // BD Slot

rkey_state_err:
    b           error_completion
    phvwrpair   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rkey_state_err), 1 // BD Slot

rkey_acc_ctrl_err:
    b           error_completion
    phvwrpair   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rkey_acc_ctrl_err), 1 // BD Slot

rkey_va_err:
    b           error_completion
    phvwrpair   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rkey_va_err), 1 // BD Slot

error_completion:
    // When rkey validation for a read response fails, we generate a NAK
    // and ensure further read responses are not generated by moving the QP to
    // error disable state

    CAPRI_SET_FIELD2(phv_global_common, _error_disable_qp, 1)

    phvwr       CAPRI_PHV_FIELD(TO_S7_P, last_syndrome), AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR)
    phvwr       p.aeth.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR)
    b           dcqcn_mpu_only
    CAPRI_SET_FIELD2(TO_S5_P, ack_nak_process, 1) // BD Slot

contig_mr:
    
    bbeq        CAPRI_KEY_FIELD(IN_P, skip_rkey), 1, add_headers
    CAPRI_SET_TABLE_0_VALID(0) // BD Slot

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PYLD_BASE)
    // r1 has DMA_CMD_BASE

    sub         r2, K_XFER_VA, d.base_va
    add         r2, r2, d.phy_base_addr

    DMA_MEM2PKT_SETUP(DMA_CMD_BASE, c0, K_XFER_BYTES, r2)

    b           add_headers
    nop
