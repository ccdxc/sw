#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_sqlkey_process_k_t k;
struct key_entry_aligned_t d;

#define INFO_OUT_T struct req_tx_lkey_to_ptseg_info_t

%%
    .param    req_tx_sqptseg_process

.align
req_tx_sqlkey_process:

     // if (!(lkey_p->access_ctrl & ACC_CTRL_LOCAL_WRITE))
     and          r2, d.acc_ctrl, ACC_CTRL_LOCAL_WRITE
     beq          r2, r0, access_violation

     // if ((lkey_info_p->sge_va < lkey_p->base_va) ||
     //     ((lkey_info_p->sge_va + lkey_info_p->sge_bytes) > (lkey_p->base_va + lkey_p->len)))
     slt          c1, k.args.sge_va, d.base_va // Branch Delay Slot
     add          r3, d.len, d.base_va
     sslt         c2, r3, k.args.sge_va, k.args.sge_bytes
     bcf          [c1|c2], access_violation

     // my_pt_base_addr = (void *)(hbm_addr_get(PHV_GLOBAL_PT_BASE_ADDR_GET()) +
     //                            (lkey_p->pt_base * sizeof(u64))
     PT_BASE_ADDR_GET(r4) // Branch Delay Slot
     add          r3, r4, d.pt_base, CAPRI_LOG_SIZEOF_U64

     // pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE * lkey_info_p->page_size
     add          r4, d.log_page_size, LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE

     // lkey_p->base_va % pt_seg_size
     add          r5, d.base_va, r0
     mincr        r5, r4, r0

     // transfer_offset = lkey_info_p->sge_va - lkey_p->base_va + (lkey_p->base_va % pt_seg_size)
     sub          r2, k.args.sge_va, d.base_va
     add          r2, r2, r5

     // if ((transfer_offset + transfer_bytes) > pt_seg_size)
     add          r5, r2, k.args.sge_bytes
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
     add          r3, r3, r2, CAPRI_LOG_SIZEOF_U64  // Branch Delay Slot
     //add          r3, r3, r2, LOG_HBM_CACHE_LINE_SIZE

     // Unaligned PT access
pt_unaligned_access:
     // pt_offset = transfer_offset % lkey_info_p->page_size
     mincr        r5, d.log_page_size, r0

     // pt_seg_p = (u64 *)my_pt_base_addr + (transfer_offset / lkey_info_p->log_page_size)
     srlv         r2, r2, d.log_page_size
     add          r3, r3, r2, CAPRI_LOG_SIZEOF_U64

set_arg:
     seq          c1, k.args.sge_index, 0
     CAPRI_GET_TABLE_0_OR_1_ARG(req_tx_phv_t, r7, c1)
     CAPRI_SET_FIELD(r7, INFO_OUT_T, pt_offset, r5)
     CAPRI_SET_FIELD(r7, INFO_OUT_T, log_page_size, d.log_page_size)
     //CAPRI_SET_FIELD(r7, INFO_OUT_T, pt_bytes, k.args.sge_bytes)
     //CAPRI_SET_FIELD(r7, INFO_OUT_T, dma_cmd_start_index, k.args.dma_cmd_start_index)
     //CAPRI_SET_FIELD(r7, INFO_OUT_T, sge_index, k.args.sge_index)
     //CAPRI_SET_FIELD(r7, INFO_OUT_T, dma_cmd_eop, k.args.dma_cmd_eop)
     CAPRI_SET_FIELD_RANGE(r7, INFO_OUT_T, pt_bytes, dma_cmd_eop, k.{args.sge_bytes...args.dma_cmd_eop})

     CAPRI_GET_TABLE_0_OR_1_K(req_tx_phv_t, r7, c1)
     CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqptseg_process)
     CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

     nop.e
     nop

access_violation:
    nop.e
    nop
