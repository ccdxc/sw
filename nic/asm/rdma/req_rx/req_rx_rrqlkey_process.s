#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_rrqlkey_process_k_t k;

#define LKEY_TO_PTSEG_T struct req_rx_rrqlkey_to_ptseg_info_t

%%
    .param    req_rx_rrqptseg_process

.align
req_rx_rrqlkey_process:

     //r1 = lkey_p = lkey_p + k.args.key_id
     // big-endian
     sub          r1, (HBM_NUM_KEY_ENTRIES_PER_CACHE_LINE -1), k.args.key_id
     add          r1, r0, r1, LOG_SIZEOF_KEY_ENTRY_T_BITS

     // r2 = lkey_p->acc_trl
     CAPRI_TABLE_GET_FIELD(r2, r1, KEY_ENTRY_T, acc_ctrl)

     // if (!(lkey_p->access_ctrl & ACC_CTRL_LOCAL_WRITE))
     andi         r2, r2, ACC_CTRL_LOCAL_WRITE
     beq          r2, r0, access_violation

     // r2 = lkey_p->base_va
     tblrdp       r2, r1, offsetof(KEY_ENTRY_T, base_va), sizeof(KEY_ENTRY_T.base_va)  // Branch Delay Slot

     // r3 = lkey_p->len
     tblrdp       r3, r1, offsetof(KEY_ENTRY_T, len), sizeof(KEY_ENTRY_T.len)

     // if ((lkey_info_p->sge_va < lkey_p->base_va) ||
     //     ((lkey_info_p->sge_va + lkey_info_p->sge_bytes) > (lkey_p->base_va + lkey_p->len)))
     slt          c1, k.args.sge_va, r2
     add          r3, r3, r2
     add          r4, k.args.sge_va, k.args.sge_bytes
     slt          c2, r3, r4
     bcf          [c1|c2], access_violation

     // my_pt_base_addr = (void *)(hbm_addr_get(PHV_GLOBAL_PT_BASE_ADDR_GET()) +
     //                            (lkey_p->pt_base * sizeof(u64))
     tblrdp       r3, r1, offsetof(KEY_ENTRY_T, pt_base), sizeof(KEY_ENTRY_T.pt_base) // Branch Delay Slot
     PT_BASE_ADDR_GET(r4)
     add          r3, r4, r3, CAPRI_LOG_SIZEOF_U64

     // pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE * lkey_info_p->page_size
     tblrdp       r1, r1, offsetof(KEY_ENTRY_T, log_page_size), sizeof(KEY_ENTRY_T.log_page_size)
     add          r4, r1, LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE

     // lkey_p->base_va % pt_seg_size
     add          r5, r2, r0
     mincr        r5, r4, r0

     // transfer_offset = lkey_info_p->sge_va - lkey_p->base_va + (lkey_p->base_va % pt_seg_size)
     sub          r2, k.args.sge_va, r2
     add          r2, r2, r5

     // if ((transfer_offset + transfer_bytes) > pt_seg_size)
     add          r5, r2, k.args.sge_bytes
     //  pt_seg_size = 1 << (LOG_PAGE_SIZE + HBM_MUM_PT_ENTRIES_PER_CACHE_LINE)
     sllv         r6, 1, r4
     slt          c1, r6, r5
     add          r5, r2, r0

     // Unaligned PT access
pt_unaligned_access:
     // pt_offset = transfer_offset % lkey_info_p->page_size
     mincr.c1     r5, k.args.log_page_size, r0

     // pt_seg_p = (u64 *)my_pt_base_addr + (transfer_offset / lkey_info_p->log_page_size)
     add.c1       r4, r1, r0
     srlv.c1      r2, r2, r4
     add.c1       r3, r3, r2, CAPRI_LOG_SIZEOF_U64

     // else
     // Aligned PT access
pt_aligned_access:
     // pt_offset = transfer_offset % pt_seg_size
     mincr.!c1    r5, r4, r0

     // pt_seg_p = (u64 *)my_pt_base_addr + ((transfer_offset /lkey_info_p->page_size) / HBM_NUM_PT_ENTRIES_PER_CACHE_LINE)
     srlv.!c1     r2, r2, r4
     add.!c1      r3, r3, r2, CAPRI_LOG_SIZEOF_U64 
     //add.!c1      r3, r3, r2, LOG_HBM_CACHE_LINE_SIZE

     add          r2, k.args.sge_index, r0
     CAPRI_GET_TABLE_I_ARG(req_rx_phv_t, r2, r7)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, pt_offset, r5)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, pt_bytes, k.args.sge_bytes)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, dma_cmd_start_index, k.args.dma_cmd_start_index)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, log_page_size, r1)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, dma_cmd_eop, k.args.dma_cmd_eop)
     CAPRI_SET_FIELD(r7, LKEY_TO_PTSEG_T, sge_index, k.args.sge_index)

     CAPRI_GET_TABLE_I_K(req_rx_phv_t, r2, r7)
     CAPRI_SET_RAW_TABLE_PC(r6, req_rx_rrqptseg_process)
     CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

     nop.e
     nop

access_violation:
    nop.e
    nop
