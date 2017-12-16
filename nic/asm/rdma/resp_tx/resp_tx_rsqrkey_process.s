#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct resp_tx_rsqrkey_process_k_t k;
struct key_entry_aligned_t d;

#define KEY_P r7
#define LOG_PAGE_SIZE r4
#define MY_PT_BASE_ADDR r2
#define LOG_PT_SEG_SIZE r1
#define PT_SEG_P r5
#define PT_OFFSET r3
#define RAW_TABLE_PC r2
#define RQCB1_ADDR r5
#define DMA_CMD_BASE r1
#define IN_PROGRESS r2

#define PTSEG_INFO_T    struct resp_tx_rkey_to_ptseg_info_t
#define RQCB1_WB_INFO_T struct resp_tx_rqcb1_write_back_info_t

%%
    .param      resp_tx_rsqptseg_process
    .param      resp_tx_rqcb1_write_back_process
    .param      resp_tx_dcqcn_enforce_process

resp_tx_rsqrkey_process:

    
    // lkey_p = lkey_p + lkey_info_p->key_id;
    //big-endian
    sub         KEY_P, (HBM_NUM_KEY_ENTRIES_PER_CACHE_LINE - 1), k.args.key_id
    add         KEY_P, r0, KEY_P, LOG_SIZEOF_KEY_ENTRY_T_BITS

    // if (!(lkey_p->acc_ctrl & ACC_CTRL_LOCAL_WRITE)) {
    ARE_ALL_FLAGS_SET_B(c1, d.acc_ctrl, ACC_CTRL_REMOTE_READ)
    bcf         [!c1], error_completion
    nop         //BD slot

    //  if ((lkey_info_p->sge_va < lkey_p->base_va) ||
    //  ((lkey_info_p->sge_va + lkey_info_p->sge_bytes) > (lkey_p->base_va + lkey_p->len))) {
    slt         c1, k.args.transfer_va, d.base_va
    add         r1, d.base_va, d.len
    sslt        c2, r1, k.args.transfer_va, k.args.transfer_bytes
    bcf         [c1 | c2], error_completion
    nop         //BD slot

    // my_pt_base_addr = (void *)
    //     (hbm_addr_get(PHV_GLOBAL_PT_BASE_ADDR_GET()) +
    //         (lkey_p->pt_base * sizeof(u64)));

    PT_BASE_ADDR_GET(r2)
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
    add         r3, r3, k.args.transfer_va
    // subtract base_va
    sub         r3, r3, d.base_va
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
    add         LOG_PAGE_SIZE, r0, d.log_page_size
    srlv        r5, r3, LOG_PAGE_SIZE
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
    add         PT_SEG_P, MY_PT_BASE_ADDR, r5, CAPRI_LOG_SIZEOF_U64

invoke_pt:

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rsqptseg_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, PT_SEG_P)
    
    CAPRI_GET_TABLE_0_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, pt_seg_offset, PT_OFFSET)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, pt_seg_bytes, k.args.transfer_bytes)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, dma_cmd_start_index, RESP_TX_DMA_CMD_PYLD_BASE)
    CAPRI_SET_FIELD(r7, PTSEG_INFO_T, log_page_size, d.log_page_size)
    //CAPRI_SET_FIELD(r7, PTSEG_INFO_T, tbl_id, TABLE_0)

add_headers:

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_HDR_TEMPLATE)
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, k.args.header_template_size, k.args.header_template_addr)
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)

    // For PAD and ICRC
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PAD_ICRC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, icrc, icrc)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)

    seq         c1, k.args.send_aeth, 1
    bcf         [!c1], invoke_dcqcn
    nop         //BD Slot

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_AETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, aeth, aeth)

invoke_dcqcn:
    // Note: Next stage(DCQCN) does not use stage-to-stage keys. So this will be passed to write-back stage untouched!
    CAPRI_GET_TABLE_1_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, RQCB1_WB_INFO_T, curr_read_rsp_psn, k.args.curr_read_rsp_psn)
    seq         c1, k.args.last_or_only, 1
    cmov        IN_PROGRESS, c1, 0, 1
    CAPRI_SET_FIELD(r7, RQCB1_WB_INFO_T, read_rsp_in_progress, IN_PROGRESS)

    CAPRI_GET_TABLE_1_K(resp_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_dcqcn_enforce_process)
    bbeq           k.to_stage.s3.rsq_rkey.congestion_mgmt_enable, 1, dcqcn
    add            r3,  r0, k.to_stage.s3.rsq_rkey.dcqcn_cb_addr // BD slot

dcqcn_mpu_only:
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, RAW_TABLE_PC, r3)
    nop.e
    nop

dcqcn:
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r3)

exit:
    nop.e
    nop

error_completion:
    //TODO

    nop.e
    nop
