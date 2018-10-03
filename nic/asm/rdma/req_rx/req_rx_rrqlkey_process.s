#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
//this routine is invoked on s3_t0 and s3_t1
struct req_rx_s3_t0_k k;
struct key_entry_aligned_t d;

#define IN_P t0_s2s_rrqsge_to_lkey_info
#define IN_TO_S_P to_s3_rrqlkey_info

#define K_SGE_VA CAPRI_KEY_RANGE(IN_P, sge_va_sbit0_ebit7, sge_va_sbit56_ebit63)
#define K_SGE_BYTES CAPRI_KEY_RANGE(IN_P, sge_bytes_sbit0_ebit7, sge_bytes_sbit8_ebit15)
#define K_SGE_INDEX CAPRI_KEY_RANGE(IN_P, sge_index_sbit0_ebit1, sge_index_sbit2_ebit7)
#define K_DMA_CMD_START_INDEX CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)
#define K_PD CAPRI_KEY_FIELD(IN_TO_S_P, pd)

#define LKEY_TO_PTSEG_T struct req_rx_rrqlkey_to_ptseg_info_t
#define RRQSGE_TO_LKEY_P t0_s2s_rrqsge_to_lkey_info

%%
    .param    req_rx_rrqptseg_process

.align
req_rx_rrqlkey_process:

     bbne        CAPRI_KEY_FIELD(IN_P, bubble_one_stage), 1, proceed_rrqlkey_process
     phvwr       CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, bubble_one_stage), 0 //BD Slot
     nop //BD Slot
     nop.e
     nop
     
proceed_rrqlkey_process:

     //If Reserved LKEY is used, but QP doesn't have privileged operations enabled
     bbeq        CAPRI_KEY_FIELD(IN_P, rsvd_key_err), 1, error_completion

     seq          c1, d.state, KEY_STATE_VALID //BD Slot
     bcf          [!c1], invalid_region
  
     seq          c1, K_PD, d.pd // Branch Delay Slot
     bcf          [!c1], pd_check_failure

     // if (!(lkey_p->access_ctrl & ACC_CTRL_LOCAL_WRITE))
     and          r2, d.acc_ctrl, ACC_CTRL_LOCAL_WRITE // Branch Delay Slot
     beq          r2, r0, access_violation

     // if ((lkey_info_p->sge_va < lkey_p->base_va) ||
     //     ((lkey_info_p->sge_va + lkey_info_p->sge_bytes) > (lkey_p->base_va + lkey_p->len)))
     slt          c1, K_SGE_VA, d.base_va // Branch Delay Slot
     add          r3, d.len, d.base_va
     sslt         c2, r3, K_SGE_VA, K_SGE_BYTES
     bcf          [c1|c2], access_violation

     // my_pt_base_addr = (void *)(hbm_addr_get(PHV_GLOBAL_PT_BASE_ADDR_GET()) +
     //                            (lkey_p->pt_base * sizeof(u64))
     PT_BASE_ADDR_GET2(r4) // Branch Delay Slot
     add          r3, r4, d.pt_base, CAPRI_LOG_SIZEOF_U64

     // pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE * lkey_info_p->page_size
     add          r4, d.log_page_size, LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE

     // lkey_p->base_va % pt_seg_size
     add          r5, d.base_va, r0
     mincr        r5, r4, r0

     // transfer_offset = lkey_info_p->sge_va - lkey_p->base_va + (lkey_p->base_va % pt_seg_size)
     sub          r2, K_SGE_VA, d.base_va
     add          r2, r2, r5

     // if ((transfer_offset + transfer_bytes) > pt_seg_size)
     add          r5, r2, K_SGE_BYTES
     //  pt_seg_size = 1 << (LOG_PAGE_SIZE + HBM_MUM_PT_ENTRIES_PER_CACHE_LINE)
     sllv         r6, 1, r4
     blt          r6, r5, pt_unaligned_access
     add          r5, r2, r0 // Branch Delay Slot

     // Aligned PT access
pt_aligned_access:
     // pt_offset = transfer_offset % pt_seg_size
     mincr        r5, r4, r0

     // pt_seg_p = (u64 *)my_pt_base_addr + ((transfer_offset /lkey_info_p->page_size) / HBM_NUM_PT_ENTRIES_PER_CACHE_LINE)
     srlv         r2, r2, r4
     b            set_arg
     add          r3, r3, r2, (CAPRI_LOG_SIZEOF_U64 + LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE)  // Branch Delay Slot
     //add          r3, r3, r2, LOG_HBM_CACHE_LINE_SIZE

pt_unaligned_access:
     // pt_offset = transfer_offset % lkey_info_p->page_size
     mincr        r5, d.log_page_size, r0

     // pt_seg_p = (u64 *)my_pt_base_addr + (transfer_offset / lkey_info_p->log_page_size)
     srlv         r2, r2, d.log_page_size
     add          r3, r3, r2, CAPRI_LOG_SIZEOF_U64

set_arg:
     add          r2, K_SGE_INDEX, r0
     CAPRI_GET_TABLE_I_ARG(req_rx_phv_t, r2, r7)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, pt_offset, r5)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, pt_bytes, K_SGE_BYTES)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, is_atomic, CAPRI_KEY_FIELD(IN_P, is_atomic))
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, dma_cmd_start_index, K_DMA_CMD_START_INDEX)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, log_page_size, d.log_page_size)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, dma_cmd_eop, CAPRI_KEY_FIELD(IN_P, dma_cmd_eop))
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, sge_index, K_SGE_INDEX)

     CAPRI_GET_TABLE_I_K(req_rx_phv_t, r2, r7)
     CAPRI_NEXT_TABLE_I_READ_PC(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqptseg_process, r3)

     nop.e
     nop

pd_check_failure:
invalid_region:
    b              error_completion
    phvwrpair      p.cqe.status, CQ_STATUS_LOCAL_PROT_ERR, p.cqe.error, 1 

access_violation:
    phvwrpair      p.cqe.status, CQ_STATUS_LOCAL_ACC_ERR, p.cqe.error, 1 
    //fall through

error_completion:
    add          r1, K_SGE_INDEX, r0
    CAPRI_SET_TABLE_I_VALID(r1, 0)

    phvwr.e        CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp),  1
    nop
