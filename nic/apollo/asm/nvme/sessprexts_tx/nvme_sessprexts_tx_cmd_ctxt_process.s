#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_k_ k;
struct s2_t0_nvme_sessprexts_tx_cmd_ctxt_process_d d;

#define DMA_CMD_BASE    r5

%%
    .param  nvme_sessprexts_tx_cb_writeback_process

.align
nvme_sessprexts_tx_cmd_ctxt_process:
    add         r1, r0, d.num_pages
    seq         c1, k.to_s2_info_incr_num_pages, 1
    cmov        r2, c1, 1, 0

    //note that there is a possible race condition of num_pages getting
    //updated before the page pointer updation, as num_pages field is getting
    //updated using tblwr where as page pointer is updated using DMA.
    //XXX TBD: fix this later
    //one option is to update num_pages update also using DMA instruction, but
    //for that we might have to introduce a new CB state variable to track the
    //current num_pages value. Other option is to compute num_pages value up front
    //during SQ pass itself. Though it doesn't fix the problem, it might simplify
    //the code in terms of clarity.
    tbladd.f    d.num_pages, r2 //Flush

    bcf         [!c1], skip_page_ptr_update
    mfspr       r3, spr_tbladdr     //BD Slot
    add         r3, r3, NVME_CMD_CTXT_PAGE_LIST_OFFSET
    add         r3, r3, r1, LOG_NUM_PAGE_PTR_BYTES

    DMA_CMD_BASE_GET(DMA_CMD_BASE, page_ptr_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, page_ptr_ptr, page_ptr_ptr, r3)

skip_page_ptr_update:

    phvwr       p.{t0_s2s_cmd_ctxt_to_writeback_info_slba...t0_s2s_cmd_ctxt_to_writeback_info_nlb}, d.{slba...nlb}
    phvwrpair   p.t0_s2s_cmd_ctxt_to_writeback_info_log_lba_size, d.log_lba_size, \
                p.t0_s2s_cmd_ctxt_to_writeback_info_log_host_page_size, d.log_host_page_size
    phvwr       p.t0_s2s_cmd_ctxt_to_writeback_info_prp1_offset, d.prp1_offset
    
    //store the keys in the xts descr
    phvwr       p.xts_desc_key_desc_index, d.key_index
    phvwr       p.xts_desc_second_key_desc_index, d.sec_key_index

    add         r1, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sessprexts_tx_cb_writeback_process,
                                r1) //Exit Slot
exit:
    phvwr.e p.p4_intr_global_drop, 1
    nop             //Exit Slot
