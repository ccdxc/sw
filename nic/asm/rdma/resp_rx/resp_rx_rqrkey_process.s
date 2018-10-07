#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s2_t1_k k;
struct key_entry_aligned_t d;

#define MY_PT_BASE_ADDR r2
#define LOG_PT_SEG_SIZE r1
#define PT_SEG_P r5
#define PT_OFFSET r3

#define DMA_CMD_BASE r1
#define GLOBAL_FLAGS r6
#define KT_BASE_ADDR r6
#define KEY_ADDR r6
#define ABS_VA r4

#define RKEY_TO_PT_INFO_T   struct resp_rx_lkey_to_pt_info_t
#define RKEY_TO_LKEY_INFO_T struct resp_rx_key_info_t
#define INFO_WBCB1_P t2_s2s_rqcb1_write_back_info
#define TO_S_LKEY_P  to_s4_lkey_info
#define TO_S_WB1_P to_s5_wb1_info
#define TO_S_CQCB_P to_s6_cqcb_info


#define IN_P t1_s2s_rkey_info
#define IN_TO_S_P to_s2_ext_hdr_info

#define K_VA CAPRI_KEY_RANGE(IN_P, va_sbit0_ebit7, va_sbit8_ebit63)
#define K_ACC_CTRL CAPRI_KEY_RANGE(IN_P, acc_ctrl_sbit0_ebit4, acc_ctrl_sbit5_ebit7)

%%
    .param  resp_rx_ptseg_mpu_only_process
    .param  resp_rx_rqcb1_write_back_mpu_only_process
    .param  resp_rx_rqlkey_mr_cookie_process

.align
resp_rx_rqrkey_process:

    // check if invalidate is attempted on rsvd_key value
    bbeq        CAPRI_KEY_FIELD(IN_P, rsvd_key_err), 1, error_completion

    //ARE_ALL_FLAGS_SET_B(c1, r1, ACC_CTRL_LOCAL_WRITE)
    and         r1, d.acc_ctrl, K_ACC_CTRL // BD Slot
    seq         c1, r1, K_ACC_CTRL
    bcf         [!c1], error_completion

    add         ABS_VA, K_VA, r0 // BD Slot
    smeqb       c1, d.flags, MR_FLAG_ZBVA, MR_FLAG_ZBVA
    // if ZBVA, add key table's base_va to k_va
    add.c1      ABS_VA, ABS_VA, d.base_va

    //  if ((lkey_info_p->sge_va < lkey_p->base_va) ||
    //  ((lkey_info_p->sge_va + lkey_info_p->sge_bytes) > (lkey_p->base_va + lkey_p->len))) {
    slt         c1, ABS_VA, d.base_va  // BD Slot
    add         r1, d.base_va, d.len
    //add         r2, k.args.va, k.args.len
    //slt         c2, r1, r2
    sslt        c2, r1, ABS_VA, CAPRI_KEY_FIELD(IN_P, len)
    bcf         [c1 | c2], error_completion
    
    // check if state is valid (same for MR and MW)
    // if MR or MW and type 1, check PD
    // if MW and type 2A, check QP

    // access is allowed only in valid state
    seq         c1, d.state, KEY_STATE_VALID // BD Slot
    bcf         [!c1], error_completion

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

    b           error_completion

check_pd:
    seq         c4, d.pd, CAPRI_KEY_FIELD(IN_TO_S_P, pd) // BD slot
    bcf         [!c4], error_completion
    nop         // BD Slot

cookie_check:
    // if MR, skip cookie check
    bcf         [c5], skip_mr_cookie_check
    CAPRI_SET_FIELD2(TO_S_LKEY_P, mw_cookie, d.mr_cookie) // BD Slot

    KT_BASE_ADDR_GET2(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, d.mr_l_key)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqlkey_mr_cookie_process, KEY_ADDR)

skip_mr_cookie_check:
    seq         c1, CAPRI_KEY_FIELD(IN_P, skip_pt), 1   //BD Slot
    bcf         [c1], skip_pt
    CAPRI_SET_TABLE_1_VALID_C(c1, 0)    //BD Slot

    // my_pt_base_addr = (void *)
    //     (hbm_addr_get(PHV_GLOBAL_PT_BASE_ADDR_GET()) +
    //         (lkey_p->pt_base * sizeof(u64)));

    PT_BASE_ADDR_GET2(r2) //BD Slot
    add         MY_PT_BASE_ADDR, r2, d.pt_base, CAPRI_LOG_SIZEOF_U64
    // now r2 has my_pt_base_addr
    

    // pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE * phv_p->page_size;
    // i.e., log_pt_seg_size = LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE + LOG_PAGE_SIZE
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
    // get absolute pt_seg_size
    sllv        r6, 1, LOG_PT_SEG_SIZE
    // (transfer_offset % pt_seg_size) + transfer_bytes
    //add         r7, r7, k.args.len
    // pt_seg_size <= ((transfer_offset % pt_seg_size) + transfer_bytes)
    //sle         c1, r6, r7
    ssle        c1, r6, r7, CAPRI_KEY_FIELD(IN_P, len)
    bcf         [!c1], aligned_pt

