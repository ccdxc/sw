#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct resp_tx_rsqrkey_process_k_t k;

#define KEY_P r7
#define BASE_VA r6
#define LEN r5
#define LOG_PAGE_SIZE r4
#define MY_PT_BASE_ADDR r2
#define LOG_PT_SEG_SIZE r1
#define PT_SEG_P r5
#define PT_OFFSET r6
#define RAW_TABLE_PC r3
#define RQCB1_ADDR r5
#define DMA_CMD_BASE r1
#define IN_PROGRESS r2

#define PTSEG_INFO_T    struct resp_tx_rkey_to_ptseg_info_t
#define RQCB1_WB_INFO_T struct resp_tx_rqcb1_write_back_info_t

%%
    .param      resp_tx_rsqptseg_process
    .param      resp_tx_rqcb1_write_back_process

resp_tx_rsqrkey_process:

    
    // lkey_p = lkey_p + lkey_info_p->key_id;
    //big-endian
    sub         KEY_P, (HBM_NUM_KEY_ENTRIES_PER_CACHE_LINE - 1), k.args.key_id
    add         KEY_P, r0, KEY_P, LOG_SIZEOF_KEY_ENTRY_T_BITS

    // if (!(lkey_p->acc_ctrl & ACC_CTRL_LOCAL_WRITE)) {
    CAPRI_TABLE_GET_FIELD(r1, KEY_P, KEY_ENTRY_T, acc_ctrl)
    ARE_ALL_FLAGS_SET_B(c1, r1, ACC_CTRL_REMOTE_READ)
    bcf         [!c1], error_completion
    nop         //BD slot

    //  if ((lkey_info_p->sge_va < lkey_p->base_va) ||
    //  ((lkey_info_p->sge_va + lkey_info_p->sge_bytes) > (lkey_p->base_va + lkey_p->len))) {
    CAPRI_TABLE_GET_FIELD(BASE_VA, KEY_P, KEY_ENTRY_T, base_va)
    CAPRI_TABLE_GET_FIELD(LEN, KEY_P, KEY_ENTRY_T, len)
    CAPRI_TABLE_GET_FIELD(LOG_PAGE_SIZE, KEY_P, KEY_ENTRY_T, log_page_size)
    slt         c1, k.args.transfer_va, BASE_VA 
    add         r1, BASE_VA, LEN 
    add         r2, k.args.transfer_va, k.args.transfer_bytes
    slt         c2, r1, r2
    bcf         [c1 | c2], error_completion
    nop         //BD slot

    // my_pt_base_addr = (void *)
    //     (hbm_addr_get(PHV_GLOBAL_PT_BASE_ADDR_GET()) +
    //         (lkey_p->pt_base * sizeof(u64)));

    CAPRI_TABLE_GET_FIELD(r1, KEY_P, KEY_ENTRY_T, pt_base)
    PT_BASE_ADDR_GET(r2)
    add         MY_PT_BASE_ADDR, r2, r1, CAPRI_LOG_SIZEOF_U64
    // now r2 has my_pt_base_addr
    
    // pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE * phv_p->page_size;
    // i.e., log_pt_seg_size = LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE + LOG_PAGE_SIZE
    add         LOG_PT_SEG_SIZE, LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE, LOG_PAGE_SIZE
    // now r1 has log_pt_seg_size
    
    
    // transfer_offset = lkey_info_p->sge_va - lkey_p->base_va + lkey_p->base_va % pt_seg_size;
    add         r3, r0, BASE_VA
    // base_va % pt_seg_size
    mincr       r3, LOG_PT_SEG_SIZE, r0
    // add sge_va
    add         r3, r3, k.args.transfer_va
    // subtract base_va
    sub         r3, r3, BASE_VA
    // now r3 has transfer_offset

    // transfer_offset % pt_seg_size
    add         r7, r0, r3
    mincr       r7, LOG_PT_SEG_SIZE, r0
    // (transfer_offset % pt_seg_size) + transfer_bytes
    add         r7, r7, k.args.transfer_bytes
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
    srlv        r5, r3, LOG_PAGE_SIZE
    // transfer_offset%log_page_size 
    add         r6, r0, r3
    mincr       PT_OFFSET, LOG_PAGE_SIZE, r0
    // now r6 has pt_offset
    add         PT_SEG_P, MY_PT_BASE_ADDR, r5, CAPRI_LOG_SIZEOF_U64
    // now r5 has pt_seg_p
    b           invoke_pt
    nop
aligned_pt:
    // pt_offset = transfer_offset % pt_seg_size;
    // pt_seg_p = (u64 *)my_pt_base_addr + ((transfer_offset / phv_p->page_size) / HBM_NUM_PT_ENTRIES_PER_CACHE_LINE);
    srlv        r5, r3, LOG_PT_SEG_SIZE
    add         r6, r0, r3
    mincr       PT_OFFSET, LOG_PT_SEG_SIZE, r0
    add         PT_SEG_P, MY_PT_BASE_ADDR, r5, CAPRI_LOG_SIZEOF_U64

invoke_pt:

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rsqptseg_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, PT_SEG_P)
    
    CAPRI_GET_TABLE_0_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, pt_seg_offset, PT_OFFSET)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, pt_seg_bytes, k.args.transfer_bytes)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, dma_cmd_start_index, RESP_TX_DMA_CMD_PYLD_BASE)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, log_page_size, LOG_PAGE_SIZE)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, tbl_id, TABLE_0)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, dma_cmdeop, 1)

add_headers:

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_HDR_TEMPLATE)
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, HDR_TEMPLATE_T_SIZE_BYTES, k.args.header_template_addr)
    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)

    seq         c1, k.args.send_aeth, 1
    bcf         [!c1], invoke_write_back
    nop         //BD Slot

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_AETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, aeth, aeth)

invoke_write_back:

    CAPRI_GET_TABLE_1_K(resp_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rqcb1_write_back_process)
    RQCB1_ADDR_GET(RQCB1_ADDR)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_ADDR)

    CAPRI_GET_TABLE_1_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, RQCB1_WB_INFO_T, curr_read_rsp_psn, k.args.curr_read_rsp_psn)
    seq         c1, k.args.last_or_only, 1
    cmov        IN_PROGRESS, c1, 0, 1
    CAPRI_SET_FIELD(r7, RQCB1_WB_INFO_T, read_rsp_in_progress, IN_PROGRESS)
    CAPRI_SET_FIELD(r7, RQCB1_WB_INFO_T, new_rsq_c_index, k.args.new_rsq_c_index)

exit:
    nop.e
    nop

error_completion:
    //TODO

    nop.e
    nop
