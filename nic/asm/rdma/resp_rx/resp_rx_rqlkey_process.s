#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s4_t1_k k;
struct key_entry_aligned_t d;

#define MY_PT_BASE_ADDR r2
#define LOG_PT_SEG_SIZE r1
#define PT_SEG_P r5
#define PT_OFFSET r3

#define DMA_CMD_BASE r1
#define TMP r3
#define DB_ADDR r4
#define DB_DATA r5
#define GLOBAL_FLAGS r6
#define RQCB2_ADDR r7
#define RQCB1_ADDR r7
#define T2_ARG r5

#define LKEY_TO_PT_INFO_T   struct resp_rx_lkey_to_pt_info_t
#define INFO_WBCB1_P t2_s2s_rqcb1_write_back_info
#define TO_S_WB_P    to_s5_wb1_info

#define IN_P t1_s2s_key_info
#define IN_TO_S_P to_s4_lkey_info

#define K_VA CAPRI_KEY_RANGE(IN_P, va_sbit0_ebit23, va_sbit32_ebit63)

%%
    .param  resp_rx_ptseg_process
    .param  resp_rx_rqcb1_write_back_process

.align
resp_rx_rqlkey_process:

    //If Reserved LKEY is used, but QP doesn't have privileged operations enabled
    bbeq        CAPRI_KEY_FIELD(IN_P, rsvd_key_err), 1, error_completion

    // access is allowed only in valid state
    seq         c1, d.state, KEY_STATE_VALID //BD Slot
    // check pd for MR lkey
    seq         c2, d.pd, CAPRI_KEY_FIELD(IN_TO_S_P, pd) 
    bcf         [!c1 | !c2], error_completion

    //ARE_ALL_FLAGS_SET_B(c1, r1, ACC_CTRL_LOCAL_WRITE)
    smeqb       c1, d.acc_ctrl, ACC_CTRL_LOCAL_WRITE, ACC_CTRL_LOCAL_WRITE // BD Slot
    bcf         [!c1], error_completion

    //  if ((lkey_info_p->sge_va < lkey_p->base_va) ||
    //  ((lkey_info_p->sge_va + lkey_info_p->sge_bytes) > (lkey_p->base_va + lkey_p->len))) {
    slt         c1, K_VA, d.base_va  // BD Slot
    add         r1, d.base_va, d.len
    //add         r2, k.args.va, k.args.len
    //slt         c2, r1, r2
    sslt        c2, r1, K_VA, CAPRI_KEY_FIELD(IN_P, len)
    bcf         [c1 | c2], error_completion
    
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
    add         r3, r3, K_VA
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
    seq         c2, CAPRI_KEY_FIELD(IN_P, tbl_id), 0    //BD Slot

unaligned_pt:
    // pt_offset = transfer_offset % lkey_info_p->page_size;
    // pt_seg_p = (u64 *)my_pt_base_addr + (transfer_offset / lkey_info_p->page_size);

    // x = transfer_offset/log_page_size
    srlv        r5, r3, d.log_page_size
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
    CAPRI_GET_TABLE_0_OR_1_K(resp_rx_phv_t, r7, c2)
    CAPRI_NEXT_TABLE_I_READ_PC(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_ptseg_process, PT_SEG_P)
    CAPRI_GET_TABLE_0_OR_1_ARG(resp_rx_phv_t, r7, c2)
    CAPRI_SET_FIELD(r7, LKEY_TO_PT_INFO_T, pt_offset, PT_OFFSET)
    CAPRI_SET_FIELD_RANGE(r7, LKEY_TO_PT_INFO_T, pt_bytes, sge_index, CAPRI_KEY_RANGE(IN_P, len, tbl_id))
    CAPRI_SET_FIELD(r7, LKEY_TO_PT_INFO_T, log_page_size, d.log_page_size)
    CAPRI_SET_FIELD_RANGE(r7, LKEY_TO_PT_INFO_T, override_lif_vld, override_lif, d.{override_lif_vld...override_lif})

skip_pt:
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS 

    seq         c3, CAPRI_KEY_FIELD(IN_P, dma_cmdeop), 1
    bcf         [!c3], check_write_back

    IS_ANY_FLAG_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_RING_DBELL) // BD Slot

    CAPRI_SET_FIELD_C(r7, LKEY_TO_PT_INFO_T, dma_cmdeop, 1, !c2)
    
check_write_back:
    bbeq        CAPRI_KEY_FIELD(IN_P, invoke_writeback), 0, exit
    RQCB1_ADDR_GET(RQCB1_ADDR)      //BD Slot

    phvwr       CAPRI_PHV_FIELD(INFO_WBCB1_P, current_sge_offset), \
                CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit15, current_sge_offset_sbit24_ebit31)

    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqcb1_write_back_process, RQCB1_ADDR)

exit:
    nop.e
    nop

error_completion:
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS

    phvwr       p.s1.ack_info.aeth.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_OP_ERR)
    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_ACC_ERR, p.cqe.error, 1

    // set error disable flag 
    // if it is send and error is encourntered, even first/middle packets
    // should generate completion queue error.
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_COMPLETION
    
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS)

    RQCB2_ADDR_GET(RQCB2_ADDR)
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK)

    // prepare for NAK
    RESP_RX_POST_ACK_INFO_TO_TXDMA(DMA_CMD_BASE, RQCB2_ADDR, TMP, \
                                   K_GLOBAL_LIF,
                                   K_GLOBAL_QTYPE,
                                   K_GLOBAL_QID,
                                   DB_ADDR, DB_DATA)
    
    //Generate DMA command to skip to payload end
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/)

    //clear both lkey0 and lkey1 table valid bits
    CAPRI_SET_TABLE_0_VALID(0)
    b       check_write_back
    CAPRI_SET_TABLE_1_VALID(0)  //BD Slot