unaligned_pt:
    // pt_offset = transfer_offset % lkey_info_p->page_size;
    // pt_seg_p = (u64 *)my_pt_base_addr + (transfer_offset / lkey_info_p->page_size);

    // x = transfer_offset/log_page_size
    srlv        r5, r3, d.log_page_size // BD Slot 
    // transfer_offset%log_page_size 
    //add         r6, r0, r3
    mincr       PT_OFFSET, d.log_page_size, r0
    // now r6 has pt_offset
    b           invoke_pt
    add         PT_SEG_P, MY_PT_BASE_ADDR, r5, CAPRI_LOG_SIZEOF_U64 //BD Slot
    // now r5 has pt_seg_p
aligned_pt:
    // pt_offset = transfer_offset % pt_seg_size;
    // pt_seg_p = (u64 *)my_pt_base_addr + ((transfer_offset / phv_p->page_size) / HBM_NUM_PT_ENTRIES_PER_CACHE_LINE);
    srlv        r5, r3, LOG_PT_SEG_SIZE
    //add         r6, r0, r3
    mincr       PT_OFFSET, LOG_PT_SEG_SIZE, r0
    add         PT_SEG_P, MY_PT_BASE_ADDR, r5, (CAPRI_LOG_SIZEOF_U64 + LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE)

invoke_pt:
    // invoke ptseg mpu only
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_ptseg_mpu_only_process, PT_SEG_P)

    CAPRI_GET_TABLE_1_ARG(resp_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, RKEY_TO_PT_INFO_T, pt_offset, PT_OFFSET)
    CAPRI_SET_FIELD_RANGE(r7, RKEY_TO_PT_INFO_T, pt_bytes, sge_index, CAPRI_KEY_RANGE(IN_P, len, tbl_id))
    CAPRI_SET_FIELD(r7, RKEY_TO_PT_INFO_T, log_page_size, d.log_page_size)
    //host_addr, override_lif_vld, override_lif
    CAPRI_SET_FIELD_RANGE(r7, RKEY_TO_PT_INFO_T, host_addr, override_lif, d.{host_addr...override_lif})

skip_pt:
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    IS_ANY_FLAG_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_ATOMIC_CSWAP)
    phvwr.c2    p.pcie_atomic.compare_data_or_add_data, k.{to_s2_ext_hdr_info_ext_hdr_data[63:0]}.dx

    CAPRI_SET_FIELD(r7, RKEY_TO_PT_INFO_T, dma_cmdeop, CAPRI_KEY_FIELD(IN_P, dma_cmdeop))
    
write_back:
    // invoke mpu only program since we are in stage 2, and wb is loaded in stage 5
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

error_completion:
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    IS_ANY_FLAG_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_COMPLETION)
    IS_ANY_FLAG_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_READ_REQ|RESP_RX_FLAG_ATOMIC_FNA|RESP_RX_FLAG_ATOMIC_CSWAP)

    phvwr.!c1   CAPRI_PHV_FIELD(TO_S_WB1_P, async_or_async_error_event), 1
    phvwr.!c1   CAPRI_PHV_FIELD(TO_S_CQCB_P, async_error_event), 1
    phvwrpair.!c1 p.s1.eqwqe.code, EQE_CODE_QP_ERR_ACCESS, p.s1.eqwqe.type, EQE_TYPE_QP
    phvwr.!c1   p.s1.eqwqe.qid, K_GLOBAL_QID

    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR)
    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_ACC_ERR, p.cqe.error, 1

    // set error disable flag 
    // turn on ACK req bit when error disabling QP
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_ACK_REQ

    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS)

    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, !c2)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, c2)

    //Generate DMA command to skip to payload end
    // move dma cmd base by 2 to accomodate ACK info 
    // and doorbell ringing
    DMA_NEXT_CMD_I_BASE_GET(DMA_CMD_BASE, 2)
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/)

    b       write_back
    CAPRI_SET_TABLE_1_VALID(0)  //BD Slot
