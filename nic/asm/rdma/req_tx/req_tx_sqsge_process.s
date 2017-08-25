# include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_sqsge_process_k_t k;

#define INFO_OUT1_T struct req_tx_sge_to_lkey_info_t
#define INFO_OUT2_T struct req_tx_rrqwqe_to_hdr_info_t

#define LOG_PAGE_SIZE  10

%%
    .param    req_tx_sqlkey_process
    .param    req_tx_add_headers_process

req_tx_sqsge_process:
    // r1 = sge_index = 0
    add     r1, r0, r0
    // r2 = k.args.current_sge_offset
    add     r2, r0, k.args.current_sge_offset
    // r3 = k.args.remaining_payload_bytes
    add     r3, r0, k.args.remaining_payload_bytes
    // num_pages = 0
    add     r5, r0, r0

sge_loop:
    // Due to limited register count, use sge_index to compute pointer for table read
    // and revert back to sge_index after the read
    sll     r1, r1, LOG_SIZEOF_SGE_T_BITS

    // sge_remaining_bytes = sge_p->len - current_sge_offset
    tblrdp  r4, r1, offsetof(SGE_T, len), sizeof(SGE_T.len)
    sub     r4, r4, r2

    // transfer_bytes = min(sge_remaining_bytes, remaining_payload_bytes)
    slt     c1, r4, r3
    cmov    r4, c1, r4, r3

    // transfer_va = sge_p->va + current_sge_offset
    tblrdp   r6, r1, offsetof(SGE_T, va), sizeof(SGE_T.va)
    add      r6, r6, r2

    // Revert back to sge_index so that K and arg base in phv can be computed correctly
    srl      r1, r1, LOG_SIZEOF_SGE_T_BITS

    // Get common.common_t[0]_s2s or common.common_t[1]_s2s... args based on sge_index
    // to invoke programs in multiple MPUs
    CAPRI_GET_TABLE_I_ARG(req_tx_phv_t, r1, r7)

    // Fill stage 2 stage data in req_tx_sge_lkey_info_t for next stage
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, sge_va, r6)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, sge_bytes, r4)
    add     r5, r5, k.args.dma_cmd_start_index
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, dma_cmd_start_index, r5)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, sge_index, r1)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, log_page_size, LOG_PAGE_SIZE)

    // current_sge_offset += transfer_bytes
    add     r2, r2, r4

    // remaining_payload_bytes -= transfer_bytes
    sub     r3, r3, r2

    #get_num_pages(r5, r6, LOG_PAGE_SIZE, r5, r4)
     srl    r4, r5, LOG_PAGE_SIZE
     add    r5, r5, r6
     srl    r5, r5, LOG_PAGE_SIZE
     sub    r5, r5, r4
     add    r5, r5, 1

    // Due to limited register count, use sge_index to compute pointer for table read
    // and revert back to sge_index after the read
    sll     r1, r1, LOG_SIZEOF_SGE_T_BITS

    // r4 = sge_p->lkey
    tblrdp  r4, r5, offsetof(SGE_T, l_key), sizeof(SGE_T.l_key)

    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey & KEY_INDEX_MASK) * sizeof(key_entry_t));
    andi    r4, r4, KEY_INDEX_MASK
    sll     r4, r4, LOG_SIZEOF_KEY_ENTRY_T
    KT_BASE_ADDR_GET(r6)
    add     r4, r4, r6

    // aligned_key_addr = key_addr & ~HBM_CACHE_LINE_MASK
    and     r6, r4, HBM_CACHE_LINE_SIZE_MASK

    // key_id = (key_addr % HBM_CACHE_LINE_SIZE) / sizeof(key_entry_t);
    sub     r4, r4, r6
    srl     r4, r4, LOG_SIZEOF_KEY_ENTRY_T

    CAPRI_SET_FIELD(r7, INFO_OUT1_T, key_id, r4)

    // r4 = sge_p->len
    tblrdp  r4, r5, offsetof(SGE_T, len), sizeof(SGE_T.len)

    // if (current_sge_offset == sge_p->len)
    seq     c1, r2, r4

    // Revert back to sge_index so that K and Arg base in phv can be computed correctly
    srl     r1, r1, LOG_SIZEOF_SGE_T_BITS

    // Get common.common_te[0]_phv_table_addr or common.common_te[1]_phv_table_Addr ... based on
    // sge_index to invoke program in multiple MPUs
    CAPRI_GET_TABLE_I_K(req_tx_phv_t, r1, r7)

    // aligned_key_addr and key_id sent to next stage to load lkey
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqlkey_process, r6)

    // sge_index++;
    add.c1  r1, r1, 1

    // current_sge_offset = 0;
    add.c1  r2, r0, r0

    // while((remaining_payload_bytes > 0) &&
    //       (index < MAX_SGE_PASS) &&
    //       (index < num_valid_sges) &&
    //       (index < HBM_NUM_SGES_PER_CACHELINE))
    slt     c2, r3, r0
    slt     c3, r1, MAX_SGES_PER_PASS
    slt     c4, r1, k.args.num_valid_sges
    slt     c5, r1, HBM_NUM_SGES_PER_CACHELINE

    bcf     [!c2 & c3 & c4 & c5], sge_loop
    // sge_p++
    add.c1  r5, r5, 1, LOG_SIZEOF_SGE_T_BITS // Branch Delay Slot

    // if (index == num_valid_sges)
    seq        c1, r1, k.args.num_valid_sges
    // current_sge_id = 0
    add.c1     r1, r0, r0
    // current_sge_offset = 0
    add.c1     r2, r0, r0
    // if (index == num_valid_sges) last = TRUE else last = FALSE;
    cmov       r3, c1, 1, r0

    // else
    // current_sge_id = k.args.current_sge_id + sge_index
    add.!c1    r1, r1, k.args.current_sge_id
    // in_progress = TRUE
    cmov        r4, c1, r0, 1

    // num_sges = k.args.current_sge_id + k.args.num_valid_sges
    add     r5, k.args.current_sge_id, k.args.num_valid_sges

    add     r7, r0, offsetof(struct req_tx_phv_t, common.common_t2_s2s_s2s_data)

    CAPRI_SET_FIELD(r7, INFO_OUT2_T, in_progress, r4)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, busy, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.send_wr.current_sge_id, r1)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.send_wr.current_sge_offset, r2)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.send_wr.num_sges, r5)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, wqe_addr, k.args.wqe_addr)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, last, r3)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, first, k.args.first)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op_type, k.args.op_type)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, tbl_id, 2) // Table 2
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.send_wr.imm_data, k.args.imm_data)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.send_wr.inv_key, k.args.inv_key)

    add     r7, r0, offsetof(struct req_tx_phv_t, common.common_te2_phv_table_addr)
    SQCB1_ADDR_GET(r1)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_add_headers_process, r1)

    nop.e
    nop
