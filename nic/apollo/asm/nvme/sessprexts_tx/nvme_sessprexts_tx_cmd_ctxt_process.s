#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_k_ k;
struct s2_t0_nvme_sessprexts_tx_cmd_ctxt_process_d d;

#define DMA_CMD_BASE    r5
#define AOL_P           r3
#define SHIFT           r4

//this number encodes which 64B AOL segment given page pointer falls into
#define AOL_NUM_ARR 0x5444333222111000

%%
    .param  nvme_sessprexts_tx_cb_writeback_process
    .param  nvme_tx_aol_base

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

    //calculate the offset at which the page ptr need to be stored 
    //in the command context
    add         r3, r3, NVME_CMD_CTXT_PAGE_LIST_OFFSET
    add         r3, r3, r1, LOG_NUM_PAGE_PTR_BYTES

    //dma command to store the page ptr
    DMA_CMD_BASE_GET(DMA_CMD_BASE, page_ptr_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, page_ptr_ptr, page_ptr_ptr, r3)

    //we also need to incrementally prepare the data digest AOL chain 
    //in the space pointed by the aolid. It has space for upto 8 AOLs and in each
    //AOL we have space for 3 pointers and fourth one need to be left for chain.
    //in summary, we need to divide the num_pages value by 3
    addui       AOL_P, r0, hiword(nvme_tx_aol_base)
    addi        AOL_P, AOL_P, loword(nvme_tx_aol_base)
    add         AOL_P, AOL_P, d.aolid, LOG_AOL_ENTRY_SIZE

    sll         SHIFT, r1, LOG_BITS_PER_NIB

    addui       r6, r0, AOL_NUM_ARR >> 32
    addi        r6, r6, AOL_NUM_ARR & 0xffffffff
    srl         r6, r6, SHIFT
    and         r6, r6, NIB_MASK
    add         r6, r6, r1
    add         AOL_P, AOL_P, r6, LOG_ONE_AOL_SIZE
    
    //dma command to store the dgst one aol
    DMA_CMD_BASE_GET(DMA_CMD_BASE, dgst_one_aol_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, dgst_one_aol_A0, dgst_one_aol_R0, AOL_P)

skip_page_ptr_update:

    phvwr       p.{t0_s2s_cmd_ctxt_to_writeback_info_slba...t0_s2s_cmd_ctxt_to_writeback_info_nlb}, d.{slba...nlb}
    phvwrpair   p.t0_s2s_cmd_ctxt_to_writeback_info_log_lba_size, d.log_lba_size, \
                p.t0_s2s_cmd_ctxt_to_writeback_info_log_host_page_size, d.log_host_page_size
    phvwr       p.t0_s2s_cmd_ctxt_to_writeback_info_prp1_offset, d.prp1_offset
    
    //store the keys in the xts descr
    phvwr       p.xts_desc_key_desc_index, d.{key_index}.wx
    phvwr       p.xts_desc_second_key_desc_index, d.{sec_key_index}.wx

    add         r1, r0, k.phv_global_common_cb_addr
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sessprexts_tx_cb_writeback_process,
                                r1) //Exit Slot
exit:
    phvwr.e p.p4_intr_global_drop, 1
    nop             //Exit Slot
